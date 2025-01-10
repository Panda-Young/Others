/* **************************************************************
 * @Description: read file to shared memory
 * @Date: 2024-08-06 16:57:53
 * @Version: 0.1.0
 * @Author: pandapan@aactechnologies.com
 * @Copyright (c) 2024 by @AAC Technologies, All Rights Reserved.
 **************************************************************/

#include "log.h"
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// �ļ���
const char *filename = "sample.wav";
// �����ڴ������
const char *shm_name = "/shared_memory.wav";

// ȫ�ֱ��������ڿ���ѭ��
volatile sig_atomic_t keep_running = 1;

// �źŴ�����
void handle_signal(int signal) {
    if (signal == SIGINT || signal == SIGTSTP) {
        keep_running = 0;
    }
}

int main() {
    // ע���źŴ�����
    signal(SIGINT, handle_signal);
    signal(SIGTSTP, handle_signal);

    // ���ļ�
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        LOGE("Failed to open file: %s. %s", filename, strerror(errno));
        return 1;
    }

    // ��ȡ�ļ���С
    struct stat st;
    if (fstat(fd, &st) == -1) {
        LOGE("Failed to get file size: %s. %s", filename, strerror(errno));
        close(fd);
        return 1;
    }
    size_t file_size = st.st_size;

    // ���������ڴ�
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        LOGE("Failed to create shared memory: %s. %s", shm_name, strerror(errno));
        close(fd);
        return 1;
    }

    // ���ù����ڴ��С
    if (ftruncate(shm_fd, file_size) == -1) {
        LOGE("Failed to set shared memory size: %s. %s", shm_name, strerror(errno));
        close(fd);
        return 1;
    }

    // ӳ�乲���ڴ�
    void *shm_ptr = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        LOGE("Failed to map shared memory: %s. %s", shm_name, strerror(errno));
        close(fd);
        return 1;
    }

    // ��ȡ�ļ����ݵ������ڴ�
    if (read(fd, shm_ptr, file_size) != file_size) {
        LOGE("Failed to read file %s into shared memory: %s. %s", filename, shm_name, strerror(errno));
        munmap(shm_ptr, file_size);
        close(fd);
        return 1;
    }

    LOGI("File %s loaded into shared memory %s. File size: %lu bytes", filename, shm_name, file_size);
    // �ر��ļ�������
    close(fd);
    // ȡ�������ڴ�ӳ��
    munmap(shm_ptr, file_size);
    // �رղ�ɾ�������ڴ����
    close(shm_fd);

    // ������ѭ��
    while (keep_running) {
        // ���ֹ����ڴ����ݿɷ���
        sleep(60); // ÿ���Ӽ��һ��
    }

    shm_unlink(shm_name);

    LOGI("Exiting gracefully...");

    return 0;
}
