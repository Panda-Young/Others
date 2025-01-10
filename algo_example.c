/***************************************************************************
 * Description: algorithm exxample
 * version: 0.1.0
 * Author: Panda-Young
 * Date: 2024-05-11 11:08:17
 * Copyright (c) 2024 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#include "algo_example.h"
#include "log.h"

#define VERSION "0.1.0"
#define MAX_BUF_SIZE 1024

typedef struct algo_handle {
    char param1;
    int param2;
    char param3[MAX_BUF_SIZE];
    float *param4;
} algo_handle_t, *p_algo_handle_t;

__attribute__((constructor)) void algo_enter()
{
    LOGI("algo_enter");
}

__attribute__((destructor)) void algo_exit()
{
    LOGI("algo_exit");
}

int get_algo_version(char *version)
{
    if (version == NULL) {
        LOGE("version is NULL");
        return -1;
    }
    strcpy(version, VERSION);
    return 0;
}

void *algo_init()
{
    void *algo_handle = (p_algo_handle_t)malloc(sizeof(algo_handle_t));
    if (algo_handle == NULL) {
        LOGE("allocate for algo_handle_t failed");
        return NULL;
    }
    memset(algo_handle, 0, sizeof(algo_handle_t));
    LOGI("algo_init OK");
    return algo_handle;
}

void algo_deinit(void *algo_handle)
{
    if (!algo_handle) {
        LOGE("algo_handle is NULL");
        return;
    }
    p_algo_handle_t algo_handle_ptr = (p_algo_handle_t)algo_handle;
    if (algo_handle_ptr->param4 != NULL) {
        free(algo_handle_ptr->param4);
        algo_handle_ptr->param4 = NULL;
    }
    free(algo_handle);
    algo_handle = NULL;
    LOGI("algo_deinit OK");
}

int algo_set_param(void *algo_handle, algo_param_t cmd, void *param, uint32_t param_size)
{
    if (algo_handle == NULL) {
        LOGE("algo_handle is NULL");
        return -1;
    }
    if (param == NULL) {
        LOGE("param is NULL");
        return -1;
    }

    p_algo_handle_t algo_handle_ptr = (p_algo_handle_t)algo_handle;
    switch (cmd) {
    case SET_PARAM1: {
        if (param_size != sizeof(char)) {
            LOGE("param_size is not correct");
            return -1;
        }
        algo_handle_ptr->param1 = *(char *)param;
        LOGI("set param1: %c", algo_handle_ptr->param1);
        break;
    }
    case SET_PARAM2: {
        if (param_size != sizeof(int)) {
            LOGE("param_size is not correct");
            return -1;
        }
        algo_handle_ptr->param2 = *(int *)param;
        LOGI("set param2: %d", algo_handle_ptr->param2);
        break;
    }
    case SET_PARAM3: {
        if (param_size > MAX_BUF_SIZE) {
            LOGE("param_size is too large");
            return -1;
        }
        memset(algo_handle_ptr->param3, 0, MAX_BUF_SIZE);
        memcpy(algo_handle_ptr->param3, param, param_size);
        LOGI("set param3: %s", algo_handle_ptr->param3);
        break;
    }
    case SET_PARAM4: {
        if (param_size > MAX_BUF_SIZE) {
            LOGE("param_size is too large");
            return -1;
        }
        algo_handle_ptr->param4 = (float *)malloc(param_size);
        if (algo_handle_ptr->param4 == NULL) {
            LOGE("allocate for param4 failed");
            return -1;
        }
        memcpy(algo_handle_ptr->param4, param, param_size);
        break;
    }
    default:
        LOGE("cmd is invalid");
        return -1;
    }
    return 0;
}

int algo_process(void *algo_handle, void *input, void *output, int block_size)
{
    if (algo_handle == NULL) {
        LOGE("algo_handle is NULL");
        return -1;
    }
    if (input == NULL) {
        LOGE("input is NULL");
        return -1;
    }
    if (output == NULL) {
        LOGE("output is NULL");
        return -1;
    }
    if (block_size <= 0) {
        LOGE("block_size is not correct");
        return -1;
    }
    p_algo_handle_t algo_handle_ptr = (p_algo_handle_t)algo_handle;
 
    // memcpy(output, input, block_size);

    int *input_buffer = (int *)input;
    int *output_buffer = (int *)output;

    for (int i = 0; i < block_size / sizeof(int); i++) {
        int result = input_buffer[i] * algo_handle_ptr->param2;
        if (result > INT_MAX) {
            output_buffer[i] = INT_MAX;
        } else if (result < INT_MIN) {
            output_buffer[i] = INT_MIN;
        } else {
            output_buffer[i] = result;
        }
    }

    return 0;
}

/*Compile Command:
    Unix/Linux:
        gcc -shared -fPIC -Wall algo_example.c log.c -o libalgo_example.so
    Windows:
        gcc -shared -Wall algo_example.c log.c -o libalgo_example.dll
Compile options：
    `-shared`选项告诉编译器生成一个动态链接库
    `-fPIC`选项告诉编译器生成位置无关的代码（Position Independent Code，PIC）。
        这是一种特殊类型的机器代码，它可以在内存中任何位置执行。动态链接库在加载时可能会被加载到内存的任意位置
        `-fpic`生成的代码对全局偏移表（Global Offset Table，GOT）的大小有限制，而`-fPIC`生成的代码没有这个限制。
        在Windows平台上，所有的代码默认都是位置无关的，因此这两个选项在Windows平台上没有效果。
*/
