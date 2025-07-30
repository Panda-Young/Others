/* **************************************************************************
 * @Description:
 * @Version: 0.1.0
 * @Author: pandapan@aactechnologies.com
 * @Date: 2025-04-10 10:50:35
 * @Copyright (c) 2025 by @AAC Technologies, All Rights Reserved.
 **************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <binary_file>\n", argv[0]);
        return -1;
    }

    FILE *fpin = fopen(argv[1], "rb");
    if (fpin == NULL)
    {
        printf("Error: cannot open file %s\n", argv[1]);
        return -1;
    }

    char *filename = strrchr(argv[1], '/');
    if (filename == NULL)
    {
        filename = argv[1];
    }
    else
    {
        filename++; // skip '/'
    }

    size_t len = strlen(filename);
    char *outfile = malloc(len + 5); // +5 for ".txt" and '\0'
    if (outfile == NULL)
    {
        fclose(fpin);
        printf("Error: memory allocation failed\n");
        return -1;
    }

    snprintf(outfile, len + 5, "%s.txt", filename);

    FILE *fpout = fopen(outfile, "w");
    if (fpout == NULL)
    {
        fclose(fpin);
        free(outfile);
        printf("Error: cannot create output file %s\n", outfile);
        return -1;
    }

    float data;
    int count = 0;
    while (fread(&data, sizeof(float), 1, fpin) == 1)
    {
        if (fprintf(fpout, "%f ", data) < 0)
        {
            printf("Error: failed to write to output file\n");
            fclose(fpout);
            fclose(fpin);
            free(outfile);
            return -1;
        }
        
        count++;
        if (count % 16 == 0)
        {
            if (fprintf(fpout, "\n") < 0)
            {
                printf("Error: failed to write to output file\n");
                fclose(fpout);
                fclose(fpin);
                free(outfile);
                return -1;
            }
        }
    }

    // 添加最后的换行符（如果文件末尾不是完整的一行）
    if (count % 16 != 0)
    {
        if (fprintf(fpout, "\n") < 0)
        {
            printf("Error: failed to write to output file\n");
            fclose(fpout);
            fclose(fpin);
            free(outfile);
            return -1;
        }
    }

    fclose(fpout);
    fclose(fpin);
    free(outfile);
    return 0;
}
