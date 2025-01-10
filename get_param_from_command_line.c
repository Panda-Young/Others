/***************************************************************************
 * Description: get param from command line
 * version: 0.1.0
 * Author: Panda-Young
 * Date: 2024-05-06 23:18:06
 * Copyright (c) 2024 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_PARAMS 10
#define printf_grey(fmt, args...) printf("\e[1;30m" fmt "\e[0m", ##args)
#define printf_red(fmt, args...) printf("\e[1;31m" fmt "\e[0m", ##args)
#define printf_green(fmt, args...) printf("\e[1;32m" fmt "\e[0m", ##args)
#define printf_yellow(fmt, args...) printf("\e[1;33m" fmt "\e[0m", ##args)
#define printf_blue(fmt, args...) printf("\e[1;34m" fmt "\e[0m", ##args)
#define printf_purple(fmt, args...) printf("\e[1;35m" fmt "\e[0m", ##args)
#define printf_light_blue(fmt, args...) printf("\e[1;36m" fmt "\e[0m", ##args)
#define printf_white(fmt, args...) printf("\e[1;37m" fmt "\e[0m", ##args)

static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'v'},
    {"integer", required_argument, 0, 'i'},
    {"string", required_argument, 0, 's'},
    {0, 0, 0, 0}};

void display_help()
{
    printf_yellow("Usage:\n");
    printf_yellow("  myprogram [-h] [-v] [-s <string>] [-i <integer>]\n");
    printf_yellow("  myprogram [--help] [--version] [--string <string>] [--integer <integer>]\n");
    printf_yellow("\n");
    printf_yellow("Options:\n");
    printf_yellow("  -h, --help     Display this help message\n");
    printf_yellow("  -v, --version  Display version information\n");
    printf_yellow("  -s, --string   Followed by a string\n");
    printf_yellow("  -i, --integer  Followed by an integer\n");
}

void display_version()
{
    printf_blue("Version 1.1.0\n");
}

int get_param_from_command_line(int argc, char *argv[])
{
    int opt;
    int option_index = 0;
    int integer_values[MAX_PARAMS];
    char *string_values[MAX_PARAMS];
    int integer_count = 0;
    int string_count = 0;

    while ((opt = getopt_long(argc, argv, "hvi:s:", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'h':
            display_help();
            exit(0);
        case 'v':
            display_version();
            exit(0);
        case 'i':
            if (integer_count < MAX_PARAMS) {
                integer_values[integer_count++] = atoi(optarg);
                printf("Parsed integer: %d\n", integer_values[integer_count - 1]);
            }
            break;
        case 's':
            if (optarg && string_count < MAX_PARAMS) {
                string_values[string_count++] = optarg;
                printf("get string: %s\n", string_values[string_count - 1]);
            }
            break;
        default:
            display_help();
            return -1;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        display_help();
        return 0;
    } else {
        if (get_param_from_command_line(argc, argv) != 0) {
            return -1;
        }
    }
}
