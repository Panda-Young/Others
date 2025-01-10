/*
 * @Description: thread test
 * @Date: 2024-05-09 12:33:24
 * @Version: 0.1.0
 * @Author: pandapan@aactechnologies.com
 * Copyright (c) 2024 by @AAC Technologies, All Rights Reserved.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *my_thread_func(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);  // 启用取消状态
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); // 设置取消类型为延迟取消

    while (1) {
        printf("Thread is running...\n");
        pthread_testcancel(); // 这是一个取消点
        sleep(1);             // 模拟一些工作
    }

    pthread_exit(NULL); // 线程函数正常退出，但在这个例子中永远不会到达这里
}

int main()
{
    pthread_t my_thread;
    int ret;

    // 创建线程
    ret = pthread_create(&my_thread, NULL, my_thread_func, NULL);
    if (ret != 0) {
        printf("Error: pthread_create() failed\n");
        exit(EXIT_FAILURE);
    }

    // 让主线程等待一段时间，以便可以看到子线程在运行
    sleep(5);

    // 取消线程
    printf("Canceling thread...\n");
    ret = pthread_cancel(my_thread);
    if (ret != 0) {
        printf("Error: pthread_cancel() failed\n");
    }

    // 等待线程结束
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
