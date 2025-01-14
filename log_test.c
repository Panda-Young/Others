/***************************************************************************
 * Description: test log
 * version: 0.1.0
 * Author: Panda-Young
 * Date: 2025-01-07 12:44:39
 * Copyright (c) 2025 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#include "log.h"

int main()
{
    char c = 'A';

    LOGD2("This is a debug log without context.");
    LOGD("This is a debug log with value: %s", &c);
    LOGI2("This is an info log without context.");
    LOGI("This is an info log with value: %d", c);
    LOGW2("This is a warning log without context.");
    LOGW("This is a warning log with value: %c", c);
    LOGE2("This is an error log without context.");
    LOGE("This is an error log with value: %p", &c);

    return 0;
}
