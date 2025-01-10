/* **************************************************************
 * @Description: struct for class
 * @Date: 2024-02-18 14:13:48
 * @Version: 0.1.0
 * @Author: pandapan@aactechnologies.com
 * @Copyright (c) 2024 by @AAC Technologies, All Rights Reserved.
 **************************************************************/
#include <stdio.h>

typedef struct BOX_t {
    double length;
    double breadth;
    double height;
    // void (*BOX_t)();
    // void (*_BOX_t)();
    double (*getVolume)(struct BOX_t*); // 函数指针，返回体积
    double (*getSurfaceArea)(struct BOX_t*); // 函数指针，返回表面积
    void (*setDimensions)(struct BOX_t*, double, double, double); // 函数指针，设置尺寸
} *BOX_p;

void __attribute__((constructor)) BOX_t() {
    printf("BOX_t is being created\n");
}

void __attribute__((destructor)) _BOX_t() {
    printf("BOX_t is being destroyed\n");
}

double getVolume(struct BOX_t* box) {
    return box->length * box->breadth * box->height;
}

double getSurfaceArea(struct BOX_t* box) {
    return 2 * (box->length * box->breadth + box->breadth * box->height + box->height * box->length);
}

void setDimensions(struct BOX_t* box, double length, double breadth, double height) {
    box->length = length;
    box->breadth = breadth;
    box->height = height;
}

int main() {
    struct BOX_t box = {0};
    // box.BOX_t = BOX_t;
    // box._BOX_t = _BOX_t;
    box.getVolume = getVolume;
    box.getSurfaceArea = getSurfaceArea;
    box.setDimensions = setDimensions;

    box.setDimensions(&box, 2.0, 3.0, 4.0);
    printf("Volume: %f\n", box.getVolume(&box));
    printf("Surface Area: %f\n", box.getSurfaceArea(&box));

    return 0;
}
