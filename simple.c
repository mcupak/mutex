#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/*
 * Mutual exclusion experiments.
 * Specify the algorithm you want to use by setting ALG macro to:
 *      0: default implementation with 2 processes
 *      1: first attempt
 *      2: second attempt
 *      3: third attempt
 *      4: peterson's algorithm
 *      5: lamport's algorithm
 *      6: semaphors
 *
 * Set NUM_THREADS to specify the number of threads. Replace for with while
 * where commented to get rid of fixed number of iterations.
 *
 * Completed tasks:
 * - experiments with time bounds in sleep (how probable is violation)
 * - implement different algoritms:
 *    - "first attempt" (alternation)
 *    - "third attempt" (deadlock)
 *    - peterson's algorithm
 *    - lamport's bakery algorithm
 *    - using semaphors (pthread_mutex_lock)
 * - redo the implementation with just one parametrized process specification
 */

#define ALG             6  // algorithm
#define K               5  // number of iterations of the loop
#define SLEEP           1000000  // max. length of random sleep, 1000000 = 1 s
#define NUM_THREADS     3 // number of threads

volatile int inCS = 0;
volatile int flag1 = 0;
volatile int flag2 = 0;
volatile int turn = 0;
volatile int flags[NUM_THREADS];
volatile int number[NUM_THREADS];
volatile int choosing[NUM_THREADS];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// used for artificial waiting to increase probability of different interleavings

void random_sleep() {
    usleep(rand() % SLEEP);
}

void *process1(void* arg) {
    int i;
    for (i = 0; i < K; i++) {
        // noncritical section
        printf("1: NCS (%d)\n", i);
        random_sleep();

        // wait section
        printf("1: Wait (%d)\n", i);
        while (flag2 == 1) {
        }
        random_sleep();
        flag1 = 1;

        // critical section
        inCS++;
        printf("1: CS (%d)\n", i);
        if (inCS > 1) printf("    Mutual exclusion violated!\n");
        random_sleep();
        inCS--;

        // exit section
        flag1 = 0;
    }
    pthread_exit(NULL);
}

void *process2(void* arg) {
    int i;
    for (i = 0; i < K; i++) {
        // noncritical section
        printf("\t\t2: NCS (%d)\n", i);
        random_sleep();

        // wait section
        printf("\t\t2: Wait (%d)\n", i);
        while (flag1 == 1) {
        }
        random_sleep();
        flag2 = 1;

        // critical section
        inCS++;
        printf("\t\t2: CS (%d)\n", i);
        if (inCS > 1) printf("    Mutual exclusion violated!\n");
        random_sleep();
        inCS--;

        // exit section
        flag2 = 0;
    }
    pthread_exit(NULL);
}

/*
    check if any flag is set, except mine
 */
int any_flag(int thread) {
    int i;
    for (i = 0; i < NUM_THREADS; i++) {
        if ((i != thread) && (flags[i] != 0)) return 1;
    }
    return 0;
}

/*
    second attempt from the slides
 */
void *second_process(void *threadid) {
    int tid = (int) threadid;
    int i;
    /*
        while (1) {
     */
    for (i = 0; i < K; i++) {
        // noncritical section
        printf("%d: NCS (%d)\n", tid, i);
        random_sleep();

        // wait section
        printf("%d: Wait (%d)\n", tid, i);
        while (any_flag(tid) == 1) {
        };
        random_sleep();
        flags[tid] = 1;

        // critical section
        inCS++;
        printf("%d: CS (%d)\n", tid, i);
        if (inCS > 1) printf("    Mutual exclusion violated!\n");
        random_sleep();
        inCS--;
        flags[tid] = 0;
    }
    pthread_exit(NULL);
}

/*
    find max number
 */
int max_number() {
    int i, result = 0;
    for (i = 0; i < NUM_THREADS; i++) {
        if (number[i] > result) result = i;
    }
    return result;
}

/*
    third attempt from the slides
 */
void *third_process(void *threadid) {
    int tid = (int) threadid;
    int i;
    /*
        while (1) {
     */
    for (i = 0; i < K; i++) {
        // noncritical section
        printf("%d: NCS (%d)\n", tid, i);
        random_sleep();

        // wait section
        printf("%d: Wait (%d)\n", tid, i);
        flags[tid] = 1;
        // uncomment the following line to increase deadlock probability
        random_sleep();
        while (any_flag(tid) == 1) {
        };
        random_sleep();

        // critical section
        inCS++;
        printf("%d: CS (%d)\n", tid, i);
        if (inCS > 1) printf("    Mutual exclusion violated!\n");
        random_sleep();
        inCS--;
        flags[tid] = 0;
    }
    pthread_exit(NULL);
}

