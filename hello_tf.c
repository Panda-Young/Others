/* **************************************************************************
 * @Description: test tensorflow c library
 * @Version: 0.1.0
 * @Author: 1641140221@qq.com
 * @Date: 2025-01-10 14:47:51
 * @Copyright (c) 2025 by @AAC Technologies, All Rights Reserved.
 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"

#include <tensorflow/c/c_api.h>

int main() {
    LOGI("Hello from TensorFlow C library version %s\n", TF_Version());
    return 0;
}

/* compile with:
    gcc -o hello_tf hello_tf.c log.c -ltensorflow && ./hello_tf
*/
