/*
 * @Description:
 * @Date: 2024-05-16 19:45:55
 * @Version: 0.1.0
 * @Author: 1641140221@qq.com
 * Copyright (c) 2024 by @AAC Technologies, All Rights Reserved.
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 辅助函数，用于跳过空白字符
void skip_whitespace(const char **xml)
{
    while (**xml && isspace(**xml)) {
        (*xml)++;
    }
}

// 辅助函数，用于查找并返回XML标签内的字符串
char *get_xml_string(const char **xml, const char *end_tag)
{
    skip_whitespace(xml);
    if (**xml != '"')
        return NULL;
    (*xml)++; // 跳过双引号

    char *start = (char *)*xml;
    while (**xml && **xml != '"' && **xml != '<' && **xml != '>') {
        (*xml)++;
    }

    if (**xml != '"')
        return NULL; // 如果没有找到结束的双引号，返回NULL
    char *end = (char *)(*xml);
    *end = '\0'; // 临时将找到的字符串结尾设为NULL
    (*xml)++;    // 跳过结束的双引号

    // 检查是否遇到了期望的结束标签
    if (strncmp(*xml, end_tag, strlen(end_tag)) == 0) {
        (*xml) += strlen(end_tag); // 跳过结束标签
    } else {
        // 如果不是期望的结束标签，则恢复字符串并返回NULL
        *end = '"';
        return NULL;
    }

    return start;
}

// 修改parse_xml函数以添加printf语句
int parse_xml(const char *filename, int id, char *path, size_t path_size)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return -1;
    }

    char buffer[1024];
    const char *xml = buffer;
    size_t len;
    int found = 0;
    int current_id = 0;

    while ((len = fread(buffer, 1, sizeof(buffer) - 1, file)) > 0) {
        buffer[len] = '\0'; // 确保字符串以NULL结尾

        printf("正在处理缓冲区...\n");

        while (*xml) {
            if (strncmp(xml, "<pos id=\"", 9) == 0) {
                // ...（之前的处理保持不变，但添加printf）...
                printf("找到<pos>标签，id为: %d\n", current_id);

                // ...（继续处理，并添加适当的printf语句来显示进度）...

            } else {
                // ...（跳过其他内容）...
            }

            // ...（检查缓冲区末尾和文件结束）...
        }

        if (found)
            break; // 如果找到了匹配的项，则退出循环
    }

    fclose(file);

    return found ? 0 : -2; // 使用-2表示未找到匹配的id，以区别于文件打开失败
}
// 主测试函数
int main()
{
    const char *filename = "chime3D.xml"; // 替换为你的XML文件名
    int id_to_find = 2;                   // 假设我们要查找id为2的fileName
    char path[1024];                      // 存储解析结果的路径

    printf("开始解析XML文件 '%s' 来查找id为 '%d' 的fileName...\n", filename, id_to_find);

    int result = parse_xml(filename, id_to_find, path, sizeof(path));

    if (result == 0) {
        printf("找到匹配的fileName: '%s'\n", path);
    } else if (result == -1) {
        printf("打开文件失败\n");
    } else {
        printf("未在文件中找到匹配的id\n");
    }

    return 0;
}
