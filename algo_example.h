/* **************************************************************
 * @Description: header for algorithm example
 * @Date: 2024-05-16 17:24:47
 * @Version: 0.1.0
 * @Author: pandapan@aactechnologies.com
 * @Copyright (c) 2024 by @AAC Technologies, All Rights Reserved. 
 **************************************************************/

#ifndef _TEST_H
#define _TEST_H

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum algo_param {
    SET_PARAM_START = 0,
    SET_PARAM1,
    SET_PARAM2,
    SET_PARAM3,
    SET_PARAM4,
    SET_PARAM_END,
} algo_param_t;

int get_algo_version(char *version);
void *algo_init();
void algo_deinit(void *algo_handle);
int algo_set_param(void *algo_handle, algo_param_t cmd, void *param, uint32_t param_size);
int algo_process(void *algo_handle, void *input, void *output, int block_size);

#endif
