/* **************************************************************
 * @Description: use algorithm API
 * @Date: 2024-05-29 02:05:26
 * @Version: 0.1.0
 * @Author: 1641140221@qq.com
 * @Copyright (c) 2024 by @Panda-Young, All Rights Reserved.
 **************************************************************/
#include "AlgoAPI.hpp"
#include <stdio.h>

#define SAMPLE_COUNT 8

int main()
{
    void *m_algo_handle = NULL;
    char LIB_NAME[] = "libalgo_example.so";
    test::AlgoAPI *algo_instance = new test::AlgoAPI(LIB_NAME);
    if (!algo_instance) {
        printf("Failed to create AlgoAPI instance.\n");
        return 1;
    }
    if (algo_instance->shared_lib_handle == NULL) {
        printf("Failed to load shared library.\n");
        return 1;
    }

    char version[128] = {0};
    if (algo_instance->get_algo_version(version) == 0) {
        printf("Algorithm Version: %s\n", version);
    } else {
        printf("Failed to get version.\n");
    }

    if ((m_algo_handle = algo_instance->algo_init()) == NULL) {
        printf("Failed to init algo.\n");
        return 1;
    } else {
        printf("Algorithm init success. m_algo_handle: %p\n", m_algo_handle);
    }

    int param2 = 2;
    if (algo_instance->set_algo_param(m_algo_handle, SET_PARAM2, (void *)&param2, sizeof(int)) != 0) {
        printf("Failed to set algo param cmd %d size %lu.\n", SET_PARAM2, sizeof(int));
    } else {
        printf("Algorithm set param cmd %d size %lu Bytes success.\n", SET_PARAM2, sizeof(int));
    }

    char param3[] = "param3";
    if (algo_instance->set_algo_param(m_algo_handle, SET_PARAM3, param3, sizeof(param3)) != 0) {
        printf("Failed to set algo param cmd %d size %lu.\n", SET_PARAM3, sizeof(param3));
    } else {
        printf("Algorithm set param cmd %d size %lu Bytes success.\n", SET_PARAM3, sizeof(param3));
    }

    int input[SAMPLE_COUNT] = {0, 1, 2, 3, 4, 5, 6, 7};
    int output[SAMPLE_COUNT] = {0};
    if (algo_instance->algo_process(m_algo_handle, input, output, SAMPLE_COUNT * sizeof(int)) != 0) {
        printf("Failed to process algo.\n");
        return 1;
    }

    for (int i = 0; i < SAMPLE_COUNT; i++) {
        printf("%d ", output[i]);
    }
    printf("\n");

    algo_instance->algo_deinit(m_algo_handle);

    delete algo_instance;

    return 0;
}

/*Add library path:
    export LD_LIBRARY_PATH=.
*/
