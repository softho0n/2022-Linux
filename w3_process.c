#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main() {
	while(1) {
		char *cmd = NULL;
		size_t size = 0;
		getline(&cmd, &size, stdin);
		cmd[strlen(cmd)-1] = '\0';
		
		int i = 0;
		char *arg[11];
		char *ptr = strtok(cmd, " ");

		while(ptr != NULL) {
			arg[i] = ptr;
			i++;
			ptr = strtok(NULL, " ");
		}
		arg[i] = NULL;
		
		char path[100];
		sprintf(path, "/bin/%s", arg[0]);
		
		
		if (strcmp("quit", arg[0]) == 0) {
			exit(1);
		}	
		if (fork() == 0) {
			execv(path, arg);
		}
		wait(NULL);
	}
}