/*
    first attempt from the slides
 */
void *first_process(void *threadid) {
    long tid = (long) threadid;
    int i;
    /*
        while (1) {
     */
    for (i = 0; i < K; i++) {
        // noncritical section
        printf("%d: NCS (%d)\n", tid, i);
        random_sleep();

        // wait section
        printf("%d: Wait (%d)\n", tid, i);
        while (turn != tid) {
        };
        random_sleep();

        // critical section
        inCS++;
        printf("%d: CS (%d)\n", tid, i);
        if (inCS > 1) printf("    Mutual exclusion violated!\n");
        random_sleep();
        inCS--;
        turn = (turn + 1) % NUM_THREADS;
    }
    pthread_exit(NULL);
}

/*
    peterson's algorithm
 */
void *peterson_process(void *threadid) {
    int tid = (int) threadid;
    int i;
    /*
        while (1) {
     */
    for (i = 0; i < K; i++) {
        // noncritical section
        printf("%d: NCS (%d)\n", tid, i);
        random_sleep();

        // wait section
        printf("%d: Wait (%d)\n", tid, i);
        flags[tid] = 1;
        // round robin turn
        turn = (turn + 1) % NUM_THREADS;
        while ((any_flag(tid) == 1) && (turn != tid)) {
        };
        random_sleep();

        // critical section
        inCS++;
        printf("%d: CS (%d)\n", tid, i);
        // if (inCS > 1) printf("    Mutual exclusion violated!\n");
        random_sleep();
        inCS--;
        flags[tid] = 0;
    }
    pthread_exit(NULL);
}

/*
    lamport's algorithm
 */
void *lamport_process(void *threadid) {
    int tid = (int) threadid;
    int i;
    /*
        while (1) {
     */
    for (i = 0; i < K; i++) {
        // noncritical section
        printf("%d: NCS (%d)\n", tid, i);
        random_sleep();

        // wait section
        printf("%d: Wait (%d)\n", tid, i);

        choosing[tid] = 1;
        number[tid] = 1 + max_number();
        choosing[tid] = 0;

        int j;
        for (j = 0; j < NUM_THREADS; j++) {
            while (choosing[j] > 0) {
            }
            while ((number[j] != 0) && ((number[j] < number[tid]) || ((number[j] == number[tid]) && (j < tid)))) {
            }
        }
        random_sleep();

        // critical section
        inCS++;
        printf("%d: CS (%d)\n", tid, i);
        // if (inCS > 1) printf("    Mutual exclusion violated!\n");
        random_sleep();
        inCS--;
        number[tid] = 0;
    }
    pthread_exit(NULL);
}

/*
    semaphores
 */
void *semaphore_process(void *threadid) {
    int tid = (int) threadid;
    int i;
    /*
        while (1) {
     */
    for (i = 0; i < K; i++) {
        // noncritical section
        printf("%d: NCS (%d)\n", tid, i);
        random_sleep();

        // wait section
        printf("%d: Wait (%d)\n", tid, i);
        random_sleep();

        // critical section
        pthread_mutex_lock(&mutex);
        inCS++;
        printf("%d: CS (%d)\n", tid, i);
        // if (inCS > 1) printf("    Mutual exclusion violated!\n");
        random_sleep();
        inCS--;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

/*
    Main.
 */
int main(int argc, char *argv[]) {
    // init
    srand(time(NULL));
    int i;
    for (i = 0; i < NUM_THREADS; i++) {
        flags[i] = 0;
        number[i] = 0;
        choosing[i] = 0;
    }

    if (ALG == 0) {
        pthread_t p1, p2;

        pthread_create(&p1, NULL, process1, NULL);
        pthread_create(&p2, NULL, process2, NULL);
    } else {
        pthread_t threads[NUM_THREADS];
        int rc = 0;
        long t;
        for (t = 0; t < NUM_THREADS; t++) {
            printf("In main: creating thread %ld\n", t);
            switch (ALG) {
                case 1:
                    rc = pthread_create(&threads[t], NULL, first_process, (void *) t);
                    break;
                case 2:
                    rc = pthread_create(&threads[t], NULL, second_process, (void *) t);
                    break;
                case 3:
                    rc = pthread_create(&threads[t], NULL, third_process, (void *) t);
                    break;
                case 4:
                    rc = pthread_create(&threads[t], NULL, peterson_process, (void *) t);
                    break;
                case 5:
                    rc = pthread_create(&threads[t], NULL, lamport_process, (void *) t);
                    break;
                case 6:
                    rc = pthread_create(&threads[t], NULL, semaphore_process, (void *) t);
                    break;
            }
            if (rc) {
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                return (0);
            }
        }
    }

    pthread_exit(NULL);
}
