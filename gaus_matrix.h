#ifndef GAUS_MATRIX
#define GAUS_MATRIX

#include <iostream>

class GausMatrix
{
private:
    int32_t variance;
    long double mathEpx;
    int32_t size;
    long double* matrix;
public : 
    GausMatrix(int32_t size);
    virtual ~GausMatrix();
    void calculateMatrix();
    void print() const;
    void normalize();
    long double getMatrixSumm();
    long double operator[](int32_t it) const;
    void div(int32_t num);
    const long double* getMatrix() const;
};

#endif