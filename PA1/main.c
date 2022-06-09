#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

void find(char *fn, int fl) {
	char target_word_msg[] = "Input target word : ";
	write(1, target_word_msg, sizeof(target_word_msg));

	int current_line = 1;
	int current_col  = 0;

	char input_character_buf[1];
	char input_word[25];
	int input_word_len = 0;
	while(1) {
		read(0, input_character_buf, 1);
		if (input_character_buf[0] == '\n') {
			break;
		}
		input_word[input_word_len++] = input_character_buf[0];
	}
	
	/* Read file */
	int fd;
	if ((fd = open(fn, O_RDONLY)) < 0) {
		perror("open");
		return;
	}
	
	/* For write output */
	int output_fd;
	if ((output_fd = open("./result.txt", O_CREAT | O_RDWR | O_TRUNC, 0666)) < 0) {
		perror("open new file");
		return;
	}
	
	char read_buf[1];
	while(read(fd, read_buf, sizeof(read_buf)) > 0) {
		current_col += 1;
		if (read_buf[0] == '\n') {
			current_line += 1;
			current_col   = 0;
		}
		else if (read_buf[0] == input_word[0]) {
			int find = 1;
			for(int i=1; i<input_word_len; i++) {
				read(fd, read_buf, 1);
				if (read_buf[0] != input_word[i]) {
					find = 0;
				}
			}

			if (find) {
				char output_msg_token_1[] = "line : ";
				write(output_fd, output_msg_token_1, sizeof(output_msg_token_1) - 1);
				
				char cur_line_buf[6];
				int cur_temp = current_line;
				int idx = 0;

				while(cur_temp > 0) {
					cur_line_buf[idx++] = (cur_temp % 10) + '0';
					cur_temp = cur_temp / 10;
				}
				char reverse_cur_line[6];
				for(int i=0; i<idx; i++) {
					reverse_cur_line[i] = cur_line_buf[idx-i-1];
				}

				write(output_fd, reverse_cur_line, idx);

				char output_msg_token_2[] = " col : ";
				write(output_fd, output_msg_token_2, sizeof(output_msg_token_2) - 1);
								
				char cur_col_buf[1024];
				int col_temp = current_col;
				idx = 0;

				while(col_temp > 0) {
					cur_col_buf[idx++] = (col_temp % 10) + '0';
					col_temp = col_temp / 10;
				}
				char reverse_cur_col[1024];
				for(int i=0; i<idx; i++) {
					reverse_cur_col[i] = cur_col_buf[idx-i-1];
				}
				reverse_cur_col[idx] = '\n';
				write(output_fd, reverse_cur_col, idx + 1);

				current_col += input_word_len - 1;
			}

			else {
				lseek(fd, -(input_word_len - 1), SEEK_CUR);
			}			
		}
	}

	close(fd);
	close(output_fd);
}

void find_wo_case_sensitive(char *fn, int fl) {
	/* This function may be familiar with above find function */
	/* Only add checking case sensitive part */
	char target_word_msg[] = "Input target word : ";
	write(1, target_word_msg, sizeof(target_word_msg));

	int current_line = 1;
	int current_col  = 0;

	char input_character_buf[1];
	char input_word[25];
	int input_word_len = 0;
	while(1) {
		read(0, input_character_buf, 1);
		if (input_character_buf[0] == '\n') {
			break;
		}
		input_word[input_word_len++] = input_character_buf[0];
	}
	
	/* Read file */
	int fd;
	if ((fd = open(fn, O_RDONLY)) < 0) {
		perror("open");
		return;
	}
	
	/* For write output */
	int output_fd;
	if ((output_fd = open("./result.txt", O_CREAT | O_RDWR | O_TRUNC, 0666)) < 0) {
		perror("open new file");
		return;
	}
	
	char read_buf[1];
	while(read(fd, read_buf, sizeof(read_buf)) > 0) {
		current_col += 1;
		if (read_buf[0] == '\n') {
			current_line += 1;
			current_col   = 0;
		}
		else if (read_buf[0] - input_word[0] == 0 || read_buf[0] - input_word[0] == -32 || read_buf[0] - input_word[0] == 32) {
			int find = 1;
			for(int i=1; i<input_word_len; i++) {
				read(fd, read_buf, 1);
				if ((read_buf[0] - input_word[i] == 32) || (read_buf[0] - input_word[i] == -32) || (read_buf[0] - input_word[i] == 0)) {
					continue;
				}
				else {
					find = 0;
				}
			}

			if (find) {
				char output_msg_token_1[] = "line : ";
				write(output_fd, output_msg_token_1, sizeof(output_msg_token_1) - 1);
				
				char cur_line_buf[6];
				int cur_temp = current_line;
				int idx = 0;

				while(cur_temp > 0) {
					cur_line_buf[idx++] = (cur_temp % 10) + '0';
					cur_temp = cur_temp / 10;
				}
				char reverse_cur_line[6];
				for(int i=0; i<idx; i++) {
					reverse_cur_line[i] = cur_line_buf[idx-i-1];
				}

				write(output_fd, reverse_cur_line, idx);

				char output_msg_token_2[] = " col : ";
				write(output_fd, output_msg_token_2, sizeof(output_msg_token_2) - 1);
								
				char cur_col_buf[1024];
				int col_temp = current_col;
				idx = 0;

				while(col_temp > 0) {
					cur_col_buf[idx++] = (col_temp % 10) + '0';
					col_temp = col_temp / 10;
				}
				char reverse_cur_col[1024];
				for(int i=0; i<idx; i++) {
					reverse_cur_col[i] = cur_col_buf[idx-i-1];
				}
				reverse_cur_col[idx] = '\n';
				write(output_fd, reverse_cur_col, idx + 1);
	
				current_col += input_word_len - 1;
			}
			else {
				lseek(fd, -(input_word_len - 1), SEEK_CUR);
			}			
		}
	}

	close(fd);
	close(output_fd);
}

