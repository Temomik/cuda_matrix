#ifndef GAUS_MATRIX
#define GAUS_MATRIX

#include <iostream>

class GausMatrix
{
private:
    int32_t variance;
    double mathEpx;
    int32_t size;
    double* matrix;
public : 
    GausMatrix(const int32_t size);
    virtual ~GausMatrix();
    void calculateMatrix();
    void print() const;
    void normalize();
    double getMatrixSumm();
    void div(const int32_t num);
    const double* getMatrix() const;
};

#endif