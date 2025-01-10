/***************************************************************************
 * Description: get known folder path
 * version: 0.1.0
 * Author: Panda-Young
 * Date: 2024-05-11 11:23:29
 * Copyright (c) 2024 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#if defined(__MINGW32__) || defined(__MINGW64__)
#include <shlobj.h>
#include <windows.h>

int get_desktop_path(char *DesktopPath)
{
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, DesktopPath))) {
        return 0;
    } else {
        PWSTR wpath;
        if (SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_Desktop, 0, NULL, &wpath))) {
            setlocale(LC_ALL, "");
            wcstombs(DesktopPath, wpath, MAX_PATH);
            CoTaskMemFree(wpath);
            return 0;
        } else {
            printf("Failed to get Desktop path.\n");
            return -1;
        }
    }
}

int get_temp_path(char *TemplatesPath)
{
    if (GetTempPath(MAX_PATH, TemplatesPath) > 0) {
        return 0;
    } else {
        PWSTR wpath;
        if (SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_Templates, 0, NULL, &wpath))) {
            setlocale(LC_ALL, "");
            wcstombs(TemplatesPath, wpath, MAX_PATH);
            CoTaskMemFree(wpath);
            return 0;
        } else {
            printf("Failed to get Templates path.\n");
            return -1;
        }
    }
}

int main()
{
    char out_path[PATH_MAX] = {0};
    char *OUT_FILE_NAME = "out_player.pcm";

    if (get_desktop_path(out_path) == 0) {
        printf("Desktop path: %s\n", out_path);
        strcat(out_path, "\\");
        strcat(out_path, OUT_FILE_NAME);
        printf("Output path: %s\n", out_path);
    } else {
        printf("Failed to get Desktop path.\n");
    }

    if (get_temp_path(out_path) == 0) {
        printf("Temp path: %s\n", out_path);
        strcat(out_path, OUT_FILE_NAME);
        printf("Output path: %s\n", out_path);
    } else {
        printf("Failed to get Temp path.\n");
    }

    return 0;
}

/*Compile Command:
    gcc get_known_foleder_path.c -lshell32 -lole32 -luuid -o get_known_foleder_path  && get_known_foleder_path
*/

#else
#include <sys/types.h>
#include <pwd.h>

int get_desktop_path(char *desktop_path, size_t len) {
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    if (snprintf(desktop_path, len, "%s/Desktop", homedir) >= len) {
        printf("The path is too long for the provided buffer\n");
        return -1;
    }
    return 0;
}

int get_temp_path(char *temp_path, size_t len) {
    char* tmpdir = getenv("TMPDIR");
    char* path = tmpdir ? tmpdir : "/tmp";
    if (snprintf(temp_path, len, "%s", path) >= len) {
        printf("The path is too long for the provided buffer\n");
        return -1;
    }
    return 0;
}

int main() {
    char out_path[PATH_MAX] = {0};
    char *OUT_FILE_NAME = "out_player.pcm";

    if (get_desktop_path(out_path, sizeof(out_path)) == 0) {
        printf("Desktop path: %s\n", out_path);
        strcat(out_path, "/");
        strcat(out_path, OUT_FILE_NAME);
        printf("Output path: %s\n", out_path);
    } else {
        printf("Failed to get Desktop path.\n");
    }

    if (get_temp_path(out_path, sizeof(out_path)) == 0) {
        printf("Temp path: %s\n", out_path);
        strcat(out_path, "/");
        strcat(out_path, OUT_FILE_NAME);
        printf("Output path: %s\n", out_path);
    } else {
        printf("Failed to get Temp path.\n");
    }

    return 0;
}

#endif
