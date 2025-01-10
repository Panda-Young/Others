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

// 文件名
const char *filename = "sample.wav";
// 共享内存对象名
const char *shm_name = "/shared_memory.wav";

// 全局变量，用于控制循环
volatile sig_atomic_t keep_running = 1;

// 信号处理函数
void handle_signal(int signal) {
    if (signal == SIGINT || signal == SIGTSTP) {
        keep_running = 0;
    }
}

int main() {
    // 注册信号处理函数
    signal(SIGINT, handle_signal);
    signal(SIGTSTP, handle_signal);

    // 打开文件
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        LOGE("Failed to open file: %s. %s", filename, strerror(errno));
        return 1;
    }

    // 获取文件大小
    struct stat st;
    if (fstat(fd, &st) == -1) {
        LOGE("Failed to get file size: %s. %s", filename, strerror(errno));
        close(fd);
        return 1;
    }
    size_t file_size = st.st_size;

    // 创建共享内存
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        LOGE("Failed to create shared memory: %s. %s", shm_name, strerror(errno));
        close(fd);
        return 1;
    }

    // 设置共享内存大小
    if (ftruncate(shm_fd, file_size) == -1) {
        LOGE("Failed to set shared memory size: %s. %s", shm_name, strerror(errno));
        close(fd);
        return 1;
    }

    // 映射共享内存
    void *shm_ptr = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        LOGE("Failed to map shared memory: %s. %s", shm_name, strerror(errno));
        close(fd);
        return 1;
    }

    // 读取文件内容到共享内存
    if (read(fd, shm_ptr, file_size) != file_size) {
        LOGE("Failed to read file %s into shared memory: %s. %s", filename, shm_name, strerror(errno));
        munmap(shm_ptr, file_size);
        close(fd);
        return 1;
    }

    LOGI("File %s loaded into shared memory %s. File size: %lu bytes", filename, shm_name, file_size);
    // 关闭文件描述符
    close(fd);
    // 取消共享内存映射
    munmap(shm_ptr, file_size);
    // 关闭并删除共享内存对象
    close(shm_fd);

    // 进程主循环
    while (keep_running) {
        // 保持共享内存数据可访问
        sleep(60); // 每分钟检查一次
    }

    shm_unlink(shm_name);

    LOGI("Exiting gracefully...");

    return 0;
}
