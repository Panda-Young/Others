/***************************************************************************
 * Description: header file for algo example
 * version: 0.1.0
 * Author: Panda-Young
 * Date: 2025-01-06 23:07:49
 * Copyright (c) 2025 by Panda-Young, All Rights Reserved.
 **************************************************************************/
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
