#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

pid_t sender, receiver;
int total_number_of_signals = 0;
int total_received_signals = 0;
int total_acked_signals = 0;
int total_sending_signals = 0;

void sigalrm_handler (int sig){
	printf("sender: total remaining signal(s): %d\n", total_number_of_signals-total_acked_signals);
	for(int i=0; i<total_number_of_signals-total_acked_signals; i++) {
		kill(receiver, SIGUSR1);
	}
	alarm(1);
}

void sigusr1_handler (int sig){
	
	printf("receiver: received #%d signal and sending ack\n", ++total_received_signals);
	kill(sender, SIGUSR2);
}

void sigusr2_handler (int sig){
	total_acked_signals += 1;
	if(total_acked_signals == total_number_of_signals) {
		printf("sender: all signals have been sent\n");
		kill(receiver, SIGINT);
		exit(0);	
	}		
	// kill(receiver, SIGUSR1);	
}

void sigint_handler (int sig){
	printf("receiver: total received signals(s): %d\n", total_number_of_signals);
	exit(0);
}


/*
 *	DON't MODIFY BELOW MAIN FUNCTION!!!
 *
 */

int main(int argc, char* argv[]){
	signal(SIGUSR1, sigusr1_handler);
	signal(SIGINT, sigint_handler);
	signal(SIGALRM, sigalrm_handler);
	signal(SIGUSR2, sigusr2_handler);

	if(argc != 2){
		printf("please input total number of signals\n");
		return -1;
	}
	total_number_of_signals = atoi(argv[1]);
	printf("total number of signal(s): %d\n", total_number_of_signals);
	sender = getpid();

	if((receiver = fork()) == 0){
		while(1){}
	}else{
		for(int i = 0; i < total_number_of_signals - total_acked_signals; i++){
			kill(receiver, SIGUSR1);
		}
		alarm(1);
	}		
	while(1);
}

