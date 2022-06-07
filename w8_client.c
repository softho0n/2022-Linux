#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define MAXLINE 80

int main (int argc, char *argv[]) {
	int n, cfd, fd, nbytes;
	struct hostent *h;
	struct sockaddr_in saddr;
	char buf[MAXLINE];
	char *host = argv[1];
	int port = atoi(argv[2]);

	/* Write your Code Here */
	if((cfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket() failedn");
		exit(1);
	}

	if((h = gethostbyname(host)) == NULL) {
		printf("invalid hostname %s\n", host);
		exit(2);
	}

	memset((char*)&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	memcpy((char *)&saddr.sin_addr.s_addr, (char *)h->h_addr, h->h_length);
	saddr.sin_port = htons(port);

	if(connect(cfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
		printf("connect() failed.\n");
		exit(3);
	}

	n = read(0, buf, MAXLINE);
	printf("Send request\n");
		
	char filename[MAXLINE];
	strcpy(filename, buf);
	int buf_len = strlen(buf);
	filename[n - 1] = '\0';
	fd = open(filename, O_CREAT | O_WRONLY, 0666);
	
	write(cfd, buf, n);
	
	printf("Receive file!\n");

	char tempbuf[1]; 
	while(read(cfd, tempbuf, sizeof(tempbuf)) > 0) {
		//printf("%c", tempbuf[0]);
		write(fd, tempbuf, sizeof(tempbuf));
	}
	close(cfd);
	close(fd);
}

