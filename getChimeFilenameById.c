/***************************************************************************
 * Description: get the file path by id from xml file
 * version: 0.1.0
 * Author: Panda-Young
 * Date: 2024-05-30 16:17:12
 * Copyright (c) 2024 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getFilenameById(const char *filename, int id, char *path)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        return -1;
    }

    char line[256];
    char filePath[256] = "";
    int found_id = -1;
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "<Chime3DMapping")) {
            char *filePath_start = strstr(line, "filePath");
            if (filePath_start) {
                filePath_start = strchr(filePath_start, '='); // find '=' character
                if (filePath_start) {
                    filePath_start++; // skip past '='
                    while (*filePath_start == ' ')
                        filePath_start++; // skip past spaces
                    if (*filePath_start == '\"')
                        filePath_start++; // skip past '"'
                    char *filePath_end = strstr(filePath_start, "\"");
                    if (filePath_end) {
                        strncpy(filePath, filePath_start, filePath_end - filePath_start);
                        filePath[filePath_end - filePath_start] = '\0';
                    }
                }
            }
        } else if (strstr(line, "<pos")) {
            char *id_start = strstr(line, "id=\"");
            if (id_start) {
                found_id = atoi(id_start + 4);
            }

            if (found_id == id) {
                char *filename_start = strstr(line, "fileName");
                if (filename_start) {
                    filename_start = strchr(filename_start, '='); // find '=' character
                    if (filename_start) {
                        filename_start++; // skip past '='
                        while (*filename_start == ' ')
                            filename_start++; // skip past spaces
                        if (*filename_start == '\"')
                            filename_start++; // skip past '"'
                        char *filename_end = strstr(filename_start, "\"");
                        if (filename_end) {
                            strcpy(path, filePath);
                            strncat(path, filename_start, filename_end - filename_start);
                            path[strlen(filePath) + filename_end - filename_start] = '\0';
                            fclose(file);
                            return 0;
                        }
                    }
                }
            }
        }
    }

    fclose(file);
    return -1;
}

int main()
{
    char path[256];
    if (getFilenameById("chime3D.xml", 1, path) == 0) {
        printf("Path: %s\n", path);
    }
    return 0;
}
