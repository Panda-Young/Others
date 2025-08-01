/***************************************************************************
 * Description: algorithm exxample
 * version: 0.1.0
 * Author: Panda-Young
 * Date: 2024-05-11 11:08:17
 * Copyright (c) 2024 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#ifndef _ALGO_EXAMPLE_H
#define _ALGO_EXAMPLE_H

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(_WIN64) || defined(__WIN64__) || defined(WIN64)
    #ifdef ALGO_EXPORTS
        #define ALGO_API __declspec(dllexport)
    #else
        #define ALGO_API __declspec(dllimport)
    #endif
#else
    #define ALGO_API
#endif

#define E_OK 0
#define E_VERSION_BUFFER_NULL -1
#define E_ALGO_HANDLE_NULL -2
#define E_PARAM_BUFFER_NULL -3
#define E_PARAM_SIZE_INVALID -4
#define E_ALLOCATE_FAILED -5
#define E_PARAM_OUT_OF_RANGE -6

typedef enum algo_param {
    ALGO_PARAM1 = 1,
    ALGO_PARAM2,
    ALGO_PARAM3,
    ALGO_PARAM4,
} algo_param_t;

ALGO_API int get_algo_version(char *version);
ALGO_API void *algo_init();
ALGO_API void algo_deinit(void *algo_handle);
ALGO_API int algo_set_param(void *algo_handle, algo_param_t cmd, void *param, int param_size);
ALGO_API int algo_get_param(void *algo_handle, algo_param_t cmd, void *param, int param_size);
ALGO_API int algo_process(void *algo_handle, const float *input, float *output, int block_size);

#endif

/* Compile Command:
Windows Visal Studio:
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    cl /LD /DALGO_EXPORTS algo_example.c log.c /link /out:algo_example.dll
Windows MinGW:
    gcc -shared -DALGO_EXPORTS -fPIC algo_example.c log.c -lm -o algo_example.dll
Linux:
    gcc -shared -DALGO_EXPORTS -fPIC algo_example.c log.c -lm -o libalgo_example.so
*/
