/* **************************************************************
 * @Description: get current executable path
 * @Date: 2024-02-18 14:13:48
 * @Version: 0.1.0
 * @Author: Panda-Young
 * @Copyright (c) 2024 by @Panda-Young, All Rights Reserved.
 **************************************************************/
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __MINGW32__
#include <windows.h>
#endif

char *get_current_exe_path(void)
{
#ifdef __MINGW32__
    // char *resolved_path = (char *)malloc(PATH_MAX);
    // if (_fullpath(resolved_path, NULL, PATH_MAX) == NULL) {
    //     perror("_fullpath");
    //     exit(EXIT_FAILURE);
    // }
    char *resolved_path = (char *)malloc(MAX_PATH);
    if (GetModuleFileName(NULL, resolved_path, MAX_PATH) == 0) {
        printf("GetModuleFileName failed with error %d\n", GetLastError());
        free(resolved_path);
        return NULL;
    }
#else
    char path[PATH_MAX];
    pid_t pid = getpid();
    snprintf(path, sizeof(path), "/proc/%d/exe", (int)pid);
    char *resolved_path = realpath(path, NULL);
    if (resolved_path == NULL) {
        perror("realpath");
        exit(EXIT_FAILURE);
    }
#endif

    return resolved_path;
}

int main(int argc, char *argv[])
{
    char *exe_path = get_current_exe_path();
    printf("Executable path: %s\n", exe_path);
    printf("Executable path: %s\n", argv[0]);
    free(exe_path);
    return 0;
}
