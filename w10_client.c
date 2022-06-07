#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

#define MAXLINE 80

void *handle_recv(void *arg){
	// WRITE YOUR CODE : print received message from the server
	pthread_detach(pthread_self());
	while(1) {
		int n;
		char buf[MAXLINE];
		int connfd = *((int*)arg);
		while ((n = read(connfd, buf, MAXLINE)) > 0)
			write(1, buf, n);
		close(connfd);
	}

	return NULL;
	// WRITE YOUR CODE : print received message from the server
}


int main (int argc, char *argv[]) {
	int n, cfd, fd, nbytes;
	struct hostent *h;
	struct sockaddr_in saddr;
	char buf[MAXLINE];
	char *host = argv[1];
	int port = atoi(argv[2]);

	if ((cfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket() failed.\n"); 
		exit(1);
	}

	if ((h = gethostbyname(host)) == NULL) {
		printf("invalid hostname %s\n", host); 
		exit(2);
	}   
	
	memset((char *)&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	memcpy((char *)&saddr.sin_addr.s_addr, (char *)h->h_addr, h->h_length);
	saddr.sin_port = htons(port);
	
	char username[MAXLINE];

	write(1, "Insert your name : ", 20);
	n = read(0, username, MAXLINE);
	if (connect(cfd,(struct sockaddr *)&saddr,sizeof(saddr)) < 0) {
		printf("connect() failed.\n");
		exit(3);
	}
	pthread_t tid;
	int *connfdp;
	connfdp = &cfd;
	pthread_create(&tid, NULL, handle_recv, connfdp);

	// WRITE YOUR CODE : send username to the server
	// WRITE YOUR CODE : send message from SDTIN to the server
	username[n-1] = '\0';
	write(cfd, username, n);
	while(1) {
		n = read(0, buf, MAXLINE);
		buf[n-1] = '\0';
		if(strcmp(buf, "quit") == 0) {
			exit(1);
		}
		else {
			write(cfd, buf, n);
		}
	}
	// WRITE YOUR CODE : send username to the server
	// WRITE YOUR CODE : send message from SDTIN to the server
	close(cfd);
}



