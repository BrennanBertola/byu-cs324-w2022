// Replace PUT_USERID_HERE with your actual BYU CS user id, which you can find
// by running `id -u` on a CS lab machine.
#define USERID 1823698971

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

int SERVER = 1;
int PORT = 2;
int LEVEL = 3;
int SEED = 4;

int verbose = 0;
int INBOUND = 64;
int INITIAL = 8;
int TREASURE = 1024;
int BUF = 4;

void print_bytes(unsigned char *bytes, int byteslen);

int main(int argc, char *argv[]) {
	unsigned char treasure[1024];
	unsigned int treasureIndex = 0;
	unsigned char initial[INITIAL];
	unsigned char inbound[INBOUND];
	
	if (argc < 4) {
		fprintf(stderr, "cmd line arguments\n");
		exit(1);
	}

	unsigned int id = htonl(USERID); //0x6cb3701b
	unsigned short seed = htons(*argv[SEED]);

	initial[0] = 0;
	initial[1] = atoi(argv[LEVEL]);
	memcpy(&initial[2], &id, 4);
	memcpy(&initial[6], &seed, 2);

	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;
	int af = AF_INET;
	int chunkSize = 0;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = af;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	s = getaddrinfo(argv[SERVER], argv[PORT], &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(1);
	}

		for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (sfd == -1)
			continue;

		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;  /* Success */

		close(sfd);
	}

	if (rp == NULL) {   /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		exit(1);
	}

	freeaddrinfo(result);   /* No longer needed */

	send(sfd, initial, INITIAL, 0);
	int nread = read(sfd, &inbound, INBOUND);

	if (nread == -1) {
		fprintf(stderr, "read");
		exit(1);
	}

	chunkSize = inbound[0];
	memcpy(&treasure[treasureIndex], &inbound[1], chunkSize);
	treasureIndex += chunkSize;

	for(;;) {
		if (chunkSize == 0) {
			break;
		}
		if (chunkSize > 127) {
			fprintf(stderr, "error: %d\n", inbound[0]);
			break;
		}

		unsigned int nonce;
		memcpy(&nonce, &inbound[inbound[0] + 4], 4);
		nonce = ntohl(nonce) + 1;
		nonce = htonl(nonce);

		unsigned char buf[BUF];
		memcpy(&buf, &nonce, 4);
		send(sfd, buf, BUF, 0);
		nread = read(sfd, &inbound, INBOUND);

		if (nread == -1) {
			fprintf(stderr, "read");
			exit(1);
		}

		chunkSize = inbound[0];
		memcpy(&treasure[treasureIndex], &inbound[1], chunkSize);
		treasureIndex += chunkSize;
	}
	treasure[treasureIndex] = '\0';
	printf("%s\n", treasure);
	//print_bytes(treasure, treasureIndex);
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
