#ifndef ARRAY_UTILS_H
#define ARRAY_UTILS_H
#include <stdio.h>
#include <stdlib.h>

#define CONTIG_ARRAY

float *NewFloat1D(int d1, float fInit = 0.0f);
float *DeleteFloat1D(float *pArray, int d1 = 0);
float **NewFloat2D(int d1, int d2, float fInit = 0.0);
float **DeleteFloat2D(float **ppArray, int d1, int d2 = 0);
float ***NewFloat3D(int d1, int d2, int d3, float fInit = 0.0);
float ***DeleteFloat3D(float ***pppArray, int d1, int d2, int d3 = 0);

double *NewDouble1D(int d1);
double *DeleteDouble1D(double *pArray, int d1 = 0);

int *NewInt1D(int d1);
int *DeleteInt1D(int *pArray, int d1);
int *DeleteInt1D(int *pArray);
int **NewInt2D(int d1, int d2);
int **DeleteInt2D(int **ppArray, int d1, int d2);

short *NewShort1D(int d1);
short *DeleteShort1D(short *pArray, int d1 = 0);

char **DeleteChar2D(char **ppArray, int d1, int d2 = 0);

#endif
