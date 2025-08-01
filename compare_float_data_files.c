/***************************************************************************
 * Description: compare two binary float data files
 * version: 0.1.0
 * Author: Panda-Young
 * Date: 2025-01-06 23:07:49
 * Copyright (c) 2025 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s <file1> <file2>\n", argv[0]);
        return 1;
    }

    FILE *fp1 = fopen(argv[1], "rb");
    if (fp1 == NULL) {
        printf("Error: %s\n", strerror(errno));
        return 1;
    }
    FILE *fp2 = fopen(argv[2], "rb");
    if (fp2 == NULL) {
        printf("Error: %s\n", strerror(errno));
        return 1;
    }

    float f1, f2;
    float sum_diff = 0;
    float max_diff = FLT_MIN;
    int count = 0;
    while (fread(&f1, sizeof(float), 1, fp1) == 1 && fread(&f2, sizeof(float), 1, fp2) == 1) {
        float diff = f1 - f2;
        sum_diff += diff;
        if (diff > max_diff) {
            max_diff = diff;
        }
        count++;
    }

    printf("Average difference: %.8f\n", sum_diff / count);
    printf("Maximum difference: %.8f\n", max_diff);

    if (fp1 != NULL) {
        if (fclose(fp1) != 0) {
            printf("Error: %s\n", strerror(errno));
            return 1;
        }
        fp1 = NULL;
    }
    if (fp2 != NULL) {
        if (fclose(fp2) != 0) {
            printf("Error: %s\n", strerror(errno));
            return 1;
        }
        fp2 = NULL;
    }

    return 0;
}
