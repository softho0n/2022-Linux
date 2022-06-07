#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{
	// define return struct for thread_exit
	int value;
}thread_data;

int **matrix;
int *vector;
int M, N;

void *thread(void *arg) {
	long tmp = (long)arg;

	// Claculate matrix[t] * vector and return result using thread_data struct	
	thread_data *my_data = (thread_data*)malloc(sizeof(thread_data));

	int return_value = 0;
	for(int i=0; i<N; i++) {
		return_value += matrix[tmp][i] * vector[i];
	}
	my_data->value = return_value;
	pthread_exit((void*)my_data);
	// Code 
}


int main(int argc, char* argv[]) {
	M = (*argv[1]) - '0';
	N = (*argv[2]) - '0';
	pthread_t *tid = (pthread_t*)malloc(sizeof(pthread_t) * M); 

	printf(" *** Matrix ***\n");
	matrix = (int**)malloc(sizeof(int*)*M);
	for (int i = 0; i < M; i++){
		matrix[i] = (int*)malloc(sizeof(int)*N);
		for (int j = 0; j < N; j++){
			matrix[i][j] = rand()%10;
			printf("[ %d ] ", matrix[i][j]);
		}
		printf("\n");
	}
	
	printf(" *** Vector ***\n");
	vector = (int*) malloc(sizeof(int)*N);
	for (int i = 0; i < N; i++){
		vector[i] = rand()%10;
		printf("[ %d ] \n", vector[i]);
	}
	
	// Create new thread to calculate Rows
	long t;

	for (t=0; t<M; t++) {
		if (pthread_create(&tid[t], NULL, thread, (void *)t)) {
			exit(1);
		}
	}
	// Code 
	
	printf(" *** Result ***\n");
	
	for (t=0; t<M; t++) {
		thread_data *res;
		pthread_join(tid[t], (void**)&res);
		printf("[ %d ] \n", res->value);
	}
	// print the result, get thread_data struct from threads using thread_join() function.
	
	// Code 
}


