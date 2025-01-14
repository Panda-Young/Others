/***************************************************************************
 * Description: Example of using structs and function pointers in C
 * Version: 0.1.0
 * Author: Panda-Young
 * Date: 2025-01-06 23:07:50
 * Copyright (c) 2025 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#include <stdio.h>

// Define a struct to represent a box with dimensions and function pointers
typedef struct BOX_t {
    double length;
    double breadth;
    double height;
    // void (*BOX_t)();
    // void (*_BOX_t)();
    double (*getVolume)(struct BOX_t*); // Function pointer to calculate volume
    double (*getSurfaceArea)(struct BOX_t*); // Function pointer to calculate surface area
    void (*setDimensions)(struct BOX_t*, double, double, double); // Function pointer to set dimensions
} *BOX_p;

// Constructor function to be called when the struct is created
void __attribute__((constructor)) BOX_t() {
    printf("BOX_t is being created\n");
}

// Destructor function to be called when the struct is destroyed
void __attribute__((destructor)) _BOX_t() {
    printf("BOX_t is being destroyed\n");
}

// Function to calculate the volume of the box
double getVolume(struct BOX_t* box) {
    return box->length * box->breadth * box->height;
}

// Function to calculate the surface area of the box
double getSurfaceArea(struct BOX_t* box) {
    return 2 * (box->length * box->breadth + box->breadth * box->height + box->height * box->length);
}

// Function to set the dimensions of the box
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
