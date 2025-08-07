/*
 * @Description: thread test
 * @Date: 2024-05-09 12:33:24
 * @Version: 0.1.0
 * @Author: Panda-Young
 * Copyright (c) 2024 by @Panda-Young, All Rights Reserved.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *my_thread_func(void *arg)
{
    // Enable thread cancellation
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    // Set cancellation type to deferred
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    while (1) {
        printf("Thread is running...\n");
        // Check for cancellation request
        pthread_testcancel();
        // Simulate some work
        sleep(1);
    }

    // Exit the thread
    pthread_exit(NULL);
}

int main()
{
    pthread_t my_thread;
    int ret;

    // Create a new thread
    ret = pthread_create(&my_thread, NULL, my_thread_func, NULL);
    if (ret != 0) {
        printf("Error: pthread_create() failed\n");
        exit(EXIT_FAILURE);
    }

    // Let the thread run for a while
    sleep(5);

    // Cancel the thread
    printf("Canceling thread...\n");
    ret = pthread_cancel(my_thread);
    if (ret != 0) {
        printf("Error: pthread_cancel() failed\n");
    }

    // Wait for the thread to finish
    void *res;
    pthread_join(my_thread, &res);

    if (res == PTHREAD_CANCELED) {
        printf("Thread was canceled\n");
    } else {
        printf("Thread exited with status %p\n", res);
    }

    printf("Main thread exiting\n");
    return 0;
}
