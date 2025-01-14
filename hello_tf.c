/*
 * @Description: 
 * @version: 0.1.0
 * @Author: Panda-Young
 * @Date: Fri 10 Jan 2025 02:47:36 PM CST
 * Copyright (c) 2023 by Panda-Young, All Rights Reserved.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"

#include <tensorflow/c/c_api.h>

int main() {
  LOGI("Hello from TensorFlow C library version %s\n", TF_Version());
  return 0;
}

// compile with:
// gcc -o hello_tf hello_tf.c log.c -ltensorflow && ./hello_tf
