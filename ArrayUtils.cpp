#include "ArrayUtils.h"
#include <string.h>
// Allocation and Deletion Utility functions

float *NewFloat1D(int d1, float fInit)
{
    float *pArray = new float[d1];
    if (fInit != 0.0f)
        for (int i = 0; i < d1; i++)
            pArray[i] = fInit;
    else
        memset(pArray, 0x00, d1 * sizeof(float));

    return pArray;
}

float *DeleteFloat1D(float *pArray, int d1)
{
    if (pArray)
        delete[] pArray;

    return NULL;
}

float **NewFloat2D(int d1, int d2, float fInit)
{
    float **ppArray = NULL;

#ifndef CONTIG_ARRAY
    if (d1 > 0) {
        ppArray = new float *[d1];

        for (int i = 0; i < d1; i++) {
            ppArray[i] = NULL;
            if (d2 > 0) {
                ppArray[i] = new float[d2];
                for (int j = 0; j < d2; j++)
                    ppArray[i][j] = fInit;
            }
        }
    }
#else
    {
        unsigned n_rows = d2;
        unsigned n_cols = d1;

        unsigned c;

        unsigned n_elements = n_rows * n_cols;

        float *p_base = (float *)malloc(n_elements * sizeof(float) + n_cols * sizeof(float *));

        float **pp_ret = (float **)p_base;

        float *p_data = (float *)(pp_ret + n_cols);

        for (c = 0; c < n_cols; ++c) {
            pp_ret[c] = p_data + c * n_rows;
        }

        ppArray = pp_ret;

        for (int i = 0; i < d1; i++) {
            for (int j = 0; j < d2; j++) {
                ppArray[i][j] = fInit;
            }
        }
    }
#endif

    return ppArray;
}

float **DeleteFloat2D(float **ppArray, int d1, int d2)
{

#ifndef CONTIG_ARRAY
    if (ppArray) {
        for (int i = 0; i < d1; i++) {
            if (ppArray[i])
                delete[] ppArray[i];
        }
        delete[] ppArray;
    }
#else
    free(ppArray);
#endif

    return NULL;
}

float ***NewFloat3D(int d1, int d2, int d3, float fInit)
{

    float ***pppArray = new float **[d1];

#ifndef CONTIG_ARRAY

    for (int i = 0; i < d1; i++) {
        pppArray[i] = new float *[d2];
        for (int j = 0; j < d2; j++) {
            pppArray[i][j] = new float[d3];
            for (int k = 0; k < d3; k++)
                pppArray[i][j][k] = fInit;
        }
    }

#else

    unsigned n_rows = d3;
    unsigned n_cols = d2;
    unsigned n_layers = d1;

    unsigned l, c;

    unsigned n_elements = n_rows * n_cols * n_layers;

    float *p_base = (float *)malloc(n_elements * sizeof(float) + (n_cols * n_layers) * sizeof(float *) + n_layers * sizeof(float **));

    float ***ppp_ret = (float ***)p_base;

    float **pp_col_ptrs = (float **)(ppp_ret + n_layers);

    float *p_data = (float *)(pp_col_ptrs + n_cols * n_layers);

    for (l = 0; l < n_layers; ++l) {
        ppp_ret[l] = pp_col_ptrs + l * n_cols;
        for (c = 0; c < n_cols; ++c) {
            ppp_ret[l][c] = p_data + (l * n_cols * n_rows) + (c * n_rows);
        }
    }

    pppArray = ppp_ret;

    for (int i = 0; i < d1; i++) {
        for (int j = 0; j < d2; j++) {
            for (int k = 0; k < d3; k++)
                pppArray[i][j][k] = fInit;
        }
    }
#endif

    return pppArray;
}

float ***DeleteFloat3D(float ***pppArray, int d1, int d2, int d3)
{

#ifndef CONTIG_ARRAY

    if (pppArray) {
        for (int i = 0; i < d1; i++) {
            if (pppArray[i]) {
                for (int j = 0; j < d2; j++) {
                    if (pppArray[i][j])
                        delete[] pppArray[i][j];
                }
                delete[] pppArray[i];
            }
        }
        delete[] pppArray;
    }

#else
    free(pppArray);
#endif

    return NULL;
}

double *NewDouble1D(int d1)
{

    double *pArray = new double[d1];
    memset(pArray, 0x00, d1 * sizeof(double));

    return pArray;
}

double *DeleteDouble1D(double *pArray, int d1)
{

    if (pArray)
        delete[] pArray;

    return NULL;
}

int *NewInt1D(int d1)
{

    int *pArray = new int[d1];
    memset(pArray, 0x00, d1 * sizeof(int));

    return pArray;
}

int *DeleteInt1D(int *pArray, int d1)
{

    if (pArray)
        delete[] pArray;

    return NULL;
}

int *DeleteInt1D(int *pArray)
{

    if (pArray)
        delete[] pArray;

    return NULL;
}

int **NewInt2D(int d1, int d2)
{
    int **ppArray;

#ifndef CONTIG_ARRAY

    ppArray = new int *[d1];

    for (int i = 0; i < d1; i++) {
        ppArray[i] = new int[d2];
        ZeroMemory(ppArray[i], d2 * sizeof(int));
    }

#else
    {
        unsigned n_rows = d2;
        unsigned n_cols = d1;

        unsigned c;

        unsigned n_elements = n_rows * n_cols;

        int *p_base = (int *)malloc(n_elements * sizeof(int) + n_cols * sizeof(int *));

        int **pp_ret = (int **)p_base;

        int *p_data = (int *)(pp_ret + n_cols);

        for (c = 0; c < n_cols; ++c) {
            pp_ret[c] = p_data + c * n_rows;
        }

        ppArray = pp_ret;

        for (int i = 0; i < d1; i++) {
            for (int j = 0; j < d2; j++) {
                ppArray[i][j] = 0;
            }
        }
    }
#endif

    return ppArray;
}

int **DeleteInt2D(int **ppArray, int d1, int d2)
{

#ifndef CONTIG_ARRAY
    if (ppArray) {
        for (int i = 0; i < d1; i++) {
            if (ppArray[i])
                delete[] ppArray[i];
        }
        delete[] ppArray;
    }
#else
    free(ppArray);
#endif

    return NULL;
}

short *NewShort1D(int d1)
{

    short *pArray = new short[d1];
    memset(pArray, 0x00, d1 * sizeof(short));

    return pArray;
}

short *DeleteShort1D(short *pArray, int d1)
{

    if (pArray)
        delete[] pArray;

    return NULL;
}

char **DeleteChar2D(char **ppArray, int d1, int d2)
{

    if (ppArray) {
        for (int i = 0; i < d1; i++) {
            if (ppArray[i])
                delete[] ppArray[i];
        }
        delete[] ppArray;
    }

    return NULL;
}
