/***************************************************************************
 * Description: Read data from shared memory
 * Version: 0.1.0
 * Author: 1641140221@qq.com
 * Date: 2024-08-02 16:38:21
 * Copyright (c) 2024 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#include <iostream>
#include <stdio.h>
#include "log.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

// Shared memory name
const char* shm_name = "/shared_memory.wav";

int main() {
#ifdef _WIN32
    HANDLE hMapFile;
    LPCTSTR pBuf;

    // Open the shared memory
    hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, shm_name);
    if (hMapFile == NULL) {
        LOGE("Failed to open shared memory: %s. Error: %d", shm_name, GetLastError());
        return 1;
    }

    // Map the shared memory
    pBuf = (LPTSTR) MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
    if (pBuf == NULL) {
        LOGE("Failed to map shared memory: %s. Error: %d", shm_name, GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }

    // Read data from shared memory, here we just print the first 128 bytes as an example
    char buffer[129];
    memcpy(buffer, pBuf, 128);
    buffer[128] = '\0';
    LOGI("First 128 bytes of shared memory:\n%s", buffer);

    // Unmap the shared memory
    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);

#else
    // Open the shared memory
    int shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        LOGE("Failed to open shared memory: %s. %s", shm_name, strerror(errno));
        return 1;
    }

    // Get the size of the shared memory
    struct stat st;
    if (fstat(shm_fd, &st) == -1) {
        LOGE("Failed to get shared memory %s size. %s", shm_name, strerror(errno));
        close(shm_fd);
        return 1;
    }
    size_t file_size = st.st_size;

    // Map the shared memory
    void* shm_ptr = mmap(NULL, file_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        LOGE("Failed to map shared memory %s. %s", shm_name, strerror(errno));
        close(shm_fd);
        return 1;
    }

    // Read data from shared memory, here we just print the first 128 bytes as an example
    char buffer[5];
    memcpy(buffer, shm_ptr, 4);
    buffer[4] = '\0';
    LOGI("First 128 bytes of shared memory:\n%s", buffer);

    // Unmap the shared memory
    munmap(shm_ptr, file_size);
    close(shm_fd);
#endif

    return 0;
}
