#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int main(int argc, char **argv) {
	int fd;
	if ((fd = open("./peterpan.txt", O_RDONLY)) < 0) {
		perror("open");
		exit(1);
	}

	int fd2;
	if ((fd2 = open("./result.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666)) < 0)
	{
		perror("open new file");
		exit(1);
	}

	int cur_line = 1;
	char input[1];
	char ipt[10];
	int len = 0;
	while(1) {
		read(0, input, 1);
		if (input[0] == '\n') {
			break;
		}
		ipt[len++] = input[0];
	}
	

	char buf[1];
	while(read(fd, buf, sizeof(buf)) > 0) {
		if (buf[0] == '\n') {
			cur_line += 1;
		}
		else if (buf[0] == ipt[0]) {
			int find = 1;
			for(int i=1; i<len; i++) {
				read(fd, buf, 1);
				if (buf[0] != ipt[i]) {
					find = 0;
				}
			}

			if (find) {
				char output_buf[1024];
				int temp = cur_line;
				int idx = 0;
				while(temp > 0) {
					output_buf[idx++] = (temp % 10) + '0';
					temp = temp / 10;
				}
				char real_output[1024];
				for(int i=0; i<idx; i++) {
					real_output[i] = output_buf[idx-i-1];
				}
				real_output[idx] = '\n';
				write(fd2, real_output, idx+1);
			}
			else {
				lseek(fd, -(len-1), SEEK_CUR);
			}
		}
	}
}

