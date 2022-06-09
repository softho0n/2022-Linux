#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef struct _query {
    int user;
    int action;
    int seat;
} query;

int main (int argc, char *argv[]) {
    int client_socket = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    struct hostent *h;

    if (argc < 3) {
	printf("argv[1]: server address, argv[2]: port number\n");
	exit(1);
    }
    
    if ((h = gethostbyname(argv[1])) == NULL) {
        printf("invalid hostname %s\n", argv[1]);
        exit(2);
    } 

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    memcpy((char *)&server_addr.sin_addr.s_addr, (char *)h->h_addr, h->h_length);

    if (connect(client_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1) {
	printf("Connection failed\n");
	exit(1);
    }

    while (1) {
	query q;
	scanf("%d %d %d", &q.user, &q.action, &q.seat);
	write(client_socket, &q, sizeof(q));
	int ret;
	if (!(q.user | q.action | q.seat))
	{
	    int arr[256];
	    read(client_socket, arr, sizeof(arr));
	    int i;
	    for(i=0;i<256;i++) printf("%d ", arr[i]);
	    printf("\n");
	    break;
	}
	else{
		read(client_socket, &ret, sizeof(ret));
		printf("%d\n", ret);
	}
    }
}
