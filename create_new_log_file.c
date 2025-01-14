/*
 * @Description:
 * @Date: 2024-05-24 19:37:42
 * @Version: 0.1.0
 * @Author: 1641140221@qq.com
 * Copyright (c) 2024 by @Panda-Young, All Rights Reserved.
 */
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

int main()
{
    DIR *d;
    struct dirent *dir;
    d = opendir("test_dir");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            char filepath[PATH_MAX] = {0};
            sprintf(filepath, "test_dir/%s", dir->d_name);
            if (strstr(dir->d_name, "test_") != NULL) {
                if (remove(filepath) == 0) {
                    printf("Deleted file: %s\n", filepath);
                } else {
                    printf("Unable to delete file: %s Due to %s\n", filepath, strerror(errno));
                }
            }
        }
    }
    closedir(d);

    char path[100] = "test_dir/test_";
    sprintf(&path[strlen(path)], "%p", path);
    sprintf(&path[strlen(path)], ".log");
    printf("Path: %s\n", path);
    FILE *fp = fopen(path, "w");
    if (fp == NULL) {
        printf("Unable to create file: %s\n", path);
    }

    fwrite("Hello, World!", 1, 13, fp);
    fclose(fp);

    return 0;
}
