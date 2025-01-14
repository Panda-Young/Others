/***************************************************************************
 * Description: Read a file into shared memory
 * Version: 0.1.0
 * Author: Panda-Young
 * Date: 2024-08-06 16:57:53
 * Copyright (c) 2024 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#include "log.h"
#include <iostream>
#include <signal.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

// File name to be read
const char *filename = "example.wav";
// Shared memory name
const char *shm_name = "/shared_memory.wav";

// Global variable to control the running loop
volatile sig_atomic_t keep_running = 1;

// Signal handler to catch termination signals
void handle_signal(int signal) {
    if (signal == SIGINT
#if !defined(_WIN32) && !defined(_WIN64)
        || signal == SIGTSTP
#endif
    ) {
        keep_running = 0;
    }
}

int main() {
    // Register signal handlers
    signal(SIGINT, handle_signal);
#if !defined(_WIN32) && !defined(_WIN64)
    signal(SIGTSTP, handle_signal);
#endif

#ifdef _WIN32
    HANDLE hFile = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        LOGE("Failed to open file: %s. Error: %d", filename, GetLastError());
        return 1;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        LOGE("Failed to get file size: %s. Error: %d", filename, GetLastError());
        CloseHandle(hFile);
        return 1;
    }

    // Try to open existing file mapping and close it if it exists
    HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, shm_name);
    if (hMapFile != NULL) {
        CloseHandle(hMapFile);
    }

    hMapFile = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, fileSize, shm_name);
    if (hMapFile == NULL) {
        LOGE("Failed to create file mapping: %s. Error: %d", shm_name, GetLastError());
        CloseHandle(hFile);
        return 1;
    }

    LPVOID pBuf = MapViewOfFile(hMapFile, FILE_MAP_WRITE, 0, 0, fileSize);
    if (pBuf == NULL) {
        LOGE("Failed to map view of file: %s. Error: %d", shm_name, GetLastError());
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return 1;
    }

    DWORD bytesRead;
    if (!ReadFile(hFile, pBuf, fileSize, &bytesRead, NULL)) {
        LOGE("Failed to read file: %s. Error: %d", filename, GetLastError());
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return 1;
    }

    LOGI("Read %d bytes from file into shared memory", bytesRead);

    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    CloseHandle(hFile);

#else
    // Open the file
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        LOGE("Failed to open file: %s. %s", filename, strerror(errno));
        return 1;
    }

    // Get the file size
    struct stat st;
    if (fstat(fd, &st) == -1) {
        LOGE("Failed to get file size: %s. %s", filename, strerror(errno));
        close(fd);
        return 1;
    }
    size_t file_size = st.st_size;

    // Create the shared memory object
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        LOGE("Failed to create shared memory: %s. %s", shm_name, strerror(errno));
        close(fd);
        return 1;
    }

    // Set the size of the shared memory object
    if (ftruncate(shm_fd, file_size) == -1) {
        LOGE("Failed to set size of shared memory: %s. %s", shm_name, strerror(errno));
        close(fd);
        close(shm_fd);
        return 1;
    }

    // Map the shared memory object
    void* shm_ptr = mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        LOGE("Failed to map shared memory: %s. %s", shm_name, strerror(errno));
        close(fd);
        close(shm_fd);
        return 1;
    }

    // Read the file into the shared memory
    ssize_t bytes_read = read(fd, shm_ptr, file_size);
    if (bytes_read == -1) {
        LOGE("Failed to read file: %s. %s", filename, strerror(errno));
        munmap(shm_ptr, file_size);
        close(fd);
        close(shm_fd);
        return 1;
    }

    LOGI("Read %ld bytes from file into shared memory", bytes_read);

    // Unmap the shared memory and close file descriptors
    munmap(shm_ptr, file_size);
    close(fd);
    close(shm_fd);
#endif

    return 0;
}
