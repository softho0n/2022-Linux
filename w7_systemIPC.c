#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

struct msgbuf{
	long msgtype;
	char text[256];
};

void sigchld_handler (int sig) {
	exit(0);
}

int main(){
	int send_id_int, recv_id_int;
	char *send_id = NULL;
	char *recv_id = NULL;
	size_t size;
	printf("My id:");
	getline(&send_id, &size, stdin);
	send_id[size-1] ='\0';
	printf("Receiver's id:");
	getline(&recv_id, &size, stdin);
	recv_id[size-1] ='\0';
	key_t send_k = ftok(".", atoi(send_id));
	key_t recv_k = ftok(".", atoi(recv_id));

	int send_qid = msgget(send_k, IPC_CREAT | 0660);
	int recv_qid = msgget(recv_k, IPC_CREAT | 0660);
	pid_t pid;
	int flag = 1;
	if(send_qid == -1 || recv_qid == -1){
		perror("msgget error");
		exit(0);
	}

	signal(SIGCHLD, sigchld_handler);
	if((pid =fork ()) == 0){
		// Child - Receiver
		while(1) {
			struct msgbuf *msg = malloc(sizeof(struct msgbuf));
			if(msgrcv(recv_qid, (void *)msg, sizeof(struct msgbuf), 1, 0) == -1) {
				perror("msgrcv error");
				exit(0);
			}
			if(strcmp(msg->text, "quit\n") == 0) {
				printf("quit\n");
				printf("QUIT");
				exit(0);
			}
			printf("RECEIVED: %s", msg->text);
			free(msg);
		}
		return 0;
	}else{
		// Parent - Sender
		while(1) {
			struct msgbuf *msg = malloc(sizeof(struct msgbuf));
			msg->msgtype = 1;
			char *sendMsg = NULL;
			getline(&sendMsg, &size, stdin);
			strcpy(msg->text, sendMsg);
			
			if(msgsnd(send_qid, (void *)msg, sizeof(struct msgbuf), 0) == -1) {
				perror("msgsnd error");
				exit(0);
			}
			if(strcmp(msg->text, "quit\n") == 0) {
				printf("QUIT");
				kill(pid, SIGINT);
				exit(0);
			}
			free(msg);
		}
	}
}
