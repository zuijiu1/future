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

/* #define QCIF (176*144) */
/* #define CIF (352*288) */
/* #define D1 (720*576) */
/* #define SIZE (CIF*3/2) */
static int SIZE;

typedef struct sockaddr SA;
#define LISTENQ 10

void sp2p(unsigned char *buf, unsigned char *buf_sp, int size)
{
	int i;
	unsigned char *p;
	
	memcpy(buf, buf_sp, size*2/3);

	buf += size*2/3;
	buf_sp += size*2/3;
	p = buf_sp;

	for (i=0;i<size/3;i+=2){
		*buf++ = *(p+i);

	}
	for (i=1;i<size/3;i+=2){
		*buf++ = *(p+i);
	}
}

int recv1frame(int fd, unsigned char *buf_sp, int size)
{
	int n;
	int sz;

	sz = size;
	while((n = recv(fd, buf_sp, sz, 0)) > 0){
		if (n < sz){
			sz = sz - n;
			buf_sp += n;
		}
		else
			break;
	}
	if (n <= 0){
		fprintf(stderr, "connection break\n");
		return -1;
	}
	
	return size;
}

static void echo(int fd)
{
	unsigned char *buf_sp;
	unsigned char *buf;
	int n;
	int j;

	buf_sp = (unsigned char *)malloc(SIZE);
	buf = (unsigned char *)malloc(SIZE);
	memset(buf_sp, 0, SIZE);
	memset(buf, 0, SIZE);
	
	while((n = recv1frame(fd, buf_sp, SIZE)) > 0){
		fprintf(stderr, "%s:expect %d, server recv %d bytes\n", __FUNCTION__, SIZE, n);
		sp2p(buf, buf_sp, SIZE);
		n = write(1, buf, n);
		if (n < 0){
			fprintf(stderr, "write to stdout failed!\n");
			break;
		}
	}

	free(buf);
	free(buf_sp);
}


int main(int argc, char **argv)
{
	int fd, port, optval, ret, clen, connfd;
	struct sockaddr_in saddr;
	struct sockaddr_in caddr;
	struct hostent *hp;
	char *haddrp;
	
	if (argc != 4){
		fprintf(stderr, "usage: %s <port><width><height>\n", argv[0]);
		exit(0);
	}
	port = atoi(argv[1]);
	SIZE = atoi(argv[2]) * atoi(argv[3]) * 3/2;
	
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		return fd;
	ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
	                 (const void *)&optval, sizeof(int));
	if (ret < 0)
		return ret;

	memset(&saddr, 0, sizeof(struct sockaddr_in));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons((unsigned short)port);

	ret = bind(fd, (SA *)&saddr, sizeof(saddr));
	if (ret < 0)
		return ret;
	ret = listen(fd, LISTENQ);
	if (ret < 0)
		return ret;

	while(1) {
		clen = sizeof(caddr);
		connfd = accept(fd, (SA *)&caddr, &clen);
		if (connfd < 0)
			fprintf(stderr, "accept failed %d errno %s\n", connfd, strerror(errno));
		fprintf(stderr, "accept %d\n", connfd);
		haddrp = inet_ntoa(caddr.sin_addr);
		fprintf(stderr, "server connected to %s\n", haddrp);
		echo(connfd);
		close(connfd);
	}
	
	close(fd);

	return 0;
}

