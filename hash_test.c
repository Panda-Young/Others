/*
 * @Description: hash test
 * @Date: 2024-05-11 16:18:36
 * @Version: 0.1.0
 * @Author: pandapan@aactechnologies.com
 * Copyright (c) 2024 by @AAC Technologies, All Rights Reserved.
 */
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

// ��ϣ�����������ַ����������Լ򵥵�ʹ�� g_str_hash
GHashFunc hash_func = g_str_hash;

// ��Ⱥ����������ַ����������Լ򵥵�ʹ�� g_str_equal
GEqualFunc equal_func = g_str_equal;

// ������
int main(void)
{
    // ����һ����ϣ��
    GHashTable *hash_table = g_hash_table_new_full(hash_func, equal_func, g_free, g_free);

    // ���ϣ�������Ԫ��
    g_hash_table_insert(hash_table, g_strdup("apple"), g_strdup("red"));
    g_hash_table_insert(hash_table, g_strdup("banana"), g_strdup("yellow"));
    g_hash_table_insert(hash_table, g_strdup("cherry"), g_strdup("red"));

    // ���Ҳ���ӡԪ��
    const char *color = g_hash_table_lookup(hash_table, "banana");
    if (color) {
        printf("find result: Banana is %s\n", color);
    }

    // ������ϣ����ӡ����Ԫ��
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, hash_table);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        printf("%s => %s\n", (char *)key, (char *)value);
    }

    // ���ٹ�ϣ������ͷ����м���ֵ����Ϊ�����ڴ�����ϣ��ʱָ�������ٺ�����
    g_hash_table_destroy(hash_table);

    return 0;
}

/*Compile Command: 
    Linux:
        pkg-config --cflags --libs glib-2.0
    Windows:
        -I"C:\Program Files\msys64\mingw64\include\glib-2.0" -I"C:\Program Files\msys64\mingw64\lib\glib-2.0\include" -L"C:\Program Files\msys64\mingw64\lib" -lintl -lglib-2.0
*/
