#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define MAXLINE 80

int main (int argc, char *argv[]) {
	int n, listenfd, connfd, caddrlen, nbytes, fd;
	struct hostent *h;
	struct sockaddr_in saddr, caddr;
	char buf[MAXLINE];
	int port = atoi(argv[1]);

	
	/* Write your code Here */	
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket() failed.\n");
		exit(1);
	}

	memset((char *)&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(port);
	if (bind(listenfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
		printf("bind() failed.\n");
		exit(2);
	}
	if (listen(listenfd, 5) < 0) {
		printf("listen() failed.\n");
		exit(3);
	}
	printf("wait connection ...\n");
	while (1) {
		caddrlen = sizeof(caddr);
		if ((connfd = accept(listenfd, (struct sockaddr *)&caddr, (socklen_t *)&caddrlen)) < 0) {
			printf ("accept() failed.\n");
			continue;
		}
		while ((n = read(connfd, buf, MAXLINE)) > 0) {
			char filename[MAXLINE];
			strcpy(filename, buf);
			filename[n-1] = '\0';
			printf ("Client request %s file!\n", filename);
			fd = open(filename, O_RDONLY);
			
			char tempbuf[1];
			while(read(fd, tempbuf, sizeof(tempbuf)) > 0) {
				//printf("%c", tempbuf[0]);
				write(connfd, tempbuf, sizeof(tempbuf));
			}
			printf("Send complete!\n");
			printf("wait connection ...\n");
			close(fd);
			close(connfd);
		}
	}
}

