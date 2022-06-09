#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define LMAX 205

int return_val = 0;

void sigint_handler(int sig) {

}

void sigtstp_handler(int sig) {
	int status;
	waitpid(-1, &status, WNOHANG | WUNTRACED);
	if (WIFSTOPPED(status))
	{
		kill(0, SIGKILL);
	} 
}

int cmd_type_checking(char *cmd) {
	int type_of_cmd = -1;
	if((strcmp(cmd, "ls") == 0) || (strcmp(cmd, "man") == 0) || (strcmp(cmd, "grep") == 0) || (strcmp(cmd, "sort") == 0) || (strcmp(cmd, "bc") == 0)) {
		type_of_cmd = 1;
	}
	else if(strstr(cmd, "./") != NULL) {
		type_of_cmd = 2;
	}
	else if((strcmp(cmd, "head") == 0) || (strcmp(cmd, "tail") == 0) || (strcmp(cmd, "cat") == 0)) {
		type_of_cmd = 3;
	}
	else if((strcmp(cmd, "mv") ==0) || (strcmp(cmd, "rm") == 0) || (strcmp(cmd, "cp") == 0) || (strcmp(cmd, "cd") == 0)) {
		type_of_cmd = 4;
	}
	else if((strcmp(cmd, "pwd") == 0) || (strcmp(cmd, "exit") == 0)) {
		type_of_cmd = 5;
	}
	return type_of_cmd;
}

