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
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);  // ����ȡ��״̬
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); // ����ȡ������Ϊ�ӳ�ȡ��

    while (1) {
        printf("Thread is running...\n");
        pthread_testcancel(); // ����һ��ȡ����
        sleep(1);             // ģ��һЩ����
    }

    pthread_exit(NULL); // �̺߳��������˳������������������Զ���ᵽ������
}

int main()
{
    pthread_t my_thread;
    int ret;

    // �����߳�
    ret = pthread_create(&my_thread, NULL, my_thread_func, NULL);
    if (ret != 0) {
        printf("Error: pthread_create() failed\n");
        exit(EXIT_FAILURE);
    }

    // �����̵߳ȴ�һ��ʱ�䣬�Ա���Կ������߳�������
    sleep(5);

    // ȡ���߳�
    printf("Canceling thread...\n");
    ret = pthread_cancel(my_thread);
    if (ret != 0) {
        printf("Error: pthread_cancel() failed\n");
    }

    // �ȴ��߳̽���
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
