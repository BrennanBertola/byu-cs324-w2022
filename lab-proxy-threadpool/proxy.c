#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define HOST_PREFIX 2
#define PORT_PREFIX 1
#define BUF_SIZE 500

static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:97.0) Gecko/20100101 Firefox/97.0";

int all_headers_received(char *);
int parse_request(char *, char *, char *, char *, char *, char *);
int open_sfd();
void test_parser();
void print_bytes(unsigned char *, int);
void handle_client(int nsfd);


int main(int argc, char *argv[])
{
	// test_parser();
	printf("%s\n", user_agent_hdr);

	int sfd = open_sfd(argc, argv);
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
	while(1) {
		peer_addr_len = sizeof(struct sockaddr_storage);
		int nsfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_len);
		handle_client(nsfd);
	}
	return 0;
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
				buf = &buf[i + 1];
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
				buf = &buf[i+1];
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

		"GET http://www.example.com:8080/index.html HTTP/1.0\r\n",

		NULL
	};
	
	for (i = 0; reqs[i] != NULL; i++) {
		printf("Testing %s\n", reqs[i]);
		if (parse_request(reqs[i], method, hostname, port, path, headers)) {
			printf("METHOD: %s\n", method);
			printf("HOSTNAME: %s\n", hostname);
			printf("PORT: %s\n", port);
			printf("HEADERS: %s\n", headers);
		} else {
			printf("REQUEST INCOMPLETE\n");
		}
	}
}

int open_sfd(int argc, char* argv[]) {
	int sfd, s;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

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

	if (bind(sfd, result->ai_addr, result->ai_addrlen) < 0) {
		fprintf(stderr, "Could not bind");
		exit(1);
	}

	listen(sfd, 100);

	return sfd;
}

void handle_client(int nsfd) {
	char buf[BUF_SIZE];
	int nread = 0;
	for(;;) {
		nread = recv(nsfd, &buf[nread], BUF_SIZE, 0);
		
		if (all_headers_received(buf) == 1) {
			break;
		}
	}

	char method[16], hostname[64], port[8], path[64], headers[1024];
	printf("Testing %s\n", buf);
	if (parse_request(buf, method, hostname, port, path, headers)) {
		printf("METHOD: %s\n", method);
		printf("HOSTNAME: %s\n", hostname);
		printf("PORT: %s\n", port);
		printf("PATH: %s\n", path);
		printf("HEADERS: %s\n", headers);
	} else {
		printf("REQUEST INCOMPLETE\n");
	}

	close(nsfd);
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
