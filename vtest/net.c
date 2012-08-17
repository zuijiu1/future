#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

typedef struct sockaddr SA;

static struct sockaddr_in saddr;
static struct sockaddr_in caddr;
static int nfd;

static int neton = 0;

int netinit(char *host, int port)
{
	int nfd, ret;

	nfd = socket(AF_INET, SOCK_STREAM, 0);
	if (nfd < 0){
		fprintf(stderr, "%s",strerror(errno));
		return nfd;
	}
	memset(&saddr, 0, sizeof(struct sockaddr_in));
	saddr.sin_family = AF_INET;
	printf("1\n");
	saddr.sin_addr.s_addr = inet_addr(host);
		printf("2\n");
	saddr.sin_port = htons((unsigned short)port);

	return nfd;
}

int netsend(int nfd, char *buf, int size)
{
	int ret;

	if (!neton){
		ret = connect(nfd, (SA *)&saddr, sizeof(saddr));
		if (ret < 0){
//			printf("connect failed %d errno %s\n", ret, strerror(errno));
			return -1;
		}

		neton = 1;
	}

	ret = send(nfd, buf, size, 0);
	if (ret < 0){
//		printf ("connection breakout!\n");
		neton = 0;
		return -1;
	}
	return size;
}

int netexit(int nfd)
{
	close(nfd);
}