void execute_command_without_pipeline(char **cmds, int redirection_count, int cmds_len) {
	char path[100];
	int cmd_type = cmd_type_checking(*cmds);
	if(cmd_type == 2) {
		sprintf(path, "%s", cmds[0]);
	}
	else sprintf(path, "/bin/%s", cmds[0]);

	if(redirection_count == 2) { // command < [filename] > [filename]
		int first_index_of_redirection_char, second_index_of_redirection_char;
		int can_overwrite = 0;

		for(int i=0; i<cmds_len; i++) {
			if(strcmp("<", cmds[i]) == 0) {
				first_index_of_redirection_char = i;
			}
			else if(strcmp(">", cmds[i]) == 0) {
				second_index_of_redirection_char = i;
			}
			else if(strcmp(">>", cmds[i]) == 0) {
				second_index_of_redirection_char = i;
				can_overwrite = 1;
			}
		}

		char *temp_cmds_buf[LMAX];
		int temp_cmds_len = 0;
		for(int j=0; j<first_index_of_redirection_char; j++) {
			temp_cmds_buf[j] = cmds[j];
			temp_cmds_len += 1;
		}
		temp_cmds_buf[first_index_of_redirection_char] = NULL;

		int input_fd  = open(cmds[first_index_of_redirection_char + 1], O_RDONLY);
		int output_fd;
		if(can_overwrite) {
			output_fd = open(cmds[second_index_of_redirection_char + 1], O_WRONLY | O_APPEND, 0666);
		}
		else {
			output_fd = open(cmds[second_index_of_redirection_char + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
		}
		
		if(strcmp(temp_cmds_buf[0], "head") == 0) {
			if(input_fd < 0) {
				printf("swsh: No such file\n");
			}
			else {
				int read_cnt = 0;
				int stdout_ = dup(1);

				if(temp_cmds_len == 2) { // head text.txt
					read_cnt = 10;
					input_fd = open(temp_cmds_buf[1], O_RDONLY);
				}
				else { // head -n 15 test.txt
					read_cnt = atoi(temp_cmds_buf[2]);
					input_fd = open(temp_cmds_buf[3], O_RDONLY);
				}
				
				
				char read_buf[1];
				int last_line_check = 1;

				dup2(output_fd, 1);
				while(read(input_fd, read_buf, sizeof(read_buf)) > 0) {
					if(read_buf[0] == '\n') {
						read_cnt -= 1;
						last_line_check = 0;
						printf("\n");
					}
					else {
						printf("%c", read_buf[0]);
						last_line_check = 1;
					}
					if(read_cnt == 0) break;
				}

				if(last_line_check) {
					printf("\n");
				}

				close(input_fd);
				dup2(stdout_, 1);
				close(stdout_);
				close(output_fd);
			}
		}
		else if(strcmp(temp_cmds_buf[0], "tail") == 0) {
			if(input_fd < 0) {
				printf("swsh: No such file\n");
			}
			else {
				int read_cnt = 0;
				int stdout_ = dup(1);

				if(temp_cmds_len == 1) {
					read_cnt = 10;
					input_fd = open(temp_cmds_buf[1], O_RDONLY);
				}
				else {
					read_cnt = atoi(temp_cmds_buf[2]);
					input_fd = open(temp_cmds_buf[3], O_RDONLY);
				}

				int total_line_cnt = 0;
				int last_line_add_flag = 1;
				char read_buf[1];
				while(read(input_fd, read_buf, sizeof(read_buf)) > 0) {
					if(read_buf[0] == '\n') {
						total_line_cnt += 1;
						last_line_add_flag = 0;
					}
					else {
						last_line_add_flag = 1;
					}
				}

				if(last_line_add_flag) total_line_cnt += 1;

				int last_line_check = 1;
				lseek(input_fd, 0, SEEK_SET);
				dup2(output_fd, 1);
				if(total_line_cnt <= read_cnt) {
					while(read(input_fd, read_buf, sizeof(read_buf)) > 0) {
						if(read_buf[0] == '\n') {
							printf("\n");
							last_line_check = 0;
						}
						else {
							printf("%c", read_buf[0]);
							last_line_check = 1;
						}
					}

					if(last_line_check) {
						printf("\n");
					}
				}
				else {
					int first_read_cnt = total_line_cnt - read_cnt;
					while(read(input_fd, read_buf, sizeof(read_buf)) > 0) {
						if(read_buf[0] == '\n') {
							first_read_cnt -= 1;
						}
						if(first_read_cnt == 0) break;
					}

					while(read(input_fd, read_buf, sizeof(read_buf)) > 0) {
						if(read_buf[0] == '\n') {
							printf("\n");
							last_line_check = 0;
						}
						else {
							printf("%c", read_buf[0]);
							last_line_check = 1;
						}
					}

					if(last_line_check) {
						printf("\n");
					}
				}
				close(input_fd);
				dup2(stdout_, 1);
				close(stdout_);
				close(output_fd);
			}
		}
		else if(strcmp(temp_cmds_buf[0], "cat") == 0) {
			if(input_fd < 0) {
				printf("swsh: No such file\n");
			}
			else {
				char read_buf[1];
				int stdout_ = dup(1);
				dup2(output_fd, 1);
				input_fd = open(temp_cmds_buf[1], O_RDONLY);
				while(read(input_fd, read_buf, sizeof(read_buf)) > 0) {
					printf("%c", read_buf[0]);
				}
				close(input_fd);
				dup2(stdout_, 1);
				close(stdout_);
				close(output_fd);
			}
		}
		else if(strcmp(temp_cmds_buf[0], "mv") == 0) {
			/* DO NOTHING, DONT HAVE OUTPUT & INPUT */
		}
		else if(strcmp(temp_cmds_buf[0], "rm") == 0) {
			/* DO NOTHING, DONT HAVE OUTPUT & INPUT */
		}
		else if(strcmp(temp_cmds_buf[0], "cp") == 0) {
			/* DO NOTHING, DONT HAVE OUTPUT & INPUT */
		}
		else if(strcmp(temp_cmds_buf[0], "cd") == 0) {
			/* DO NOTHING, DONT HAVE OUTPUT & INPUT */
		}
		else if(strcmp(temp_cmds_buf[0], "pwd") == 0) {
			/* DO NOTHING, DONT HAVE OUTPUT & INPUT */
		}
		else if(strcmp(temp_cmds_buf[0], "exit") == 0) {
			/* DO NOTHING, DONT HAVE OUTPUT & INPUT */
		}
		else if(cmd_type == 1 || cmd_type == 2){
			if((strcmp(cmds[0], "sort") == 0) || (strcmp(cmds[0], "man") == 0) || (strcmp(cmds[0], "bc") == 0)) {
				sprintf(path, "/usr/bin/%s", cmds[0]);
			}
			pid_t pid;
			if((pid = fork()) == 0) {
				setpgid(0, getpgrp());
				dup2(input_fd, 0);
				dup2(output_fd, 1);
				execv(path, temp_cmds_buf);
			}
			wait(NULL);
		}
		else {
			fprintf(stderr, "swsh: Command not found\n");
		}
	}
	else if(redirection_count == 1) { // command < [filename] or command > [filename]
		int left_arrow_redirection_flag = 1; //  < (1) > (2) >> (3)
		int index_of_redirection_char = 0;
		for(int i=0; i<cmds_len; i++) {
			if(strcmp("<", cmds[i]) == 0) {
				index_of_redirection_char = i;
				break;
			}
			else if(strcmp(">", cmds[i]) == 0) {
				left_arrow_redirection_flag = 2;
				index_of_redirection_char = i;
				break;
			}
			else if(strcmp(">>", cmds[i]) == 0) {
				left_arrow_redirection_flag = 3;
				index_of_redirection_char = i;
				break;
			}
		}
		char *temp_cmds_buf[LMAX];
		int temp_cmds_len = 0;
		for(int j=0; j<index_of_redirection_char; j++) {
			temp_cmds_buf[j] = cmds[j];
			temp_cmds_len += 1;
		}
		temp_cmds_buf[index_of_redirection_char] = NULL;
		
		if(left_arrow_redirection_flag == 1) { // command < [filename]
			int fd = open(cmds[index_of_redirection_char + 1], O_RDONLY);
			if(strcmp(temp_cmds_buf[0], "head") == 0) {
				if(fd < 0) {
					printf("swsh: No such file\n");
				}
				else {
					int read_cnt = 0;
					if(temp_cmds_len == 2) {
						read_cnt = 10;
						fd = open(temp_cmds_buf[1], O_RDONLY);
					}
					else {
						read_cnt = atoi(temp_cmds_buf[2]);
						fd = open(temp_cmds_buf[3], O_RDONLY);
					}

					char read_buf[1];
					int last_line_check = 1;

					while(read(fd, read_buf, sizeof(read_buf)) > 0) {
						if(read_buf[0] == '\n') {
							read_cnt -= 1;
							last_line_check = 0;
							printf("\n");
						}
						else {
							printf("%c", read_buf[0]);
							last_line_check = 1;
						}

						if(read_cnt == 0) break;
					}

					if(last_line_check) {
						printf("\n");
					}
					close(fd);
				}
			}
			else if(strcmp(temp_cmds_buf[0], "tail") == 0) {
				/* DO NOTHING, DONT HAVE OUTPUT */
				if(fd < 0) {
					printf("swsh: No such file\n");
				}
				else {
					int read_cnt = 0;
					if(temp_cmds_len == 2) {
						read_cnt = 10;
						fd = open(temp_cmds_buf[1], O_RDONLY);
					}
					else {
						read_cnt = atoi(temp_cmds_buf[2]);
						fd = open(temp_cmds_buf[3], O_RDONLY);
					}

					int total_line_cnt = 0;
					int last_line_add_flag = 1;
					char read_buf[1];
					while(read(fd, read_buf, sizeof(read_buf)) > 0) {
						if(read_buf[0] == '\n') {
							total_line_cnt += 1;
							last_line_add_flag = 0;
						}
						else {
							last_line_add_flag = 1;
						}
					}

					if(last_line_add_flag) total_line_cnt += 1;

					int last_line_check = 1;
					lseek(fd, 0, SEEK_SET);
					if(total_line_cnt <= read_cnt) { /* real all */
						while(read(fd, read_buf, sizeof(read_buf)) > 0) {
							if(read_buf[0] == '\n') {
								printf("\n");
								last_line_check = 0;
							}
							else {
								printf("%c", read_buf[0]);
								last_line_check = 1;
							}
						}

						if(last_line_check) {
							printf("\n");
						}
					}
					else {
						int first_read_cnt = total_line_cnt - read_cnt;
						while(read(fd, read_buf, sizeof(read_buf)) > 0) {
							if(read_buf[0] == '\n') {
								first_read_cnt -= 1;
							}
							if(first_read_cnt == 0) break;
						}

						while(read(fd, read_buf, sizeof(read_buf)) > 0) {
							if(read_buf[0] == '\n') {
								printf("\n");
								last_line_check = 0;
							}
							else {
								printf("%c", read_buf[0]);
								last_line_check = 1;
							}
						}

						if(last_line_check) {
							printf("\n");
						}
					}
				}
			}
			else if(strcmp(temp_cmds_buf[0], "cat") == 0) {
				/* DO NOTHING, DONT HAVE OUTPUT */
				if(fd < 0) {
					printf("swsh: No such file\n");
				}
				else {
					char read_buf[1];
					fd = open(temp_cmds_buf[1], O_RDONLY);
					int newline_flag = 0;
					while(read(fd, read_buf, sizeof(read_buf)) > 0) {
						printf("%c", read_buf[0]);
						if(read_buf[0] == '\n') {
							newline_flag = 1;
						}
						else {
							newline_flag = 0;
						}
					}

					if(!newline_flag) {
						printf("\n");
					}
					close(fd);
				}
			}
			else if(strcmp(temp_cmds_buf[0], "mv") == 0) {
				/* DO NOTHING, DONT HAVE OUTPUT */
			}
			else if(strcmp(temp_cmds_buf[0], "rm") == 0) {
				/* DO NOTHING, DONT HAVE OUTPUT */
			}
			else if(strcmp(temp_cmds_buf[0], "cp") == 0) {
				/* DO NOTHING, DONT HAVE OUTPUT */
			}
			else if(strcmp(temp_cmds_buf[0], "cd") == 0) {
				/* DO NOTHING, DONT HAVE OUTPUT */
			}
			else if(strcmp(temp_cmds_buf[0], "pwd") == 0) {
				/* DO NOTHING, DONT HAVE OUTPUT */
			}
			else if(strcmp(temp_cmds_buf[0], "exit") == 0) {
				/* DO NOTHING, DONT HAVE OUTPUT */
			}
			else if(cmd_type == 1 || cmd_type == 2) {
				if((strcmp(temp_cmds_buf[0], "sort") == 0) || (strcmp(temp_cmds_buf[0], "man") == 0) || (strcmp(temp_cmds_buf[0], "bc") == 0)) {
					sprintf(path, "/usr/bin/%s", temp_cmds_buf[0]);
				}
				if(fd < 0) {
					printf("swsh: No such file\n");
				}
				else {
					pid_t pid;
					if((pid = fork()) == 0) {
						setpgid(0, getpgrp());
						dup2(fd, 0);
						execv(path, temp_cmds_buf);
					}
					wait(NULL);
				}
			}
			else {
				fprintf(stderr, "swsh: Command not found\n");
			}
		}
		else if(left_arrow_redirection_flag >= 2) { // command > [filename]
			int fd;
			if(left_arrow_redirection_flag == 2) {
				fd = open(cmds[index_of_redirection_char + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
			}
			else if(left_arrow_redirection_flag == 3) {
				fd = open(cmds[index_of_redirection_char + 1], O_WRONLY | O_APPEND, 0666);
			}
			if(strcmp(temp_cmds_buf[0], "head") == 0) {
				int read_cnt = 0;
				int read_fd;
				int stdout_ = dup(1);

				if(temp_cmds_len == 2) { /* No OPTIONS */
					read_fd = open(temp_cmds_buf[1], O_RDONLY);
					read_cnt = 10;
				}
				else { /* There is OPTION * such as -n 10 */
					read_fd = open(temp_cmds_buf[3], O_RDONLY);
					read_cnt = atoi(temp_cmds_buf[2]);
				}

				char read_buf[1];
				int last_line_check = 1;

				dup2(fd, 1);
				while(read(read_fd, read_buf, sizeof(read_buf)) > 0) {
					if(read_buf[0] == '\n') {
						read_cnt -= 1;
						last_line_check = 0;
						printf("\n");
					}
					else {
						printf("%c", read_buf[0]);
						last_line_check = 1;
					}

					if(read_cnt == 0) break;
				}

				if(last_line_check) {
					printf("\n");
				}
				close(read_fd);
				dup2(stdout_, 1);
				close(stdout_);
				close(fd);
			}
			else if(strcmp(temp_cmds_buf[0], "tail") == 0) {
				int read_cnt = 0;
				int read_fd;
				int stdout_ = dup(1);
				if(temp_cmds_len == 2) { /* No OPTIONS */
					read_fd = open(temp_cmds_buf[1], O_RDONLY);
					read_cnt = 10;
				}
				else { /* There is OPTION * such as -n 10 */
					read_fd = open(temp_cmds_buf[3], O_RDONLY);
					read_cnt = atoi(temp_cmds_buf[2]);
				}

				int total_line_cnt = 0;
				int last_line_add_flag = 1;
				char read_buf[1];
				while(read(read_fd, read_buf, sizeof(read_buf)) > 0) {
					if(read_buf[0] == '\n') {
						total_line_cnt += 1;
						last_line_add_flag = 0;
					}
					else {
						last_line_add_flag = 1;
					}
				}

				if(last_line_add_flag) total_line_cnt += 1;

				int last_line_check = 1;
				lseek(read_fd, 0, SEEK_SET);
				dup2(fd, 1);
				if(total_line_cnt <= read_cnt) { /* real all */
					while(read(read_fd, read_buf, sizeof(read_buf)) > 0) {
						if(read_buf[0] == '\n') {
							printf("\n");
							last_line_check = 0;
						}
						else {
							printf("%c", read_buf[0]);
							last_line_check = 1;
						}
					}

					if(last_line_check) {
						printf("\n");
					}
				}
				else {
					int first_read_cnt = total_line_cnt - read_cnt;
					while(read(read_fd, read_buf, sizeof(read_buf)) > 0) {
						if(read_buf[0] == '\n') {
							first_read_cnt -= 1;
						}
						if(first_read_cnt == 0) break;
					}

					while(read(read_fd, read_buf, sizeof(read_buf)) > 0) {
						if(read_buf[0] == '\n') {
							printf("\n");
							last_line_check = 0;
						}
						else {
							printf("%c", read_buf[0]);
							last_line_check = 1;
						}
					}

					if(last_line_check) {
						printf("\n");
					}
				}
				close(read_fd);
				dup2(stdout_, 1);
				close(stdout_);
				close(fd);
			}
			else if(strcmp(temp_cmds_buf[0], "cat") == 0) {
				int read_fd = open(temp_cmds_buf[1], O_RDONLY);
				char read_buf[1];
				int stdout_ = dup(1);
				dup2(fd, 1);
				int newline_flag = 0;
				while(read(read_fd, read_buf, sizeof(read_buf)) > 0) {
					printf("%c", read_buf[0]);
					if(read_buf[0] == '\n') {
						newline_flag = 1;
					}
					else {
						newline_flag = 0;
					}
				}

				if(!newline_flag) {
					printf("\n");
				}

				close(read_fd);
				dup2(stdout_, 1);
				close(stdout_);
				close(fd);
			}
			else if(strcmp(temp_cmds_buf[0], "mv") == 0) {
				/* DO NOTHING, DONT HAVE OUTPUT */
			}
			else if(strcmp(temp_cmds_buf[0], "rm") == 0) {
				/* DO NOTHING, DONT HAVE OUTPUT */
			}
			else if(strcmp(temp_cmds_buf[0], "cp") == 0) {
				/* DO NOTHING, DONT HAVE OUTPUT */
			}
			else if(strcmp(temp_cmds_buf[0], "cd") == 0) {
				/* DO NOTHING, DONT HAVE OUTPUT */
			}
			else if(strcmp(temp_cmds_buf[0], "pwd") == 0) {
				int stdout_ = dup(1);
				
				char *cwd = getcwd(NULL, 1024);
				dup2(fd, 1);
				printf("%s\n", cwd);

				dup2(stdout_, 1);
				close(fd);
				close(stdout_);
			}
			else if(strcmp(temp_cmds_buf[0], "exit") == 0) {
				/* DO NOTHING, DONT HAVE OUTPUT, STDERR */
			}
			else if(cmd_type == 1 || cmd_type == 2) {
				if((strcmp(temp_cmds_buf[0], "sort") == 0) || (strcmp(temp_cmds_buf[0], "man") == 0) || (strcmp(temp_cmds_buf[0], "bc") == 0)) {
					sprintf(path, "/usr/bin/%s", temp_cmds_buf[0]);
				}
				pid_t pid;
				if((pid = fork()) == 0) {
					setpgid(0, getpgrp());
					dup2(fd, 1);
					execv(path, temp_cmds_buf);
				}
				wait(NULL);
			}
			else {
				fprintf(stderr, "swsh: Command not found\n");
			}
		}
	}
	else if(redirection_count == 0) { // command
		if(strcmp(cmds[0], "head") == 0) {
			int read_cnt = 0;
			int fd;
			if(cmds_len == 2) { /* No OPTIONS */
				fd = open(cmds[1], O_RDONLY);
				read_cnt = 10;
			}
			else { /* There is OPTION * such as -n 10 */
				fd = open(cmds[3], O_RDONLY);
				read_cnt = atoi(cmds[2]);
			}

			char read_buf[1];
			int last_line_check = 1;
			while(read(fd, read_buf, sizeof(read_buf)) > 0) {
				if(read_buf[0] == '\n') {
					read_cnt -= 1;
					last_line_check = 0;
					printf("\n");
				}
				else {
					printf("%c", read_buf[0]);
					last_line_check = 1;
				}

				if(read_cnt == 0) break;
			}

			if(last_line_check) {
				printf("\n");
			}
			close(fd);
		}
		else if(strcmp(cmds[0], "tail") == 0) {
			int read_cnt = 0;
			int fd;
			if(cmds_len == 2) { /* No OPTIONS */
				fd = open(cmds[1], O_RDONLY);
				read_cnt = 10;
			}
			else { /* There is OPTION * such as -n 10 */
				fd = open(cmds[3], O_RDONLY);
				read_cnt = atoi(cmds[2]);
			}

			int total_line_cnt = 0;
			int last_line_add_flag = 1;
			char read_buf[1];
			while(read(fd, read_buf, sizeof(read_buf)) > 0) {
				if(read_buf[0] == '\n') {
					total_line_cnt += 1;
					last_line_add_flag = 0;
				}
				else {
					last_line_add_flag = 1;
				}
			}

			if(last_line_add_flag) total_line_cnt += 1;

			int last_line_check = 1;
			lseek(fd, 0, SEEK_SET);
			if(total_line_cnt <= read_cnt) { /* real all */
				while(read(fd, read_buf, sizeof(read_buf)) > 0) {
					if(read_buf[0] == '\n') {
						printf("\n");
						last_line_check = 0;
					}
					else {
						printf("%c", read_buf[0]);
						last_line_check = 1;
					}
				}

				if(last_line_check) {
					printf("\n");
				}
			}
			else {
				int first_read_cnt = total_line_cnt - read_cnt;
				while(read(fd, read_buf, sizeof(read_buf)) > 0) {
					if(read_buf[0] == '\n') {
						first_read_cnt -= 1;
					}
					if(first_read_cnt == 0) break;
				}

				while(read(fd, read_buf, sizeof(read_buf)) > 0) {
					if(read_buf[0] == '\n') {
						printf("\n");
						last_line_check = 0;
					}
					else {
						printf("%c", read_buf[0]);
						last_line_check = 1;
					}
				}

				if(last_line_check) {
					printf("\n");
				}
			}
		}
		else if(strcmp(cmds[0], "cat") == 0) {
			int fd = open(cmds[1], O_RDONLY);
			char read_buf[1];
			int newline_flag = 0;
			while(read(fd, read_buf, sizeof(read_buf)) > 0) {
				printf("%c", read_buf[0]);
				if(read_buf[0] == '\n') {
					newline_flag = 1;
				}
				else {
					newline_flag = 0;
				}
			}

			if(!newline_flag) {
				printf("\n");
			}
			close(fd);
		}
		else if(strcmp(cmds[0], "mv") == 0) {
			int mv_result = rename(cmds[1], cmds[2]);
			if(errno == EACCES) {
				fprintf(stderr, "%s: %s\n", cmds[0], "Permission denied");
			}
			else if(errno == EISDIR) {
				fprintf(stderr, "%s: %s\n", cmds[0], "Is a directory");
			}
			else if(errno == ENOENT) {
				fprintf(stderr, "%s: %s\n", cmds[0], "No such file or directory");
			}
			else if(errno == ENOTDIR) {
				fprintf(stderr, "%s: %s\n", cmds[0], "Not a directory");
			}
			else if(errno == EPERM) {
				fprintf(stderr, "%s: %s\n", cmds[0], "Permission denied");
			}
			else if(mv_result != 0) {
				fprintf(stderr, "%s: Error occurred: %d\n", cmds[0], errno);
			}
		}
		else if(strcmp(cmds[0], "rm") == 0) {
			int rm_result = unlink(cmds[1]);
			if(errno == EACCES) {
				fprintf(stderr, "%s: %s\n", cmds[0], "Permission denied");
			}
			else if(errno == EISDIR) {
				fprintf(stderr, "%s: %s\n", cmds[0], "Is a directory");
			}
			else if(errno == ENOENT) {
				fprintf(stderr, "%s: %s\n", cmds[0], "No such file or directory");
			}
			else if(errno == ENOTDIR) {
				fprintf(stderr, "%s: %s\n", cmds[0], "Not a directory");
			}
			else if(errno == EPERM) {
				fprintf(stderr, "%s: %s\n", cmds[0], "Permission denied");
			}
			else if(rm_result != 0) {
				fprintf(stderr, "%s: Error occurred: %d\n", cmds[0], errno);
			}
		}
		else if(strcmp(cmds[0], "cp") == 0) {
			int original_fd = open(cmds[1], O_RDONLY);
			int copy_fd     = open(cmds[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
			
			char read_buf[1];
			while(read(original_fd, read_buf, sizeof(read_buf)) > 0) {
				write(copy_fd, read_buf, sizeof(read_buf));
			}

			close(original_fd);
			close(copy_fd);
		}
		else if(strcmp(cmds[0], "cd") == 0) {
			int cd_result = chdir(cmds[1]);

			if(errno == EACCES) {
				fprintf(stderr, "%s: %s\n", cmds[0], "Permission denied");
			}
			else if(errno == EISDIR) {
				fprintf(stderr, "%s: %s\n", cmds[0], "Is a directory");
			}
			else if(errno == ENOENT) {
				fprintf(stderr, "%s: %s\n", cmds[0], "No such file or directory");
			}
			else if(errno == ENOTDIR) {
				fprintf(stderr, "%s: %s\n", cmds[0], "Not a directory");
			}
			else if(errno == EPERM) {
				fprintf(stderr, "%s: %s\n", cmds[0], "Permission denied");
			}
			else if(cd_result != 0){
				fprintf(stderr, "%s: Error occurred: %d\n", cmds[0], errno);
			}
		}
		else if(strcmp(cmds[0], "pwd") == 0) {
			char *cwd = getcwd(NULL, 1024);
			printf("%s\n", cwd);
		}
		else if(strcmp(cmds[0], "exit") == 0) {
			if(cmds_len == 2) {
				return_val = atoi(cmds[1]);
			}

			fprintf(stderr, "exit");
			exit(0);
		}
		else if(cmd_type == 1 || cmd_type == 2){
			if((strcmp(cmds[0], "sort") == 0) || (strcmp(cmds[0], "man") == 0) || (strcmp(cmds[0], "bc") == 0)) {
				sprintf(path, "/usr/bin/%s", cmds[0]);
			}
			pid_t pid;
			if((pid = fork()) == 0) {
				setpgid(0, getpgrp());
				execv(path, cmds);
			}
			wait(NULL);
			if(cmd_type == 2) {
				printf("\n");
			}
		}
		else {
			fprintf(stderr, "swsh: Command not found\n");
		}
	}
}

int input_valid_check(char **input, int input_len) {
	int flag = 1;
	for(int i=0; i<input_len; i++) {
		int type_cmd = cmd_type_checking(input[i]);
		if(type_cmd == -1) {
			flag = 0;
			break;
		}
	}
	return flag;
}

void execute_command_with_pipeline(char **cmd, int redirection_count, int pipeline_count, int cmds_len, char *cmdstr) {
	char *ptr = strtok(cmdstr, "|");
	char *arg[LMAX];
	char *temp_cmd[LMAX];
	char *valid_check[LMAX];
	char ***cmds = (char ***)malloc(sizeof(char **) * LMAX);

	int i = 0;
	while(ptr != NULL) {
		arg[i++] = ptr;
		ptr = strtok(NULL, "|");
	}

	int total_index = 0;
	int prev_index = 0;
	for(int j=0; j<i; j++) {
		char *cmd_token = strtok(arg[j], " ");
		valid_check[j] = cmd_token;
		while(cmd_token != NULL) {
			temp_cmd[total_index++] = cmd_token;
			cmd_token = strtok(NULL, " ");
		}
		temp_cmd[total_index++] = NULL;

		cmds[j] = temp_cmd + prev_index;
		prev_index = total_index;
	}
	cmds[i] = NULL;

	if(input_valid_check(valid_check, i)) {
		if(redirection_count == 0) { /* Only PIPELINE */
			int current_fd = 0;
			int input_fd = 0;
			char path[100];
			int status = 0;
			int flag = 0;
			int group_id = 0;
			
			while(*cmds != NULL) {
				int fd[2] = {0, };
				int pid;
				
				pipe(fd);

				int cmd_type = cmd_type_checking(*(cmds)[0]);
				if(cmd_type == 2) {
					sprintf(path, "%s", *(cmds)[0]);
				}
				else {
					if((strcmp(*(cmds)[0], "sort") == 0) || (strcmp(*(cmds)[0], "man") == 0) || (strcmp(*(cmds)[0], "bc") == 0)) {
						sprintf(path, "/usr/bin/%s", *(cmds)[0]);
					}
					else sprintf(path, "/bin/%s", *(cmds)[0]);
				}

				pid = fork();
				if(pid != 0) {
					if(status == 0) {
						group_id = pid;
					}
					setpgid(pid, group_id);
					status += 1;
					
					wait(NULL);
					close(fd[1]);
					current_fd = fd[0];
					cmds += 1;
					flag += 1;
				}
				else {
					if(status == 0) {
						group_id = getpid();
					}
					setpgid(0, group_id);

					if(*(cmds + 1) != NULL) {
						dup2(fd[1], 1);
					}
					if(flag != 0) {
						dup2(current_fd, 0);
					}
					if(strcmp(*(cmds)[0], "head") == 0) {
						close(fd[0]);
						int read_cnt = 0;
						int f;
						char **curline = *(cmds);

						if(*(curline + 2) == NULL) { /* No OPTIONS */
							f = open(*(curline + 1), O_RDONLY);
							read_cnt = 10;
						}
						else if(*(curline + 4) == NULL) {
							f = open(*(curline + 3), O_RDONLY);
							read_cnt = atoi(*(curline + 2));
						}

						char read_buf[1];
						int last_line_check = 1;
						while(read(f, read_buf, sizeof(read_buf)) > 0) {
							if(read_buf[0] == '\n') {
								read_cnt -= 1;
								last_line_check = 0;
								printf("\n");
							}
							else {
								printf("%c", read_buf[0]);
								last_line_check = 1;
							}

							if(read_cnt == 0) break;
						}

						
						if(last_line_check) {
							printf("\n");
						}
						
						close(f);
						exit(0);
					}
					else if(strcmp(*(cmds)[0], "tail") == 0) {
						close(fd[0]);
						int read_cnt = 0;
						int f;
						char **curline = *(cmds);
						if(*(curline + 2) == NULL) { /* No OPTIONS */
							f = open(*(curline + 1), O_RDONLY);
							read_cnt = 10;
						}
						else if(*(curline + 4) == NULL) {
							f = open(*(curline + 3), O_RDONLY);
							read_cnt = atoi(*(curline + 2));
						}

						int total_line_cnt = 0;
						int last_line_add_flag = 1;
						char read_buf[1];

						while(read(f, read_buf, sizeof(read_buf)) > 0) {
							if(read_buf[0] == '\n') {
								total_line_cnt += 1;
								last_line_add_flag = 0;
							}
							else {
								last_line_add_flag = 1;
							}
						}
						
						lseek(f, 0, SEEK_SET);
						if(last_line_add_flag) total_line_cnt += 1;

						int last_line_check = 1;
						
						if(total_line_cnt <= read_cnt) { 
							while(read(f, read_buf, sizeof(read_buf)) > 0) {
								if(read_buf[0] == '\n') {
									printf("\n");
									last_line_check = 0;
								}
								else {
									printf("%c", read_buf[0]);
									last_line_check = 1;
								}
							}

							if(last_line_check) {
								printf("\n");
							}
						}
						else {
							int first_read_cnt = total_line_cnt - read_cnt;
							while(read(f, read_buf, sizeof(read_buf)) > 0) {
								if(read_buf[0] == '\n') {
									first_read_cnt -= 1;
								}
								if(first_read_cnt == 0) break;
							}

							while(read(f, read_buf, sizeof(read_buf)) > 0) {
								if(read_buf[0] == '\n') {
									printf("\n");
									last_line_check = 0;
								}
								else {
									printf("%c", read_buf[0]);
									last_line_check = 1;
								}
							}

							if(last_line_check) {
								printf("\n");
							}
						}
						close(f);
						exit(1);
					}
					else if(strcmp(*(cmds)[0], "cat") == 0) {
						close(fd[0]);
						int f;
						char **curline = *(cmds);

						char read_buf[1];
						int newline_flag = 0;
						if(*(curline + 1) == NULL) {
							f = current_fd;
						}
						else if(*(curline + 2) == NULL) {
							f = open(*(curline + 1), O_RDONLY);
						}

						while(read(f, read_buf, sizeof(read_buf)) > 0) {
							printf("%c", read_buf[0]);
							if(read_buf[0] == '\n') {
								newline_flag = 1;
							}
							else {
								newline_flag = 0;
							}
						}

						if(!newline_flag) {
							printf("\n");
						}

						close(f);
						exit(1);
					}
					else if(strcmp(*(cmds)[0], "mv") == 0) {
						/* Do NOTHING */
					}
					else if(strcmp(*(cmds)[0], "rm") == 0) {
						/* Do NOTHING */
					}
					else if(strcmp(*(cmds)[0], "cp") == 0) {
						/* Do NOTHING */
					}
					else if(strcmp(*(cmds)[0], "cd") == 0) {
						/* Do NOTHING */
					}
					else if(strcmp(*(cmds)[0], "pwd") == 0) {
						/* PWD DONT HAVE INPUT ONLY OUTPUT */
						char *cwd = getcwd(NULL, 1024);
						close(fd[0]);
						printf("%s\n", cwd);
						exit(1);
					}
					else if(strcmp(*(cmds)[0], "exit") == 0) {
						/* Do NOTHING */
					}
					else if(cmd_type == 1 || cmd_type == 2) {
						close(fd[0]);
						execv(path, *cmds);
					}
				}
			}
		}
		else if(redirection_count == 1) { /* A < B | C | * * * * | END OR A | B | C | D | * * * * > END */ 
			char **first_cmd = *cmds;
			char *file_name = NULL;
			int left_side_redirection = 0;
			int cmd_index = 0;
			int file_name_index = 0;
			while(*first_cmd != NULL) {
				if(strcmp(*first_cmd, "<") == 0) {
					left_side_redirection = 1;
					file_name_index = cmd_index + 1;
					break;
				}
				first_cmd++;
				cmd_index++;
			}

			if(left_side_redirection) { // <
				char **curline = *cmds;
				int first_input_fd  = open(*(curline + file_name_index), O_RDONLY);
				if(first_input_fd < 0) {
					printf("swsh: No such file\n");
				}
				else {
					int fd[2];
					int current_fd = 0;
					int group_id = 0;
					pid_t pid;
					char path[100];

					int cmd_cnt = 0;
					while(*cmds != NULL) {
						pipe(fd);

						int cmd_type = cmd_type_checking(*(cmds)[0]);
						if(cmd_type == 2) {
							sprintf(path, "%s", *(cmds)[0]);
						}
						else {
							if((strcmp(*(cmds)[0], "sort") == 0) || (strcmp(*(cmds)[0], "man") == 0) || (strcmp(*(cmds)[0], "bc") == 0)) {
								sprintf(path, "/usr/bin/%s", *(cmds)[0]);
							}
							else sprintf(path, "/bin/%s", *(cmds)[0]);
						}

						pid = fork();
						if(pid == 0) {

							if(cmd_cnt == 0) {
								group_id = getpid();
							}

							setpgid(pid, group_id);
							if(*(cmds + 1) != NULL) {
								dup2(fd[1], 1);
							}

							if(cmd_cnt == 0) {
								dup2(first_input_fd, 0);

								curline = *cmds;

								if(strcmp(*(cmds)[0], "head") == 0) {
									/* Do NOTHING */
								}
								else if(strcmp(*(cmds)[0], "tail") == 0) {
									/* Do NOTHING */
								}
								else if(strcmp(*(cmds)[0], "cat") == 0) {
									/* Do NOTHING */
								}
								else if(strcmp(*(cmds)[0], "mv") == 0) {
									/* Do NOTHING */
								}
								else if(strcmp(*(cmds)[0], "rm") == 0) {
									/* Do NOTHING */
								}
								else if(strcmp(*(cmds)[0], "cp") == 0) {
									/* Do NOTHING */
								}
								else if(strcmp(*(cmds)[0], "cd") == 0) {
									/* Do NOTHING */
								}
								else if(strcmp(*(cmds)[0], "pwd") == 0) {
									/* Do NOTHING */
								}
								else if(strcmp(*(cmds)[0], "exit") == 0) {
									/* Do NOTHING */
								}
								else if(cmd_type == 1 || cmd_type == 2) {
									if((strcmp(*(cmds)[0], "sort") == 0) || (strcmp(*(cmds)[0], "man") == 0) || (strcmp(*(cmds)[0], "bc") == 0)) {
										sprintf(path, "/usr/bin/%s", *(cmds)[0]);
									}
									close(fd[0]);
									*(curline + file_name_index - 1) = NULL;
									execv(path, *cmds);
								}
							}
							else {
								dup2(current_fd, 0);
								if(strcmp(*(cmds)[0], "head") == 0) {
									close(fd[0]);
									int read_cnt = 0;
									int f;
									char **curline = *(cmds);

									if(*(curline + 2) == NULL) { /* No OPTIONS */
										f = open(*(curline + 1), O_RDONLY);
										read_cnt = 10;
									}
									else if(*(curline + 4) == NULL) {
										f = open(*(curline + 3), O_RDONLY);
										read_cnt = atoi(*(curline + 2));
									}

									char read_buf[1];
									int last_line_check = 1;
									while(read(f, read_buf, sizeof(read_buf)) > 0) {
										if(read_buf[0] == '\n') {
											read_cnt -= 1;
											last_line_check = 0;
											printf("\n");
										}
										else {
											printf("%c", read_buf[0]);
											last_line_check = 1;
										}

										if(read_cnt == 0) break;
									}

									
									if(last_line_check) {
										printf("\n");
									}
									
									close(f);
									exit(1);
								}
								else if(strcmp(*(cmds)[0], "tail") == 0) {
									close(fd[0]);
									int read_cnt = 0;
									int f;
									char **curline = *(cmds);
									if(*(curline + 2) == NULL) { /* No OPTIONS */
										f = open(*(curline + 1), O_RDONLY);
										read_cnt = 10;
									}
									else if(*(curline + 4) == NULL) {
										f = open(*(curline + 3), O_RDONLY);
										read_cnt = atoi(*(curline + 2));
									}

									int total_line_cnt = 0;
									int last_line_add_flag = 1;
									char read_buf[1];

									while(read(f, read_buf, sizeof(read_buf)) > 0) {
										if(read_buf[0] == '\n') {
											total_line_cnt += 1;
											last_line_add_flag = 0;
										}
										else {
											printf("%c", read_buf[0]);
											last_line_add_flag = 1;
										}
									}
									
									lseek(f, 0, SEEK_SET);
									if(last_line_add_flag) total_line_cnt += 1;

									int last_line_check = 1;
									
									if(total_line_cnt <= read_cnt) { 
										while(read(f, read_buf, sizeof(read_buf)) > 0) {
											if(read_buf[0] == '\n') {
												printf("\n");
												last_line_check = 0;
											}
											else {
												printf("%c", read_buf[0]);
												last_line_check = 1;
											}
										}

										if(last_line_check) {
											printf("\n");
										}
									}
									else {
										int first_read_cnt = total_line_cnt - read_cnt;
										while(read(f, read_buf, sizeof(read_buf)) > 0) {
											if(read_buf[0] == '\n') {
												first_read_cnt -= 1;
											}
											if(first_read_cnt == 0) break;
										}

										while(read(f, read_buf, sizeof(read_buf)) > 0) {
											if(read_buf[0] == '\n') {
												printf("\n");
												last_line_check = 0;
											}
											else {
												printf("%c", read_buf[0]);
												last_line_check = 1;
											}
										}

										if(last_line_check) {
											printf("\n");
										}
									}
									close(f);
									exit(1);
								}
								else if(strcmp(*(cmds)[0], "cat") == 0) {
									close(fd[0]);
									int f;
									char **curline = *(cmds);

									char read_buf[1];
									if(*(curline + 1) == NULL) {
										f = current_fd;
									}
									else if(*(curline + 2) == NULL) {
										f = open(*(curline + 1), O_RDONLY);
									}

									int newline_flag = 0;
									while(read(f, read_buf, sizeof(read_buf)) > 0) {
										printf("%c", read_buf[0]);
										if(read_buf[0] == '\n') {
											newline_flag = 1;
										}
										else {
											newline_flag = 0;
										}
									}

									if(!newline_flag) {
										printf("\n");
									}

									close(f);
									exit(1);
								}
								else if(strcmp(*(cmds)[0], "mv") == 0) {
									/* Do NOTHING */
								}
								else if(strcmp(*(cmds)[0], "rm") == 0) {
									/* Do NOTHING */
								}
								else if(strcmp(*(cmds)[0], "cp") == 0) {
									/* Do NOTHING */
								}
								else if(strcmp(*(cmds)[0], "cd") == 0) {
									/* Do NOTHING */
								}
								else if(strcmp(*(cmds)[0], "pwd") == 0) {
									/* PWD DONT HAVE INPUT ONLY OUTPUT */
									char *cwd = getcwd(NULL, 1024);
									close(fd[0]);
									printf("%s\n", cwd);
									exit(1);
								}
								else if(strcmp(*(cmds)[0], "exit") == 0) {
									/* Do NOTHING */
								}
								else if(cmd_type == 1 || cmd_type == 2) {
									close(fd[0]);
									execv(path, *cmds);
								}
							}
						}
						else {
							if(cmd_cnt == 0) {
								group_id = pid;
							}
							setpgid(pid, group_id);
							wait(NULL);
							close(fd[1]);
							current_fd = fd[0];
							cmds += 1;
							cmd_cnt += 1;
						}
					}
				}
			}
			else { // >
				int fd[2];
				int current_fd = 0;
				pid_t pid;
				char path[100];
				char **curline = NULL;
				int last_flag = 0;

				int status = 0;
				int group_id = 0;

				while(*cmds != NULL) {
					pipe(fd);

					int cmd_type = cmd_type_checking(*(cmds)[0]);
					if(cmd_type == 2) {
						sprintf(path, "%s", *(cmds)[0]);
					}
					else {
						if((strcmp(*(cmds)[0], "sort") == 0) || (strcmp(*(cmds)[0], "man") == 0) || (strcmp(*(cmds)[0], "bc") == 0)) {
							sprintf(path, "/usr/bin/%s", *(cmds)[0]);
						}
						else sprintf(path, "/bin/%s", *(cmds)[0]);
					}

					pid = fork();
					if(pid == 0) {
						if(status == 0) {
							group_id = getpid();
						}

						setpgid(pid, group_id);

						if(*(cmds + 1) != NULL) {
							dup2(current_fd, 0);
							dup2(fd[1], 1);
							if(strcmp(*(cmds)[0], "head") == 0) {
								close(fd[0]);
								int read_cnt = 0;
								int f;
								char **curline = *(cmds);

								if(*(curline + 2) == NULL) { /* No OPTIONS */
									f = open(*(curline + 1), O_RDONLY);
									read_cnt = 10;
								}
								else if(*(curline + 4) == NULL) {
									f = open(*(curline + 3), O_RDONLY);
									read_cnt = atoi(*(curline + 2));
								}

								char read_buf[1];
								int last_line_check = 1;
								while(read(f, read_buf, sizeof(read_buf)) > 0) {
									if(read_buf[0] == '\n') {
										read_cnt -= 1;
										last_line_check = 0;
										printf("\n");
									}
									else {
										printf("%c", read_buf[0]);
										last_line_check = 1;
									}

									if(read_cnt == 0) break;
								}

								
								if(last_line_check) {
									printf("\n");
								}
								
								close(f);
								exit(1);
							}
							else if(strcmp(*(cmds)[0], "tail") == 0) {
								close(fd[0]);
								int read_cnt = 0;
								int f;
								char **curline = *(cmds);
								if(*(curline + 2) == NULL) { /* No OPTIONS */
									f = open(*(curline + 1), O_RDONLY);
									read_cnt = 10;
								}
								else if(*(curline + 4) == NULL) {
									f = open(*(curline + 3), O_RDONLY);
									read_cnt = atoi(*(curline + 2));
								}

								int total_line_cnt = 0;
								int last_line_add_flag = 1;
								char read_buf[1];

								while(read(f, read_buf, sizeof(read_buf)) > 0) {
									if(read_buf[0] == '\n') {
										total_line_cnt += 1;
										last_line_add_flag = 0;
									}
									else {
										last_line_add_flag = 1;
									}
								}
								
								lseek(f, 0, SEEK_SET);
								if(last_line_add_flag) total_line_cnt += 1;

								int last_line_check = 1;
								
								if(total_line_cnt <= read_cnt) { 
									while(read(f, read_buf, sizeof(read_buf)) > 0) {
										if(read_buf[0] == '\n') {
											printf("\n");
											last_line_check = 0;
										}
										else {
											printf("%c", read_buf[0]);
											last_line_check = 1;
										}
									}

									if(last_line_check) {
										printf("\n");
									}
								}
								else {
									int first_read_cnt = total_line_cnt - read_cnt;
									while(read(f, read_buf, sizeof(read_buf)) > 0) {
										if(read_buf[0] == '\n') {
											first_read_cnt -= 1;
										}
										if(first_read_cnt == 0) break;
									}

									while(read(f, read_buf, sizeof(read_buf)) > 0) {
										if(read_buf[0] == '\n') {
											printf("\n");
											last_line_check = 0;
										}
										else {
											printf("%c", read_buf[0]);
											last_line_check = 1;
										}
									}

									if(last_line_check) {
										printf("\n");
									}
								}
								close(f);
								exit(1);
							}
							else if(strcmp(*(cmds)[0], "cat") == 0) {
								close(fd[0]);
								int f;
								char **curline = *(cmds);

								char read_buf[1];
								if(*(curline + 1) == NULL) {
									f = current_fd;
								}
								else if(*(curline + 2) == NULL) {
									f = open(*(curline + 1), O_RDONLY);
								}

								while(read(f, read_buf, sizeof(read_buf)) > 0) {
									printf("%c", read_buf[0]);
								}

								close(f);
								exit(1);
							}
							else if(strcmp(*(cmds)[0], "mv") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "rm") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "cp") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "cd") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "pwd") == 0) {
								/* PWD DONT HAVE INPUT ONLY OUTPUT */
								char *cwd = getcwd(NULL, 1024);
								close(fd[0]);
								printf("%s\n", cwd);
								exit(1);
							}
							else if(strcmp(*(cmds)[0], "exit") == 0) {
								/* Do NOTHING */
							}
							else if(cmd_type == 1 || cmd_type == 2) {
								close(fd[0]);
								execv(path, *cmds);
							}

						}
						else {
							curline = *(cmds);
							int cmd_index = 0;
							int file_name_index = 0;
							int can_overwrite = 0; 
							while(*curline != NULL) {
								if(strcmp(*curline, ">") == 0) {
									file_name_index = cmd_index + 1;
									break;
								}
								else if(strcmp(*curline, ">>") == 0) {
									file_name_index = cmd_index + 1;
									can_overwrite = 1;
									break;
								}
								cmd_index += 1;
								curline++;
							}
							curline = *(cmds);

							int output_fd;
							
							if(can_overwrite) {
								output_fd = open(*(curline + file_name_index), O_WRONLY | O_APPEND, 0666);
							}
							else {
								output_fd = open(*(curline + file_name_index), O_WRONLY | O_CREAT | O_TRUNC, 0666);
							}
							
							dup2(output_fd, 1);
							if(strcmp(*(cmds)[0], "head") == 0) {
								close(fd[0]);
								int read_cnt = 0;
								int f;
								char **curline = *(cmds);
								*(curline + file_name_index - 1) = NULL;

								if(*(curline + 2) == NULL) { /* No OPTIONS */
									f = open(*(curline + 1), O_RDONLY);
									read_cnt = 10;
								}
								else if(*(curline + 4) == NULL) {
									f = open(*(curline + 3), O_RDONLY);
									read_cnt = atoi(*(curline + 2));
								}

								char read_buf[1];
								int last_line_check = 1;
								while(read(f, read_buf, sizeof(read_buf)) > 0) {
									if(read_buf[0] == '\n') {
										read_cnt -= 1;
										last_line_check = 0;
										printf("\n");
									}
									else {
										printf("%c", read_buf[0]);
										last_line_check = 1;
									}

									if(read_cnt == 0) break;
								}

								
								if(last_line_check) {
									printf("\n");
								}
								
								close(f);
								exit(1);
							}
							else if(strcmp(*(cmds)[0], "tail") == 0) {
								close(fd[0]);
								int read_cnt = 0;
								int f;
								char **curline = *(cmds);
								*(curline + file_name_index - 1) = NULL;
								if(*(curline + 2) == NULL) { /* No OPTIONS */
									f = open(*(curline + 1), O_RDONLY);
									read_cnt = 10;
								}
								else if(*(curline + 4) == NULL) {
									f = open(*(curline + 3), O_RDONLY);
									read_cnt = atoi(*(curline + 2));
								}

								int total_line_cnt = 0;
								int last_line_add_flag = 1;
								char read_buf[1];

								while(read(f, read_buf, sizeof(read_buf)) > 0) {
									if(read_buf[0] == '\n') {
										total_line_cnt += 1;
										last_line_add_flag = 0;
									}
									else {
										last_line_add_flag = 1;
									}
								}
								
								lseek(f, 0, SEEK_SET);
								if(last_line_add_flag) total_line_cnt += 1;

								int last_line_check = 1;
								
								if(total_line_cnt <= read_cnt) { 
									while(read(f, read_buf, sizeof(read_buf)) > 0) {
										if(read_buf[0] == '\n') {
											printf("\n");
											last_line_check = 0;
										}
										else {
											printf("%c", read_buf[0]);
											last_line_check = 1;
										}
									}

									if(last_line_check) {
										printf("\n");
									}
								}
								else {
									int first_read_cnt = total_line_cnt - read_cnt;
									while(read(f, read_buf, sizeof(read_buf)) > 0) {
										if(read_buf[0] == '\n') {
											first_read_cnt -= 1;
										}
										if(first_read_cnt == 0) break;
									}

									while(read(f, read_buf, sizeof(read_buf)) > 0) {
										if(read_buf[0] == '\n') {
											printf("\n");
											last_line_check = 0;
										}
										else {
											printf("%c", read_buf[0]);
											last_line_check = 1;
										}
									}

									if(last_line_check) {
										printf("\n");
									}
								}
								close(f);
								exit(1);
							}
							else if(strcmp(*(cmds)[0], "cat") == 0) {
								close(fd[0]);
								int f;
								char **curline = *(cmds);
								*(curline + file_name_index - 1) = NULL;

								char read_buf[1];
								if(*(curline + 1) == NULL) {
									f = current_fd;
								}
								else if(*(curline + 2) == NULL) {
									f = open(*(curline + 1), O_RDONLY);
								}

								while(read(f, read_buf, sizeof(read_buf)) > 0) {
									printf("%c", read_buf[0]);
								}

								close(f);
								exit(1);
							}
							else if(strcmp(*(cmds)[0], "mv") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "rm") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "cp") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "cd") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "pwd") == 0) {
								/* PWD DONT HAVE INPUT ONLY OUTPUT */
								char *cwd = getcwd(NULL, 1024);
								close(fd[0]);
								printf("%s\n", cwd);
								exit(1);
							}
							else if(strcmp(*(cmds)[0], "exit") == 0) {
								/* Do NOTHING */
							}
							else if(cmd_type == 1 || cmd_type == 2) {
								close(fd[0]);
								char **curline = *(cmds);
								*(curline + file_name_index - 1) = NULL;
								execv(path, *cmds);
							}
						}
					}
					else {
						if(status == 0) {
							group_id = pid;
						}
						setpgid(pid, group_id);


						wait(NULL);
						close(fd[1]);
						current_fd = fd[0];
						cmds += 1;
						status += 1;
					}
				}
			}
		}
		else if(redirection_count == 2) {
			char **curline = *cmds;
			int first_file_index = 0;
			int cmd_index = 0;
			while(*curline != NULL) {
				if(strcmp(*curline, "<") == 0) {
					first_file_index = cmd_index + 1;
					break;
				}
				curline++;
				cmd_index++;
			}
			curline = *cmds;
			int first_input_fd = open(*(curline + first_file_index), O_RDONLY);
			if(first_input_fd < 0) {
				printf("swsh: No such file\n");
			}
			else {
				int fd[2];
				int current_fd = 0;
				pid_t pid;
				char path[100];

				int cmd_cnt = 0;
				int group_id = 0;

				while(*cmds != NULL) {
					pipe(fd);

					int cmd_type = cmd_type_checking(*(cmds)[0]);
					if(cmd_type == 2) {
						sprintf(path, "%s", *(cmds)[0]);
					}
					else {
						if((strcmp(*(cmds)[0], "sort") == 0) || (strcmp(*(cmds)[0], "man") == 0) || (strcmp(*(cmds)[0], "bc") == 0)) {
							sprintf(path, "/usr/bin/%s", *(cmds)[0]);
						}
						else sprintf(path, "/bin/%s", *(cmds)[0]);
					}

					pid = fork();
					if(pid == 0) {
						if(cmd_cnt == 0) {
							group_id = getpid();
						}
						setpgid(pid, group_id);


						if(cmd_cnt == 0) {
							dup2(fd[1], 1);
							dup2(first_input_fd, 0);
							curline = *cmds;

							if(strcmp(*(cmds)[0], "head") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "tail") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "cat") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "mv") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "rm") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "cp") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "cd") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "pwd") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "exit") == 0) {
								/* Do NOTHING */
							}
							else if(cmd_type == 1 || cmd_type == 2) {
								close(fd[0]);
								*(curline + first_file_index - 1) = NULL;
								// *(cmds)[file_name_index - 1] = NULL;
								execv(path, *cmds);
							}
						}
						else if(*(cmds + 1) != NULL){
							dup2(fd[1], 1);
							dup2(current_fd, 0);
							curline = *cmds;

							if(strcmp(*(cmds)[0], "head") == 0) {
								close(fd[0]);
								int read_cnt = 0;
								int f;
								char **curline = *(cmds);

								if(*(curline + 2) == NULL) { /* No OPTIONS */
									f = open(*(curline + 1), O_RDONLY);
									read_cnt = 10;
								}
								else if(*(curline + 4) == NULL) {
									f = open(*(curline + 3), O_RDONLY);
									read_cnt = atoi(*(curline + 2));
								}

								char read_buf[1];
								int last_line_check = 1;
								while(read(f, read_buf, sizeof(read_buf)) > 0) {
									if(read_buf[0] == '\n') {
										read_cnt -= 1;
										last_line_check = 0;
										printf("\n");
									}
									else {
										printf("%c", read_buf[0]);
										last_line_check = 1;
									}

									if(read_cnt == 0) break;
								}

								
								if(last_line_check) {
									printf("\n");
								}
								
								close(f);
								exit(1);
							}
							else if(strcmp(*(cmds)[0], "tail") == 0) {
								close(fd[0]);
								int read_cnt = 0;
								int f;
								char **curline = *(cmds);
								if(*(curline + 2) == NULL) { /* No OPTIONS */
									f = open(*(curline + 1), O_RDONLY);
									read_cnt = 10;
								}
								else if(*(curline + 4) == NULL) {
									f = open(*(curline + 3), O_RDONLY);
									read_cnt = atoi(*(curline + 2));
								}

								int total_line_cnt = 0;
								int last_line_add_flag = 1;
								char read_buf[1];

								while(read(f, read_buf, sizeof(read_buf)) > 0) {
									if(read_buf[0] == '\n') {
										total_line_cnt += 1;
										last_line_add_flag = 0;
									}
									else {
										last_line_add_flag = 1;
									}
								}
								
								lseek(f, 0, SEEK_SET);
								if(last_line_add_flag) total_line_cnt += 1;

								int last_line_check = 1;
								
								if(total_line_cnt <= read_cnt) { 
									while(read(f, read_buf, sizeof(read_buf)) > 0) {
										if(read_buf[0] == '\n') {
											printf("\n");
											last_line_check = 0;
										}
										else {
											printf("%c", read_buf[0]);
											last_line_check = 1;
										}
									}

									if(last_line_check) {
										printf("\n");
									}
								}
								else {
									int first_read_cnt = total_line_cnt - read_cnt;
									while(read(f, read_buf, sizeof(read_buf)) > 0) {
										if(read_buf[0] == '\n') {
											first_read_cnt -= 1;
										}
										if(first_read_cnt == 0) break;
									}

									while(read(f, read_buf, sizeof(read_buf)) > 0) {
										if(read_buf[0] == '\n') {
											printf("\n");
											last_line_check = 0;
										}
										else {
											printf("%c", read_buf[0]);
											last_line_check = 1;
										}
									}

									if(last_line_check) {
										printf("\n");
									}
								}
								close(f);
								exit(1);
							}
							else if(strcmp(*(cmds)[0], "cat") == 0) {
								close(fd[0]);
								int f;
								char **curline = *(cmds);

								char read_buf[1];
								if(*(curline + 1) == NULL) {
									f = current_fd;
								}
								else if(*(curline + 2) == NULL) {
									f = open(*(curline + 1), O_RDONLY);
								}

								int newline_flag = 0;
								while(read(f, read_buf, sizeof(read_buf)) > 0) {
									printf("%c", read_buf[0]);
									if(read_buf[0] == '\n') {
										newline_flag = 1;
									}
									else {
										newline_flag = 0;
									}
								}

								if(!newline_flag) {
									printf("\n");
								}

								close(f);
								exit(1);
							}
							else if(strcmp(*(cmds)[0], "mv") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "rm") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "cp") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "cd") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "pwd") == 0) {
								/* PWD DONT HAVE INPUT ONLY OUTPUT */
								char *cwd = getcwd(NULL, 1024);
								close(fd[0]);
								printf("%s\n", cwd);
								exit(1);
							}
							else if(strcmp(*(cmds)[0], "exit") == 0) {
								/* Do NOTHING */
							}
							else if(cmd_type == 1 || cmd_type == 2) {
								close(fd[0]);
								execv(path, *cmds);
							}
						}
						else if(*(cmds + 1) == NULL) {
							curline = *(cmds);
							int cmd_index = 0;
							int file_name_index = 0;
							int can_overwrite = 0; 
							while(*curline != NULL) {
								if(strcmp(*curline, ">") == 0) {
									file_name_index = cmd_index + 1;
									break;
								}
								else if(strcmp(*curline, ">>") == 0) {
									file_name_index = cmd_index + 1;
									can_overwrite = 1;
									break;
								}
								cmd_index += 1;
								curline++;
							}
							curline = *(cmds);

							int output_fd;
							if(can_overwrite) {
								output_fd = open(*(curline + file_name_index), O_WRONLY | O_APPEND, 0666);
								output_fd = open(*(curline + file_name_index), O_WRONLY | O_CREAT | O_TRUNC, 0666);
							}
							else {
								output_fd = open(*(curline + file_name_index), O_WRONLY | O_CREAT | O_TRUNC, 0666);
							}
							*(curline + file_name_index - 1) = NULL;
							dup2(output_fd, 1);

							if(strcmp(*(cmds)[0], "head") == 0) {
								close(fd[0]);
								int read_cnt = 0;
								int f;
								char **curline = *(cmds);
								*(curline + file_name_index - 1) = NULL;
								
								if(*(curline + 2) == NULL) { /* No OPTIONS */
									f = open(*(curline + 1), O_RDONLY);
									read_cnt = 10;
								}
								else if(*(curline + 4) == NULL) {
									f = open(*(curline + 3), O_RDONLY);
									read_cnt = atoi(*(curline + 2));
								}

								char read_buf[1];
								int last_line_check = 1;
								while(read(f, read_buf, sizeof(read_buf)) > 0) {
									if(read_buf[0] == '\n') {
										read_cnt -= 1;
										last_line_check = 0;
										printf("\n");
									}
									else {
										printf("%c", read_buf[0]);
										last_line_check = 1;
									}

									if(read_cnt == 0) break;
								}

								
								if(last_line_check) {
									printf("\n");
								}
								
								close(f);
								exit(1);
							}
							else if(strcmp(*(cmds)[0], "tail") == 0) {
								close(fd[0]);
								int read_cnt = 0;
								int f;
								char **curline = *(cmds);
								*(curline + file_name_index - 1) = NULL;

								if(*(curline + 2) == NULL) { /* No OPTIONS */
									f = open(*(curline + 1), O_RDONLY);
									read_cnt = 10;
								}
								else if(*(curline + 4) == NULL) {
									f = open(*(curline + 3), O_RDONLY);
									read_cnt = atoi(*(curline + 2));
								}

								int total_line_cnt = 0;
								int last_line_add_flag = 1;
								char read_buf[1];

								while(read(f, read_buf, sizeof(read_buf)) > 0) {
									if(read_buf[0] == '\n') {
										total_line_cnt += 1;
										last_line_add_flag = 0;
									}
									else {
										last_line_add_flag = 1;
									}
								}
								
								lseek(f, 0, SEEK_SET);
								if(last_line_add_flag) total_line_cnt += 1;

								int last_line_check = 1;
								
								if(total_line_cnt <= read_cnt) { 
									while(read(f, read_buf, sizeof(read_buf)) > 0) {
										if(read_buf[0] == '\n') {
											printf("\n");
											last_line_check = 0;
										}
										else {
											printf("%c", read_buf[0]);
											last_line_check = 1;
										}
									}

									if(last_line_check) {
										printf("\n");
									}
								}
								else {
									int first_read_cnt = total_line_cnt - read_cnt;
									while(read(f, read_buf, sizeof(read_buf)) > 0) {
										if(read_buf[0] == '\n') {
											first_read_cnt -= 1;
										}
										if(first_read_cnt == 0) break;
									}

									while(read(f, read_buf, sizeof(read_buf)) > 0) {
										if(read_buf[0] == '\n') {
											printf("\n");
											last_line_check = 0;
										}
										else {
											printf("%c", read_buf[0]);
											last_line_check = 1;
										}
									}

									if(last_line_check) {
										printf("\n");
									}
								}
								close(f);
								exit(1);
							}
							else if(strcmp(*(cmds)[0], "cat") == 0) {
								close(fd[0]);
								int f;
								char **curline = *(cmds);
								*(curline + file_name_index - 1) = NULL;

								char read_buf[1];
								if(*(curline + 1) == NULL) {
									f = current_fd;
								}
								else if(*(curline + 2) == NULL) {
									f = open(*(curline + 1), O_RDONLY);
								}

								int newline_flag = 0;
								while(read(f, read_buf, sizeof(read_buf)) > 0) {
									printf("%c", read_buf[0]);
									if(read_buf[0] == '\n') {
										newline_flag = 1;
									}
									else {
										newline_flag = 0;
									}
 								}

								 if(!newline_flag) {
									 printf("\n");
								 }

								close(f);
								exit(1);
							}
							else if(strcmp(*(cmds)[0], "mv") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "rm") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "cp") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "cd") == 0) {
								/* Do NOTHING */
							}
							else if(strcmp(*(cmds)[0], "pwd") == 0) {
								/* PWD DONT HAVE INPUT ONLY OUTPUT */
								char *cwd = getcwd(NULL, 1024);
								close(fd[0]);
								printf("%s\n", cwd);
								exit(1);
							}
							else if(strcmp(*(cmds)[0], "exit") == 0) {
								/* Do NOTHING */
							}
							else if(cmd_type == 1 || cmd_type == 2) {
								close(fd[0]);
								char **curline = *(cmds);
								*(curline + file_name_index - 1) = NULL;
								execv(path, *cmds);
							}
						}
					}
					else {
						if(cmd_cnt == 0) {
							group_id = pid;
						}
						setpgid(pid, group_id);

						wait(NULL);
						close(fd[1]);
						current_fd = fd[0];
						cmds++;
						cmd_cnt += 1;
					}
				}
			}
		}
		else if(redirection_count > 2) { /* ERROR CASE */
			printf("swsh: Command not found\n");
		}
	}
	else {
		printf("swsh: Command not found\n");
	}
}

