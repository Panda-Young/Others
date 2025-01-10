/* **************************************************************
 * @Description: algorithm API wrapper
 * @Date: 2024-05-29 01:45:45
 * @Version: 0.1.0
 * @Author: 1641140221@qq.com
 * @Copyright (c) 2024 by @AAC Technologies, All Rights Reserved.
 **************************************************************/
#include "AlgoAPI.hpp"
#include "log.h"
#include <dlfcn.h>

namespace test
{
    AlgoAPI::AlgoAPI(char *lib_name)
    {
        shared_lib_handle = dlopen(lib_name, RTLD_LAZY);
        if (!shared_lib_handle) {
            const char *errorMsg = dlerror();
            if (errorMsg) {
                LOGE("Error: %s", errorMsg);
                if (strstr(errorMsg, "cannot open shared object file")) {
                    LOGW("Hint: It looks like the shared library: %s is missing. ", lib_name);
                    LOGW("Try running: export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/%s",
                         lib_name);
                }
            }
            return;
        }

        get_version = (AlgoGetVersionFunc)dlsym(shared_lib_handle, "get_algo_version");
        if (!get_version) {
            LOGE("Failed to get get_algo_version");
            return;
        }

        init = (AlgoInitFunc)dlsym(shared_lib_handle, "algo_init");
        if (!init) {
            LOGE("Failed to get algo_init");
            return;
        }

        deinit = (AlgoDeinitFunc)dlsym(shared_lib_handle, "algo_deinit");
        if (!deinit) {
            LOGE("Failed to get algo_deinit");
            return;
        }

        set_param = (AlgoSetParamFunc)dlsym(shared_lib_handle, "algo_set_param");
        if (!set_param) {
            LOGE("Failed to get set_algo_param");
            return;
        }

        process = (AlgoProcessFunc)dlsym(shared_lib_handle, "algo_process");
        if (!process) {
            LOGE("Failed to get algo_process");
            return;
        }
    }

    AlgoAPI::~AlgoAPI()
    {
        if (shared_lib_handle) {
            dlclose(shared_lib_handle);
        }
    }

    int AlgoAPI::get_algo_version(char *version)
    {
        if (get_version == NULL) {
            LOGE("get_version is NULL");
            return -1;
        }
        return get_version(version);
    }

    void *AlgoAPI::algo_init()
    {
        if (init == NULL) {
            LOGE("init is NULL");
            return NULL;
        }
        return init();
    }

    void AlgoAPI::algo_deinit(void *algo_handle)
    {
        if (deinit == NULL) {
            LOGE("deinit is NULL");
            return;
        }
        deinit(algo_handle);
    }

    int AlgoAPI::set_algo_param(void *algo_handle, algo_param_t cmd, void *param, uint32_t param_size)
    {
        if (set_param == NULL) {
            LOGE("set_param is NULL");
            return -1;
        }
        return set_param(algo_handle, cmd, param, param_size);
    }

    int AlgoAPI::algo_process(void *algo_handle, void *input, void *output, int block_size)
    {
        if (process == NULL) {
            LOGE("process is NULL");
            return -1;
        }
        return process(algo_handle, input, output, block_size);
    }
}
