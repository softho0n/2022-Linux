#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <pthread.h>

#define MAXLINE 80
#define MAXUSER 1024

int seats[256];
int userpassword[1024];
int current_login_status[1024];

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

typedef struct _query {
    int user;
    int action;
    int seat;
} query;

typedef struct _thread_full_data {
	query q;
	int fd;
} thread_data;

typedef struct _thread_data {
	int ret;
	int arr[256];
} thread_return_data;

void init_arr() {
	for(int i=0; i<256; i++) {
		seats[i] = -1;
	}

	for(int i=0; i<1024; i++) {
		userpassword[i] = -1;
		current_login_status[i] = -1;
	}
}

void *runner(void *arg) {
    thread_data *data = (thread_data *)arg;
    pthread_mutex_lock(&m);
	int ret = 1;
	thread_return_data *my_data = (thread_return_data*)malloc(sizeof(thread_return_data));

	if(data->q.action == 0 && data->q.user == 0 && data->q.seat == 0) {
		my_data->ret = -2;

		for(int i=0; i<256; i++) {
			my_data->arr[i] = seats[i];
		}

		current_login_status[data->fd] = -1;
	}
	else {
		switch (data->q.action)
		{
		case 1:
			/* 다른 클라이언트가 동일 유저로 접속하려고 하는 경우 또는 이미 로그인 되어있는데 또 시도하는 경우 */
			for(int i=0; i<1024; i++) {
				if(current_login_status[i] == data->q.user) {
					ret = -1;
					break;
				}
			}

			/* 다른 클라이언트에서 일단 현재 유저로 접속하지 않았음 */
			if(ret == 1) {
				/* 만약 현재 클라이언트에서 다른 유저가 접속되어있고, 로그아웃 하지 않았을 때 */
				if((current_login_status[data->fd] != data->q.user) && (current_login_status[data->fd] != -1)) {
					ret = -1;
				}
				else if((current_login_status[data->fd] == -1) && (userpassword[data->q.user] == -1)) {
					/* 만약 다른 유저도 없고, 로그인을 처음 할 경우 */
					current_login_status[data->fd] = data->q.user;
					userpassword[data->q.user] = data->q.seat;
					ret = 1;
				}
				else if((current_login_status[data->fd] == -1) && (userpassword[data->q.user] != -1)) {
					/* 만약 다른 유저도 없고, 패스워드로 로그인을 시도 할 경우 */
					if(userpassword[data->q.user] == data->q.seat) {
						/* 비밀번호 일치 */
						current_login_status[data->fd] = data->q.user;
						ret = 1;
					}
					else {
						/* 비밀번호 불일치 */
						ret = -1;
					}
				}
			}
			break;
		case 2:
			if(data->q.seat < 0 || data->q.seat > 255) {
				ret = -1;
			}
			else if(current_login_status[data->fd] == -1 || current_login_status[data->fd] != data->q.user) {
				ret = -1;
			}
			else if(seats[data->q.seat] == -1) {
				seats[data->q.seat] = data->q.user;
				ret = data->q.seat;
			}
			else {
				ret = -1;
			}
			break;
		case 3:
			if(current_login_status[data->fd] == -1 || current_login_status[data->fd] != data->q.user) {
				ret = -1;
			}
			else if(seats[data->q.seat] == -1 || seats[data->q.seat] != data->q.user) {
				ret = -1;
			}
			else {
				ret = data->q.seat;
			}
			break;
		case 4:
			if(current_login_status[data->fd] == -1 || current_login_status[data->fd] != data->q.user) {
				ret = -1;
			}
			else if(seats[data->q.seat] == -1 || seats[data->q.seat] != data->q.user) {
				ret = -1;
			}
			else {
				seats[data->q.seat] = -1;
				ret = data->q.seat;
			}
			break;
		case 5:
			if(current_login_status[data->fd] == -1 || current_login_status[data->fd] != data->q.user) {
				ret = -1;
			}
			else {
				current_login_status[data->fd] = -1;
				ret = 1;
			}
			break;
		default:
			break;
		}
		my_data->ret = ret;
	}
	
    pthread_mutex_unlock(&m);
    pthread_exit((void*)my_data);
}

int main (int argc, char *argv[]) {
	int n, listenfd, connfd, caddrlen, nbytes, fd;
	struct hostent *h;
	struct sockaddr_in saddr, caddr;
	char buf[MAXLINE];
	char chat[MAXLINE*2];
	char username[MAXUSER][MAXLINE];
	
	int port = atoi(argv[1]);

	// 1. Create listen socket
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		printf("socket() failed.\n");
		exit(1);
	}

	// generate listenfd socket address
	init_arr();
	memset((char *)&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET; 
	saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	saddr.sin_port = htons(port);

	// 2. bind socket to saddr
	if (bind(listenfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
		printf("bind() failed.\n");
		exit(2); 
	}

	// 3. Create listen queue with 5 clients
	if (listen(listenfd, 5) < 0) { 
		printf("listen() failed.\n"); 
		exit(3);
	}

	fd_set readset, activeset;
	FD_ZERO(&readset);
	FD_SET(listenfd, &readset);

	int fdmax = listenfd, fdnum;
	int current_user = 0;
	int num_thread = 1024;
	pthread_t *threads = malloc(num_thread * sizeof(pthread_t));
	pthread_attr_t attr;
	pthread_attr_init(&attr);


	while (1) {
		activeset = readset;

		if ((fdnum = select(FD_SETSIZE, &activeset, NULL, NULL, NULL)) < 0) {
			printf("select error");
			exit(0);
		}

		for (int i = 0; i < FD_SETSIZE; i++) {
			if (FD_ISSET(i, &activeset)) {
				if (i == listenfd) {
					if ((connfd = accept(listenfd, (struct sockaddr *)&caddr, (socklen_t *)&caddrlen)) < 0) {
						printf ("accept() failed.\n");
						continue;
					}
					FD_SET(connfd, &readset);
					if (fdmax < connfd) fdmax = connfd;
					// current_user += 1;
                   	// printf("Current Users %d\n", current_user);
				}
				else {
					// for newly arrived messages
					query q;
					if(read(i, &q, sizeof(q)) > 0) {
						thread_data *params = (thread_data *)malloc(sizeof(thread_data));
						thread_return_data *res = (thread_return_data *)malloc(sizeof(thread_return_data));
						params->q = q;
						params->fd = i;

						pthread_create(&threads[i], &attr, runner, (void *) params);
						pthread_join(threads[i], (void**)&res);
				
						if(res->ret == -2) {
							write(i, &(res->arr), sizeof(res->arr));
							
						}
						else {
							write(i, &(res->ret), sizeof(res->ret));
						}
						free(params);
						free(res);
					}
					else {
						// printf("connection terminated\n");
						current_user -= 1;
						FD_CLR(i, &readset);
						close(i);
					}
				}
			}
		}
	}
	pthread_mutex_destroy(&m);
	free(threads);
}