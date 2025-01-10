#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <so file>\n", argv[0]);
        return -1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror("fopen");
        return -2;
    }
    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    printf("file size: %d\n", file_size);
    fclose(fp);

    void *handle = dlopen(argv[1], RTLD_NOW);
    if (!handle) {
        printf("Failed to open library: %s\n", dlerror());
        return -3;
    }
    printf("Library opened successfully\n");
    dlclose(handle);

    return 0;
}
