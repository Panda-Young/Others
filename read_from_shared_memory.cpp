/* **************************************************************
 * @Description: 
 * @Date: 2024-08-02 16:38:21
 * @Version: 0.1.0
 * @Author: 1641140221@qq.com
 * @Copyright (c) 2024 by @AAC Technologies, All Rights Reserved.
 **************************************************************/
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include "log.h"

// �����ڴ������
const char* shm_name = "/shared_memory.wav";

int main() {
    // �򿪹����ڴ����
    int shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        LOGE("Failed to open shared memory: %s. %s", shm_name, strerror(errno));
        return 1;
    }

    // ��ȡ�����ڴ��С
    struct stat st;
    if (fstat(shm_fd, &st) == -1) {
        LOGE("Failed to get shared memory %s size. %s", shm_name, strerror(errno));
        close(shm_fd);
        return 1;
    }
    size_t file_size = st.st_size;

    // ӳ�乲���ڴ�
    void* shm_ptr = mmap(NULL, file_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        LOGE("Failed to map shared memory %s. %s", shm_name, strerror(errno));
        close(shm_fd);
        return 1;
    }

    // ��ȡ�����ڴ��е����ݣ���������ֻ��ӡǰ128�ֽ���Ϊʾ����
    char buffer[129];
    memcpy(buffer, shm_ptr, 128);
    buffer[128] = '\0';
    LOGI("First 128 bytes of shared memory:\n%s", buffer);    
    // ȡ�������ڴ�ӳ��
    munmap(shm_ptr, file_size);
    
    // �رչ����ڴ����
    close(shm_fd);

    return 0;
}
