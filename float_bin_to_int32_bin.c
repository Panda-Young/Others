/***************************************************************************
 * Description: convert float to int32
 * version: 0.1.0
 * Author: Panda-Young
 * Date: 2025-08-01 21:33:30
 * Copyright (c) 2025 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

int main(int argc, char *argv[])
{
    FILE *fpin = NULL, *fpout = NULL;
    char output_filename[_MAX_PATH] = {0};
    if (argc < 2)
    {
        printf("Usage: %s <float binary file path> <int32 binary file path>\n", argv[0]);
        return -1;
    } else if (argc == 2) {
        fpin = fopen(argv[1], "rb");
        if (fpin == NULL) {
            printf("Error: Cannot open input file %s\n", argv[1]);
            return -1;
        }
        
        // 安全地创建输出文件名
        if (output_filename == NULL) {
            printf("Error: Memory allocation failed\n");
            fclose(fpin);
            return -1;
        }
        strcpy(output_filename, argv[1]);
        strcat(output_filename, ".int32.bin");
        fpout = fopen(output_filename, "wb");
        if (fpout == NULL) {
            printf("Error: Cannot create output file %s\n", output_filename);
            fclose(fpin);
            return -1;
        }
    } else {
        fpin = fopen(argv[1], "rb");
        if (fpin == NULL) {
            printf("Error: Cannot open input file %s\n", argv[1]);
            return -1;
        }
        fpout = fopen(argv[2], "wb");
        if (fpout == NULL) {
            printf("Error: Cannot create output file %s\n", argv[2]);
            fclose(fpin);
            return -1;
        }
    }

    // 逐个读取float数据并转换为int32_t
    float input_value;
    int32_t output_value;
    size_t read_count;
    
    while ((read_count = fread(&input_value, sizeof(float), 1, fpin)) == 1) {
        // 按照您的需求进行转换: output_value = input_value * 2^31
        // 这样可以将[0,1)范围的浮点数映射到[0,2^31)的整数范围，便于HEX比较
        double scaled_value = (double)input_value * (double)(1U << 31);
        
        // 处理边界情况
        if (scaled_value >= (double)INT32_MAX) {
            output_value = INT32_MAX;
        } else if (scaled_value <= (double)INT32_MIN) {
            output_value = INT32_MIN;
        } else {
            output_value = (int32_t)scaled_value;
        }
        
        // 写入到输出文件
        if (fwrite(&output_value, sizeof(int32_t), 1, fpout) != 1) {
            printf("Error: Failed to write to output file\n");
            fclose(fpout);
            fclose(fpin);
            return -1;
        }
    }
    
    if (ferror(fpin)) {
        printf("Error: Failed to read input file\n");
        fclose(fpout);
        fclose(fpin);
        return -1;
    }

    printf("Conversion to \"%s\" completed successfully.\n", output_filename);
    fclose(fpout);
    fclose(fpin);
    return 0;
}
