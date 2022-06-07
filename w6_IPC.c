#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

int main(){
	char *cmd = NULL;
	size_t size = 0;
	char *arg[11];

	while(1){
		int i  = 0;
		getline(&cmd, &size, stdin);
		cmd[strlen(cmd)-1] = '\0';
		if(strlen(cmd) == 0) continue;

		char* ptr = strtok(cmd, " ");
		while(ptr != NULL){
			arg[i++] = ptr;
			ptr = strtok(NULL, " ");
		}
		arg[i] = NULL;
		if(strcmp("quit", arg[0]) == 0)	break;
				
		char path[100];
		sprintf(path, "/bin/%s", arg[0]);
	

		// Write code here!
		int flag = 0;
		for(int k=0; k<i; k++) {
			if(strcmp(">", arg[k]) == 0) {
				int f = open(arg[k+1], O_WRONLY|O_CREAT, 0666);
				char *newarg[11];
				for(int j=0; j<k; j++) {
					newarg[j] = arg[j];
				}
				newarg[k] = NULL;
				if(fork() == 0) {
					dup2(f, 1);
					execv(path, newarg);
				}
				wait(NULL);
				flag = 1;
				break;
			}
			else if(strcmp("<", arg[k]) == 0) {
				int f = open(arg[k+1], O_RDONLY);
				char *newarg[11];
				for(int j=0; j<k; j++) {
					newarg[j] = arg[j];
				}
				// print()
				newarg[k] = NULL;
				if(fork() == 0) {
					dup2(f, 0);
					execv(path, newarg);
				}
				wait(NULL);
				flag = 1;
				break;
			}
			else if(strcmp("|", arg[k]) == 0) {₩
				pid_t pid;
				char *first_cmd[11];
				char *second_cmd[11];
				for(int j=0; j<k; j++) {
					first_cmd[j] = arg[j];
				}
				first_cmd[k] = NULL;

				int idx = 0;
				for(int j=k+1; j<i; j++) {
					second_cmd[idx++] = arg[j];
				}
				second_cmd[idx] = NULL;

				int fd[2];

				if(pipe(fd) < 0) exit(1);
				
				if(fork() == 0) {
					close(fd[0]);
					dup2(fd[1], 1);
					execv(path, first_cmd);
				}

				wait(NULL);
				if(fork() == 0) {
					close(fd[1]);
					char path[100];
					sprintf(path, "/bin/%s", second_cmd[0]);
					dup2(fd[0], 0);
					execv(path, second_cmd);
				}
				
				close(fd[0]);
				close(fd[1]);
				wait(NULL);
				flag = 1;
				break;
			}
		}

		if(!flag) {
			if(fork() == 0) {
				execv(path, arg);
			}
			wait(NULL);
		}
		// Write code here!
	}
}