void replace(char *fn, int fl) {
	char target_word_msg[]  = "Input target word : ";
	char replace_word_msg[] = "Input replace word : ";

	write(1, target_word_msg, sizeof(target_word_msg));

	int current_line = 1;
	int current_col  = 0;

	char input_character_buf[1];
	char input_word[25];
	int input_word_len = 0;
	while(1) {
		read(0, input_character_buf, 1);
		if (input_character_buf[0] == '\n') {
			break;
		}
		input_word[input_word_len++] = input_character_buf[0];
	}
	
	write(1, replace_word_msg, sizeof(replace_word_msg));	
	char replace_character_buf[1];
	char replace_word[25];
	int replace_word_len = 0;
	while(1) {
		read(0, replace_character_buf, 1);
		if (replace_character_buf[0] == '\n') {
			break;
		}
		replace_word[replace_word_len++] = replace_character_buf[0];
	}
	
	/* Read file */
	int fd;
	if ((fd = open(fn, O_RDONLY)) < 0) {
		perror("open");
		return;
	}
	
	/* For write output */
	int output_fd;
	if ((output_fd = open("./result.txt", O_CREAT | O_RDWR | O_TRUNC, 0666)) < 0) {
		perror("open new file");
		return;
	}
	
	char read_buf[1];
	while(1) {
		int flag = read(fd, read_buf, 1);
		if (flag <= 0) {
			break;
		}

		if (read_buf[0] != input_word[0])
			write(output_fd, read_buf, 1);
		if (read_buf[0] == input_word[0]) {
			int find = 1;
			char temp_buf[1];
			temp_buf[0] = read_buf[0];

			for (int i=1; i<input_word_len; i++) {
				read(fd, read_buf, 1);
				if (read_buf[0] != input_word[i]) {
					find = 0;
				}
			}
			
			if (find) {
				/* Write replace word in same position */
				write(output_fd, replace_word, replace_word_len);
			}
			else {
				/* just write one character */
				lseek(fd, -(input_word_len - 1), SEEK_CUR);
				write(output_fd, temp_buf, 1);
			}
		}
	}

	close(fd);
	close(output_fd);	
}

int main(int argc, char **argv) {
	char file_name[25];

	/* First, get file name from user */
	char input_msg[] = "File name : ";
	write(1, input_msg, 12);
	
	char file_name_character_buf[1];
	int file_name_len = 0;
	while(1) {
		read(0, file_name_character_buf, 1);
		if (file_name_character_buf[0] == '\n') {
			break;
		}
		file_name[file_name_len++] = file_name_character_buf[0];
	}

	/* Second, user can choice one option in 4 options */
	char options_msg[] = "(1) Find (2) Find w/o case sensitive (3) Replace (4) Exit : ";
	write(1, options_msg, sizeof(options_msg));
	
	char option_buf[2];
	read(0, option_buf, 2);
	int user_option;

	if (option_buf[0] == '1') {
		/* Call find function */
		find(file_name, file_name_len);
	}
	else if (option_buf[0] == '2') {
		/* Call find w/o case_sensitive function */
		find_wo_case_sensitive(file_name, file_name_len);
	}
	else if (option_buf[0] == '3') {
		/* Call replace function */
		replace(file_name, file_name_len);
	}
	else {
		return 0;
	}

	return 0;
}