int main(){
	signal(SIGINT, sigint_handler);
	signal(SIGTSTP, sigtstp_handler);

	while(1){
		int i  = 0;
        int pipe_cnt = 0;
		int redirection_cnt = 0;
		char *cmd = NULL;
		size_t size = 0;
		char *arg[LMAX];
		getline(&cmd, &size, stdin);
		cmd[strlen(cmd)-1] = '\0';
		char *cmd_copy = malloc(sizeof(char) * strlen(cmd));
        strcpy(cmd_copy, cmd);

		if(strlen(cmd) == 0) continue;

		char* ptr = strtok(cmd, " ");
		while(ptr != NULL){
            if(strcmp(ptr, "|") == 0) pipe_cnt += 1;
			if(strcmp(ptr, "<") == 0) redirection_cnt += 1;
			if(strcmp(ptr, ">") == 0) redirection_cnt += 1;
			if(strcmp(ptr, ">>") == 0) redirection_cnt += 1;
			arg[i++] = ptr;
			ptr = strtok(NULL, " ");
		}
		arg[i] = NULL;
		if(strcmp("quit", arg[0]) == 0)	{
			exit(0);
			break;
		}

		if(pipe_cnt > 0) {
			/* PIPELINE */
			execute_command_with_pipeline(arg, redirection_cnt, pipe_cnt, i, cmd_copy);
		}
		else {
			/* Only COMMANDS */
			execute_command_without_pipeline(arg, redirection_cnt, i);
		}
	}
	return return_val;
}