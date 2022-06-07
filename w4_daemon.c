#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
	unsigned int pid;
	time_t t;
	struct tm *tm;	
	int fd;
	char *argv[3];
	char buf[512];
	int fd0, fd1, fd2;

	fd = open("./crontab", O_RDWR);
	pid = fork();
	
	if(pid == -1) return -1;
	if(pid != 0)
		exit(0);
	if(setsid() < 0)
		exit(0);

	umask(0);

	close(0);
	close(1);
	close(2);

	fd0 = open("/dev/null", O_RDWR);
	fd1 = open("/dev/null", O_RDWR);
	fd2 = open("/dev/null", O_RDWR);

	t = time(NULL);
	tm = localtime(&t);

	while (1)
	{
		buf[0] = '\0';
		
		char temp[1];
		int i = 0;
		while(read(fd, temp, sizeof(temp)) > 0) {
			if (temp[0] == '\n') {
				buf[i] = '\0';
				i = 0;
				
				char *token;
				char *pos = buf;
				char *cmd;
				int arg_idx = 0;
				
				while((token = strtok_r(pos, " ", &pos))) {
					argv[arg_idx++] = token;
				}
			        
			   	t = time(NULL);
				tm = localtime(&t);
					
				if(strcmp(argv[0], "*") != 0 && strcmp(argv[1], "*") == 0) {
					int target_min = atoi(argv[0]);
					if(tm->tm_min == target_min) {
						if(fork() == 0) {
							execl("/bin/sh", "/bin/sh", argv[2], (char *) NULL);
						}
					}
				}
				else if(strcmp(argv[0], "*") == 0 && strcmp(argv[1], "*") == 0) {
					if(fork() == 0) {
						execl("/bin/sh", "/bin/sh", argv[2], (char *) NULL);
					}
				}
				else if(strcmp(argv[0], "*") == 0 && strcmp(argv[1], "*") != 0) {
					int target_hour = atoi(argv[1]);
					if(tm->tm_hour == target_hour) {
						if(fork() == 0) {
							execl("/bin/sh", "/bin/sh", argv[2], (char *) NULL);
						}
					}
				}
				else if(strcmp(argv[0], "*") != 0 && strcmp(argv[1], "*") != 0) {
					int target_min = atoi(argv[0]);
					int target_hour = atoi(argv[1]);
					if(tm->tm_min == target_min && tm->tm_hour == target_hour) {
						if(fork() == 0) {
							execl("/bin/sh", "/bin/sh", argv[2], (char *) NULL);
						}
					}
				}
			}
			else {
				buf[i++] = temp[0];
			}
		}
		
		t = time(NULL);
		tm = localtime(&t);
		lseek(fd, 0, SEEK_SET);
		sleep(60 - tm->tm_sec % 60);
	}

	return 0;
}
