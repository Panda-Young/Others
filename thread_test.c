#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void *thread_function(void *arg)
{
    sleep(1);
    time_t current_time = time(NULL);
    if (current_time == (time_t)-1) {
        perror("time() failed");
    } else {
        printf("time is %ld\n", current_time);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t thread_ID;
    pthread_attr_t attr;
    int rc;
    void *thread_status;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (int i = 0; i < 5; i++) {
        if (i > 0) {
            rc = pthread_join(thread_ID, &thread_status);
            if (rc) {
                printf("Error: Return code from pthread_join() is %d\n", rc);
                exit(-1);
            }
        }

        rc = pthread_create(&thread_ID, &attr, thread_function, NULL);
        if (rc) {
            printf("Error: Return code from pthread_create() is %d\n", rc);
            exit(-1);
        } else {
            printf("pthread_create() is successful 0x%lx\n", thread_ID);
        }
    }

    rc = pthread_join(thread_ID, &thread_status);
    if (rc) {
        printf("Error: Return code from pthread_join() is %d\n", rc);
        exit(-1);
    }

    pthread_attr_destroy(&attr);
    return 0;
}
