/*
 * @Description: algo use
 * @Date: 2024-05-16 17:28:52
 * @Version: 0.1.0
 * @Author: pandapan@aactechnologies.com
 * Copyright (c) 2024 by @AAC Technologies, All Rights Reserved.
 */

#include "algo_example.h"

#define SAMPLE_COUNT 8

typedef int (*GetVersionFunc)(char *version);
typedef void *(*InitFunc)();
typedef void (*DeinitFunc)(void *algo_handle);
typedef int (*SetParamFunc)(void *algo_handle, algo_param_t cmd, void *param, uint32_t param_size);
typedef int (*ProcessFunc)(void *algo_handle, void *input, void *output, int block_size);

#if defined(__linux__)
#include <dlfcn.h>

#define LIB_NAME "libalgo_example.so"

int main()
{
    void *shared_lib_handle = dlopen(LIB_NAME, RTLD_LAZY);
    if (!shared_lib_handle) {
        printf("%s\n", dlerror());
        return 1;
    }

    GetVersionFunc get_version = (GetVersionFunc)dlsym(shared_lib_handle, "get_algo_version");
    if (!get_version) {
        printf("Failed to get function.\n");
        return 1;
    }

    char version[128] = {0};
    if (get_version(version) == 0) {
        printf("Version: %s\n", version);
    } else {
        printf("Failed to get version.\n");
    }

    InitFunc init = (InitFunc)dlsym(shared_lib_handle, "algo_init");
    if (!init) {
        printf("Failed to get function.\n");
        return 1;
    }

    void *algo_handle = init();

    SetParamFunc set_param = (SetParamFunc)dlsym(shared_lib_handle, "algo_set_param");
    if (!set_param) {
        printf("Failed to get function.\n");
        return 1;
    }

    int param2 = 2;
    if (set_param(algo_handle, SET_PARAM2, (void *)&param2, sizeof(int)) != 0) {
        printf("Failed to set algo param cmd %d size %lu.\n", SET_PARAM2, sizeof(int));
    } else {
        printf("Algorithm set param cmd %d size %lu success.\n", SET_PARAM2, sizeof(int));
    }

    char param3[] = "param3";
    if (set_param(algo_handle, SET_PARAM3, param3, sizeof(param3)) != 0) {
        printf("Failed to set algo param cmd %d size %lu.\n", SET_PARAM3, sizeof(param3));
    } else {
        printf("Algorithm set param cmd %d size %lu success.\n", SET_PARAM3, sizeof(param3));
    }

    ProcessFunc process = (ProcessFunc)dlsym(shared_lib_handle, "algo_process");
    if (!process) {
        printf("Failed to get function.\n");
        return 1;
    }

    int input[SAMPLE_COUNT] = {0, 1, 2, 3, 4, 5, 6, 7};
    int output[SAMPLE_COUNT] = {0};
    if (process(algo_handle, input, output, SAMPLE_COUNT * sizeof(int)) != 0) {
        printf("Failed to process algo.\n");
        return 1;
    }
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        printf("%d ", output[i]);
    }
    printf("\n");

    DeinitFunc deinit = (DeinitFunc)dlsym(shared_lib_handle, "algo_deinit");
    if (!deinit) {
        printf("Failed to get function.\n");
        return 1;
    }

    deinit(algo_handle);

    dlclose(shared_lib_handle);

    return 0;
}

/*Add library path:
    export LD_LIBRARY_PATH=.
*/

#elif defined(_WIN32)
#include <windows.h>

#define LIB_NAME "libalgo_example.dll"

int main()
{
    HINSTANCE hinstLib = LoadLibrary(LIB_NAME);
    if (hinstLib == NULL) {
        printf("Failed to load library %s\n", LIB_NAME);
        return 1;
    }

    GetVersionFunc get_version = (GetVersionFunc)GetProcAddress(hinstLib, "get_algo_version");
    if (get_version == NULL) {
        printf("Failed to get function.\n");
        return 1;
    }

    char version[128] = {0};
    if (get_version(version) != 0) {
        printf("Failed to get version.\n");
        return 1;
    }
    printf("Version: %s\n", version);

    InitFunc init = (InitFunc)GetProcAddress(hinstLib, "algo_init");
    if (!init) {
        printf("Failed to get function.\n");
        return 1;
    }
    void *algo_handle = init();

    SetParamFunc set_param = (SetParamFunc)GetProcAddress(hinstLib, "algo_set_param");
    if (!set_param) {
        printf("Failed to get function.\n");
        return 1;
    }
    int param2 = 2;
    if (set_param(algo_handle, SET_PARAM2, (void *)&param2, sizeof(int)) != 0) {
        printf("Failed to set algo param cmd %d size %lu.\n", SET_PARAM2, sizeof(int));
    } else {
        printf("Algorithm set param cmd %d size %lu success.\n", SET_PARAM2, sizeof(int));
    }
    char param3[] = "param3";
    if (set_param(algo_handle, SET_PARAM3, param3, sizeof(param3)) != 0) {
        printf("Failed to set algo param cmd %d size %lu.\n", SET_PARAM3, sizeof(param3));
    } else {
        printf("Algorithm set param cmd %d size %lu success.\n", SET_PARAM3, sizeof(param3));
    }

    ProcessFunc process = (ProcessFunc)GetProcAddress(hinstLib, "algo_process");
    if (!process) {
        printf("Failed to get function.\n");
        return 1;
    }
    int input[SAMPLE_COUNT] = {0, 1, 2, 3, 4, 5, 6, 7};
    int output[SAMPLE_COUNT] = {0};
    if (process(algo_handle, input, output, SAMPLE_COUNT * sizeof(int)) != 0) {
        printf("Failed to process algo.\n");
        return 1;
    }
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        printf("%d ", output[i]);
    }
    printf("\n");

    DeinitFunc deinit = (DeinitFunc)GetProcAddress(hinstLib, "algo_deinit");
    if (!deinit) {
        printf("Failed to get function.\n");
        return 1;
    }
    deinit(algo_handle);

    FreeLibrary(hinstLib);

    return 0;
}

#endif
