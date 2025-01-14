/*
 * @Description: read binary float data
 * @Date: 2024-03-25 11:29:51
 * @Version: 0.1.0
 * @Author: Panda-Young
 * Copyright (c) 2024 by @Panda-Young, All Rights Reserved.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        printf("Error: %s\n", strerror(errno));
        return 1;
    }

    float f = 0.0f;
    int count = 0;
    while (fread(&f, sizeof(float), 1, fp) == 1) {
        printf("%f\t", f);
        if (count++ % 8 == 0) {
            printf("\n");
        }
    }

    if (fp != NULL) {
        if (fclose(fp) != 0) {
            printf("Error: %s\n", strerror(errno));
            return 1;
        }
        fp = NULL;
    }

    return 0;
}
