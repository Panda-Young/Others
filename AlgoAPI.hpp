/* **************************************************************
 * @Description: algorihtm API wrapper
 * @Date: 2024-05-29 01:37:27
 * @Version: 0.1.0
 * @Author: 1641140221@qq.com
 * @Copyright (c) 2024 by @Panda-Young, All Rights Reserved.
 **************************************************************/

#ifndef _ALGO_API_H
#define _ALGO_API_H

#include <stdint.h>

typedef enum algo_param {
    SET_PARAM1 = 1,
    SET_PARAM2,
    SET_PARAM3,
    SET_PARAM4,
} algo_param_t;

namespace test
{
#ifdef __cplusplus
    extern "C" {
#endif
    typedef int (*AlgoGetVersionFunc)(char *version);
    typedef void *(*AlgoInitFunc)();
    typedef void (*AlgoDeinitFunc)(void *algo_handle);
    typedef int (*AlgoSetParamFunc)(void *algo_handle, algo_param_t cmd, void *param, uint32_t param_size);
    typedef int (*AlgoProcessFunc)(void *algo_handle, void *input, void *output, int block_size);
#ifdef __cplusplus
    }
#endif

    class AlgoAPI
    {
    public:
        AlgoAPI(char *lib_name);
        ~AlgoAPI();
        int get_algo_version(char *version);
        void *algo_init();
        void algo_deinit(void *algo_handle);
        int set_algo_param(void *algo_handle, algo_param_t cmd, void *param, uint32_t param_size);
        int algo_process(void *algo_handle, void *input, void *output, int block_size);
        void *shared_lib_handle;

    private:
        AlgoGetVersionFunc get_version;
        AlgoInitFunc init;
        AlgoDeinitFunc deinit;
        AlgoSetParamFunc set_param;
        AlgoProcessFunc process;
    };

}
#endif // _ALGO_API_H
