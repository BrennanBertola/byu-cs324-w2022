#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include<sys/epoll.h>
#include<errno.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<signal.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAXEVENTS 64
#define MAXLINE 2048
#define HOST_PREFIX 2
#define PORT_PREFIX 1

static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:97.0) Gecko/20100101 Firefox/97.0";

struct request_info {
	int cfd;
	int sfd;
	int state; //0=ReadReq, 1=SendReq, 2=ReadRes, 3=SendRes, 4=done
	char readBuf[MAX_CACHE_SIZE];
	char writeBuf[MAX_CACHE_SIZE];
	int readC;
	int writeAmount;
	int writtenS;
	int readS;
	int writtenC;
};

struct request_info *newRequest;
struct request_info *activeRequest;

struct epoll_event event;
struct epoll_event *events;

int signalFlag = 0;

int all_headers_received(char *);
int parse_request(char *, char *, char *, char *, char *, char *);
void test_parser();
void print_bytes(unsigned char *, int);
int open_sfd(int argc, char* argv[]);
void handle_new_clients(int efd, int sfd);
void handle_client(struct request_info *request, int erfd, int ewfd);

void read_request(struct request_info *request, int ewfd);
void send_request(struct request_info *request, int erfd);
void read_response(struct request_info *request, int ewfd);
void send_response(struct request_info *request);

void sig_handler(int signum) {
	signalFlag = 1;
}


int main(int argc, char* argv[])
{
	// test_parser();
	// printf("%s\n", user_agent_hdr);

	struct sigaction sigact;

	// zero out flags
	sigact.sa_flags = SA_RESTART;

	sigact.sa_handler = sig_handler;
	sigaction(SIGINT, &sigact, NULL);

	int erfd; //epoll for reading
	if ((erfd = epoll_create1(0)) < 0) {
		fprintf(stderr, "error creating epoll fd\n");
		exit(1);
	}

	int ewfd; //epoll for writting
	if ((ewfd = epoll_create1(0)) < 0) {
		fprintf(stderr, "error creating epoll fd\n");
		exit(1);
	}

	int lfd = open_sfd(argc, argv);
	newRequest = malloc(sizeof(struct request_info));
	newRequest->cfd = lfd;

	event.data.ptr = newRequest;
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(erfd, EPOLL_CTL_ADD, lfd, &event) < 0) {
		fprintf(stderr, "error adding event\n");
		exit(1);
	}

	events = calloc(MAXEVENTS, sizeof(struct epoll_event));

	while(1) {
		int result = epoll_wait(erfd, events, MAXEVENTS, 1000);

		if(result < 0) {
			if (errno == EBADF) {
				fprintf(stderr, "epoll_wait() error: EBADF\n");
				break;
			}
			else if (errno == EFAULT) {
				fprintf(stderr, "epoll_wait() error: EFAULT\n");
				break;
			}
			else if (errno == EINTR) {
				fprintf(stderr, "epoll_wait() error: EINTR\n");
				break;
			}
			else if (errno == EINVAL) {
				fprintf(stderr, "epoll_wait() error: EINVAL\n");
				break;
			}
		}
		else if (result == 0) {
			if (signalFlag) {
				break;
			}
		}

		for (int i = 0; i < result; ++i) {
			activeRequest = (struct request_info *)(events[i].data.ptr);

			if ((events[i].events & EPOLLERR) ||
					(events[i].events & EPOLLHUP) ||
					(events[i].events & EPOLLRDHUP)) {
				/* An error has occured on this fd */
				fprintf(stderr, "epoll error on %d\n", activeRequest->cfd);
				close(activeRequest->cfd);
				close(activeRequest->sfd);
				free(activeRequest);
				continue;
			}

			if (activeRequest->cfd == lfd) {
				handle_new_clients(erfd, lfd);
			}
			else {
				handle_client(activeRequest, erfd, ewfd);
			}
		}
	}
	free(events);
	close(erfd);
	close(ewfd);
	close(lfd);
	return 0;
}


