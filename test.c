/*
 * @Description: 
 * @Date: 2024-08-10 14:41:06
 * @Version: 0.1.0
 * @Author: error: git config user.email & please set dead value or install git
 * Copyright (c) 2024 by @AAC Technologies, All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


const wchar_t *str = NULL;

void print_str(const wchar_t *str)
{
    printf("str = %ls\n", str);
}


int main(int argc, char *argv[])
{
    str = L"C:\\Program Files\\Common Files\\VST\\aac_mss.onnx";
    print_str(str);

    return 0;
}
