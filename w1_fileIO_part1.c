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
	
	char buf[1];
	int nbytes;
	
	int fd2;
	if ((fd2 = open("./peterpan_upper.txt", O_CREAT | O_WRONLY, 0666)) < 0)
	{		
		perror("open new file");
		exit(1);
	}
	
	while(read(fd, buf, sizeof(buf)) > 0) {
		if (buf[0] >= 97 && buf[0] <= 122) {
			buf[0] = buf[0] - 32;
		}
		write(fd2, buf, sizeof(buf));
	}
}