void handle_new_clients(int efd, int lfd) {
	while (1) {
		socklen_t clientlen = sizeof(struct sockaddr_storage);
		struct sockaddr_storage clientaddr;
		int nsfd = accept(lfd, (struct sockaddr *)&clientaddr, &clientlen);

		if (nsfd < 0) {
			if (errno == EWOULDBLOCK ||
					errno == EAGAIN) {
				// no more clients ready to accept
				break;
			} else {
				fprintf(stderr, "error");
				exit(EXIT_FAILURE);
			}			
		}

		if (fcntl(nsfd, F_SETFL, fcntl(nsfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
			fprintf(stderr, "error setting socket option\n");
			exit(1);
		}

		newRequest = (struct request_info *)malloc(sizeof(struct request_info));
		newRequest->cfd = nsfd;
		 fprintf(stderr, "new fd: %d\n", newRequest->cfd);

		event.data.ptr = newRequest;
		event.events = EPOLLIN | EPOLLET;
		if (epoll_ctl(efd, EPOLL_CTL_ADD, nsfd, &event) < 0) {
			fprintf(stderr, "error adding event\n");
			exit(1);
		}
	}
}

void handle_client(struct request_info *request, int erfd, int ewfd){
	//fprintf(stderr, "entered handle client, cfd: %d\n", request->cfd);
	if (request->state == 0) {
		read_request(request, ewfd);
	}
	if (request->state == 1) {
		send_request(request, erfd);
	}
	if (request->state == 2) {
		read_response(request, ewfd);
	}
	if (request->state == 3) {
		send_response(request);
	}
	if (request->state == 4) {
		fprintf(stderr, "\nTOO FAR\n");
	}
}


void read_request(struct request_info *request, int ewfd) {
	fprintf (stderr, "in read req, cfd: %d\n", request->cfd);
	fflush(stderr);

	while(1) {
		int len = recv(request->cfd, &request->readBuf[request->readC], MAXLINE, 0);

		if (len < 0) {
			if (errno == EWOULDBLOCK ||
					errno == EAGAIN) {
				// no more data to be read
			} else {
				fprintf(stderr, "client recv, cfd: %d\n", request->cfd);
				close(request->cfd);
				free(request);
			}
			return;
		} 

		request->readC = request->readC + len;
		if (all_headers_received(request->readBuf) == 1) {
			break;
		} 
	}

	char method[16], hostname[64], port[8], path[64], headers[1024];
	if (parse_request(request->readBuf, method, hostname, port, path, headers)) {
		if (strcmp(port, "80")) {
			sprintf(request->writeBuf, "%s %s HTTP/1.0\r\nHost: %s:%s\r\nUser-Agent: %s\r\nConnection: close\r\nProxy-Connection: close\r\n\r\n", 
			method, path, hostname, port, user_agent_hdr);
		}
		else {
			sprintf(request->writeBuf, "%s %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\nProxy-Connection: close\r\n\r\n", 
			method, path, hostname, user_agent_hdr);
		}
		request->writeAmount = strlen(request->writeBuf);
		printf("%s", request->writeBuf);
	} else {
		printf("REQUEST INCOMPLETE\n");
		close(request->cfd);
		free(request);
		return;
	}



	int nsfd, s;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;

	s = getaddrinfo(hostname, port, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo error\n");
		close(request->cfd);
		free(request);
		return;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		nsfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (nsfd == -1)
			continue;

		if (connect(nsfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;  /* Success */

		close(nsfd);
	}

	if (fcntl(nsfd, F_SETFL, fcntl(nsfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
		fprintf(stderr, "error setting socket option\n");
		close(request->cfd);
		free(request);
		return;
	}

	request->sfd=nsfd;
	event.data.ptr = request;
	event.events = EPOLLOUT | EPOLLET;
	if (epoll_ctl(ewfd, EPOLL_CTL_ADD, nsfd, &event) < 0) {
		fprintf(stderr, "error adding event\n");
		close(request->cfd);
		close(request->sfd);
		free(request);
		return;
	}

	memset(request->readBuf, '\0', request->readC);
	request->state++;
}

void send_request(struct request_info *request, int erfd) {
	fprintf (stderr, "in send req, sfd: %d\n", request->sfd);
	fflush(stderr);

	while(1) {
		int len = write(request->sfd, &request->writeBuf[request->writtenS], request->writeAmount);

		if (len < 0) {
			if (errno == EWOULDBLOCK ||
					errno == EAGAIN) {
				// no more data to be read
			} else {
				fprintf(stderr, "server write , sfd: %d\n", request->sfd);
				close(request->cfd);
				close(request->sfd);
				free(request);
			}
			return;
		}

		request->writtenS += len;
		if (request->writtenS >= request->writeAmount) {
			break;
		} 
	}

	event.data.ptr = request;
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(erfd, EPOLL_CTL_ADD, request->sfd, &event) < 0) {
		fprintf(stderr, "error adding event\n");
		exit(1);
	}

	memset(request->writeBuf, '\0', request->writtenS);
	request->state++;
}

void read_response(struct request_info *request, int ewfd) {
	fprintf (stderr, "in read res, cfd: %d\n", request->cfd);
	fflush(stderr);

	while(1) {
		int len = recv(request->sfd, &request->readBuf[request->readS], MAXLINE, 0);

		if (len < 0) {
			if (errno == EWOULDBLOCK ||
					errno == EAGAIN) {
				// no more data to be read
			} else {
				fprintf(stderr, "server recv , sfd: %d\n", request->sfd);
				close(request->cfd);
				free(request);
			}
			return;
		} 

		if (len == 0) {

			break;
		}

		request->readS += len;
	}

	event.data.ptr = request;
	event.events = EPOLLOUT | EPOLLET;
	if (epoll_ctl(ewfd, EPOLL_CTL_ADD, request->cfd, &event) < 0) {
		fprintf(stderr, "error adding event\n");
		exit(1);
	}

	request->writeAmount = request->readS;
	request->state++;
}

void send_response(struct request_info *request) {
	fprintf (stderr, "in send res, cfd: %d\n", request->cfd);
	fflush(stderr);
	fprintf(stderr, "starting\n");
	fflush(stderr);

	while(1) {
		int len = write(request->cfd, &request->readBuf[request->writtenC], request->writeAmount);

		if (len < 0) {
			if (errno == EWOULDBLOCK ||
					errno == EAGAIN) {
				// no more data to be read
			} else {
				fprintf(stderr, "client write , cfd: %d\n", request->cfd);
				close(request->cfd);
				close(request->sfd);
				free(request);
			}
			return;
		}

		request->writtenC += len;
		if (request->writtenC >= request->writeAmount) {
			break;
		} 
	}

	fprintf(stderr, "done Writing\n");
	fflush(stderr);

	close(request->cfd);
	close(request->sfd);
	//free(request);
	request->state++;
	fprintf(stderr, "done\n");
	fflush(stderr);
}

int open_sfd(int argc, char* argv[]) {
	int sfd, s;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;

	if(argc < 1) {
		fprintf(stderr, "Missing command line argument");
		exit(1);
	}

	s = getaddrinfo(NULL, argv[1], &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo erro");
		exit(1);
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (sfd == -1) {
			continue;
		}
		break;
	}

	int optval = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

	// set listening file descriptor non-blocking
	if (fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
		fprintf(stderr, "error setting socket option\n");
		exit(1);
	}

	if (bind(sfd, result->ai_addr, result->ai_addrlen) < 0) {
		fprintf(stderr, "Could not bind");
		exit(1);
	}

	listen(sfd, 100);

	// set listening file descriptor non-blocking
	if (fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
		fprintf(stderr, "error setting socket option\n");
		exit(1);
	}

	return sfd;
}

int all_headers_received(char *request) {
	if (strstr(request, "\r\n\r\n") != NULL) {
		return 1;
	}
	return 0;
}

int parse_request(char *request, char *method,
		char *hostname, char *port, char *path, char *headers) {
			if (all_headers_received(request) == 0) {
				return 0;
			}

			char* buf;
			int found = 0;
			unsigned int i = 0;
			while (i < strlen(request)) {
				if (request[i] == ' ') {
					found = 1;
					break;
				}
				 ++i;
			}
			if (found != 1) {
				return 0;
			}
			strncpy(method, request, i);
			method[i] = '\0';
			
			buf = strstr(request, "//");
			i = HOST_PREFIX;
			int defaultPort = 0;
			found = 0;
			while (i < strlen(buf)) {
				if (buf[i] == '/') {
					found = 1;
					defaultPort = 1;
					break;
				}
				else if (buf[i] == ':') {
					found = 1;
					defaultPort = 0;
					break;
				}
				++i;
			}
			if (found != 1) {
				return 0;
			}
			strncpy(hostname, &buf[HOST_PREFIX], i - HOST_PREFIX);
			hostname[i - HOST_PREFIX] = '\0';

			if (defaultPort == 1) {
				strcpy(port, "80"); // default port
				buf = &buf[i];
			}
			else {
				found = 0;
				buf = strchr(buf, ':');
				i = PORT_PREFIX;
				while (i < strlen(buf)) {
					if (buf[i] == '/') {
						found = 1;
						break;
					}
					++i;
				}
				if (found != 1) {
					return 0;
				}
				strncpy(port, &buf[1], i - PORT_PREFIX);
				port[i - PORT_PREFIX] = '\0';
				buf = &buf[i];
			}

			found = 0;
			i = 0;
			while (i < strlen(buf)) {
				if (buf[i] == ' ') {
					found = 1;
					break;
				}
				++i;
			}
			strncpy(path, &buf[0], i);
			path[i] = '\0';

			buf = strstr(request, "\r\n");
			strcpy (headers, &buf[2]);

			return 1;
}

void test_parser() {
	int i;
	char method[16], hostname[64], port[8], path[64], headers[1024];

       	char *reqs[] = {
		"GET http://www.example.com/index.html HTTP/1.0\r\n"
		"Host: www.example.com\r\n"
		"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0\r\n"
		"Accept-Language: en-US,en;q=0.5\r\n\r\n",

		"GET http://www.example.com:8080/index.html?foo=1&bar=2 HTTP/1.0\r\n"
		"Host: www.example.com:8080\r\n"
		"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0\r\n"
		"Accept-Language: en-US,en;q=0.5\r\n\r\n",

		"GET http://localhost:1234/home.html HTTP/1.0\r\n"
		"Host: localhost:1234\r\n"
		"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0\r\n"
		"Accept-Language: en-US,en;q=0.5\r\n\r\n",

		"GET http://www.example.com:8080/index.html HTTP/1.0\r\n",

		NULL
	};
	
	for (i = 0; reqs[i] != NULL; i++) {
		printf("Testing %s\n", reqs[i]);
		if (parse_request(reqs[i], method, hostname, port, path, headers)) {
			char newReq[MAX_CACHE_SIZE];
			if (strcmp(port, "80")) {
				sprintf(newReq, "%s %s HTTP/1.0\r\nHost: %s:%s\r\nUser-Agent: %s\r\nConnection: close\r\nProxy-Connection: close\r\n\r\n", 
				method, path, hostname, port, user_agent_hdr);
			}
			else {
				sprintf(newReq, "%s %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\nProxy-Connection: close\r\n\r\n", 
				method, path, hostname, user_agent_hdr);
			}
			printf("%s", newReq);
		} else {
			printf("REQUEST INCOMPLETE\n");
		}
	}
}

void print_bytes(unsigned char *bytes, int byteslen) {
	int i, j, byteslen_adjusted;

	if (byteslen % 8) {
		byteslen_adjusted = ((byteslen / 8) + 1) * 8;
	} else {
		byteslen_adjusted = byteslen;
	}
	for (i = 0; i < byteslen_adjusted + 1; i++) {
		if (!(i % 8)) {
			if (i > 0) {
				for (j = i - 8; j < i; j++) {
					if (j >= byteslen_adjusted) {
						printf("  ");
					} else if (j >= byteslen) {
						printf("  ");
					} else if (bytes[j] >= '!' && bytes[j] <= '~') {
						printf(" %c", bytes[j]);
					} else {
						printf(" .");
					}
				}
			}
			if (i < byteslen_adjusted) {
				printf("\n%02X: ", i);
			}
		} else if (!(i % 4)) {
			printf(" ");
		}
		if (i >= byteslen_adjusted) {
			continue;
		} else if (i >= byteslen) {
			printf("   ");
		} else {
			printf("%02X ", bytes[i]);
		}
	}
	printf("\n");
}
