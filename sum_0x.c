/*
 * @Description: sum of hex numbers
 * @Date: 2024-01-10 18:46:31
 * @Version: 0.1.0
 * @Author: Panda-Young
 * Copyright (c) 2024 by @Panda-Young, All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_NUM UCHAR_MAX

int main()
{
    char str[MAX_NUM];
    int sum = 0;
    printf("\033[34mPlease input some decimal numbers:\033[0m\n");
    while (fgets(str, MAX_NUM, stdin) != NULL)
    {
        if (str[0] == '\n')
            break;
        sum += strtol(str, NULL, 16);
    }
    printf("0x%x\t%d\n", sum, sum);
    return 0;
}
