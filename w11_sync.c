#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

int point_in_circle = 0;
int num_points_per_thread;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void *runner() {
    long thread_point_in_circle = 0;

    unsigned int rand_state = rand();
    long i;
    for (i = 0; i < num_points_per_thread; i++) {
        double x = rand() / (double)RAND_MAX * 1.0;
        double y = rand() / (double)RAND_MAX * 1.0;

        if (x*x + y*y <= 1)
            thread_point_in_circle++;
    }
    
    pthread_mutex_lock(&m);
    point_in_circle += thread_point_in_circle;
    pthread_mutex_unlock(&m);
}

int main(int argc, const char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "wrong arguments");
        exit(1);
    }

	srand((unsigned)time(NULL));

    int num_thread = atoi(argv[1]);
    num_points_per_thread = atol(argv[2]);

    pthread_t *threads = malloc(num_thread * sizeof(pthread_t));

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    
	//create
    for (int i = 0; i < num_thread; i++) {
        pthread_create(&threads[i], &attr, runner, (void *) NULL);
    }

	//join
    for (int i = 0; i < num_thread; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&m);
    free(threads);

    int point_total = num_thread * num_points_per_thread;
	printf("%lf\n", 4 * (double)point_in_circle / point_total);

    return 0;
}

