#include "gaus_matrix.h"
#include <math.h>
#include <iomanip>

GausMatrix::GausMatrix(const int32_t size)
    :variance(size),mathEpx(static_cast<double>((size-1)/2)), size(size)
{
    matrix = new double[size*size];
    calculateMatrix();
}

GausMatrix::~GausMatrix()
{
    delete[] matrix;
}

const double* GausMatrix::getMatrix() const
{
    return matrix;
}

void GausMatrix::calculateMatrix()
{
    double A = 1/(sqrt(2 * M_PI * variance));
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            matrix[i * size + j] = A * exp(-(pow(i - mathEpx,2)/(2*variance)+ pow(j-mathEpx,2)/(2*variance)));
        }
    }
    normalize();
}

void GausMatrix::normalize()
{
    double summ = getMatrixSumm();
    div(summ);
}

void GausMatrix::div(const int32_t num)
{
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            matrix[i * size + j] /= num ;
        }
    }
}


double GausMatrix::getMatrixSumm()
{
    double summ = 0;
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            summ += matrix[i * size + j];
        }
    }
    return summ;
}

void GausMatrix::print() const
{
    double summ = 0;
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            summ += matrix[i * size + j];
            std::cout << std::fixed << std::setw( 11 ) << std::setprecision( 6 )  << matrix[i * size + j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::fixed << std::setw( 11 ) << std::setprecision( 6 )  << summ << std::endl;   
}

