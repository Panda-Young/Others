/***************************************************************************
 * Description: log
 * version: 0.1.0
 * Author: Panda-Young
 * Date: 2024-05-01 17:06:54
 * Copyright (c) 2024 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#if defined(_MSC_VER)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include "log.h"
#include <stdlib.h>

#if USE_SLOG2INFO
slog2_buffer_t slog_buffer;

const static slog2_buffer_set_config_t slog_config = {
    .num_buffers = 1,
    .verbosity_level = SLOG2_DEBUG2,
    .buffer_set_name = BUFFER_SET_NAME,
    .buffer_config = {
        .buffer_name = BUFFER_NAME,
        .num_pages = SLOG_PAGE_NUM,
    },
};

__attribute__((constructor)) void slog_buffer_init(void)
{
    if (-1 == slog2_register(&slog_config, &slog_buffer, 0)) {
        // can't use LOG here, use slog directly
        slogf(_SLOGC_DAYIN, _SLOG_ERROR, "Couldn't register slog2 buffer from deva!\n");
        slog_buffer = NULL;
    } else {
        slog2_set_default_buffer(slog_buffer);
    }
}
#endif

#if USE_LINUX_SYSLOG
void open_log_operation()
{
    openlog(NULL, LOG_CONS, LOG_SYSLOG);
}
void close_log_operation()
{
    closelog();
}
#endif

#if USE_FILELOG
#define MAX_LOG_FILE_SIZE 1048576 // 1MB
#define LOG_FILE_NAME "young.log"
FILE *log_file = NULL;

#if defined(__linux__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__QNX__) || defined(__ANDROID__)
__attribute__((constructor)) void open_log_file()
{
    struct stat st;
    if (stat(LOG_FILE_NAME, &st) == 0 && st.st_size > MAX_LOG_FILE_SIZE) {
        char new_file_name[1024] = {0};
        snprintf(new_file_name, sizeof(new_file_name), "%s.%04d-%02d-%02d_%02d-%02d-%02d.log", LOG_FILE_NAME,
                 localtime(&st.st_mtime)->tm_year + 1900, localtime(&st.st_mtime)->tm_mon + 1, localtime(&st.st_mtime)->tm_mday,
                 localtime(&st.st_mtime)->tm_hour, localtime(&st.st_mtime)->tm_min, localtime(&st.st_mtime)->tm_sec);
        rename(LOG_FILE_NAME, new_file_name);
    }
    log_file = fopen(LOG_FILE_NAME, "a+");
}

__attribute__((destructor)) void close_log_file()
{
    if (log_file) {
        fflush(log_file);
        fclose(log_file);
    }
}
#endif
#endif

const char *__PROGNAME = NULL;
const char *__PROGPATH = NULL;

#if defined(_WIN32) || defined(_WIN64)
#if defined(__MINGW32__) || defined(__MINGW64__)
__attribute__((constructor)) void get_exe_path()
{
    char path[1024] = {0};
    GetModuleFileName(NULL, path, sizeof(path));
    __PROGPATH = strdup(path);
    __PROGNAME = strrchr(__PROGPATH, '\\') ? strrchr(__PROGPATH, '\\') + 1 : __PROGPATH;
}
#elif defined(_MSC_VER) // only support DEBUG mode
#pragma section(".CRT$XCU", read)
#define INITIALIZER(f)                                               \
    static void __cdecl f(void);                                     \
    __declspec(allocate(".CRT$XCU")) void(__cdecl * f##_)(void) = f; \
    static void __cdecl f(void)

#if USE_FILELOG
INITIALIZER(open_log_file)
{
    struct _stat st = {0};
    if (_stat(LOG_FILE_NAME, &st) == 0 && st.st_size > MAX_LOG_FILE_SIZE) {
        char new_file_name[1024] = {0};
        snprintf(new_file_name, sizeof(new_file_name),
                 "%s.%04d-%02d-%02d_%02d-%02d-%02d.log", LOG_FILE_NAME,
                 localtime(&st.st_mtime)->tm_year + 1900,
                 localtime(&st.st_mtime)->tm_mon + 1,
                 localtime(&st.st_mtime)->tm_mday, localtime(&st.st_mtime)->tm_hour,
                 localtime(&st.st_mtime)->tm_min, localtime(&st.st_mtime)->tm_sec);
        if (rename(LOG_FILE_NAME, new_file_name) != 0) {
        }
    }
    log_file = fopen(LOG_FILE_NAME, "a+");
}
#endif
INITIALIZER(get_exe_path)
{
    char path[1024] = {0};
    DWORD result = GetModuleFileName(NULL, path, sizeof(path));
    if (result == 0 || result >= sizeof(path)) {
        // Handle error
        __PROGPATH = NULL;
        __PROGNAME = NULL;
    } else {
        size_t path_len = sizeof(path) / 2;
        for (size_t pp = 0; pp < path_len / 2; pp++) {
            path[pp] = path[pp * 2];
        }
        __PROGPATH = _strdup(path);
        __PROGNAME = strrchr(__PROGPATH, '\\') ? strrchr(__PROGPATH, '\\') + 1 : __PROGPATH;
    }
}
#pragma section(".CRT$XTU", read)
#define DESTRUCTOR(f)                                                \
    static void __cdecl f(void);                                     \
    __declspec(allocate(".CRT$XTU")) void(__cdecl * f##_)(void) = f; \
    static void __cdecl f(void)

#if USE_FILELOG
DESTRUCTOR(close_log_file)
{
    if (log_file) {
        fflush(log_file);
        fclose(log_file);
    }
}
#endif
DESTRUCTOR(free_exe_path)
{
    if (__PROGPATH) {
        free((void *)__PROGPATH);
    }
}
#endif
#elif defined(__linux__) || defined(__QNX__) || defined(__ANDROID__)
#include <sys/types.h>
#include <unistd.h>
__attribute__((constructor)) void get_exe_path()
{
    char path[1024] = {0};
    pid_t pid = getpid();
    snprintf(path, sizeof(path), "/proc/%d/exe", (int)pid);
    char *resolved_path = realpath(path, NULL);
    if (resolved_path == NULL) {
        exit(EXIT_FAILURE);
    }
    __PROGPATH = resolved_path;
    __PROGNAME = strrchr(__PROGPATH, '/') ? strrchr(__PROGPATH, '/') + 1 : __PROGPATH;
}
#endif

#if defined(__linux__) || defined(__MINGW32__) || defined(__MINGW64__)
__attribute__((destructor)) void free_exe_path()
{
    if (__PROGPATH) {
        free((void *)__PROGPATH);
    }
}
#endif
