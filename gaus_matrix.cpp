#include "gaus_matrix.h"
#include <math.h>
#include <iomanip>

GausMatrix::GausMatrix(int32_t size)
    :variance(size),mathEpx(static_cast<long double>((size-1)/2)), size(size)
{
    matrix = new long double[size*size];
    calculateMatrix();
}

GausMatrix::~GausMatrix()
{
    delete[] matrix;
}

const long double* GausMatrix::getMatrix() const
{
    return matrix;
}

void GausMatrix::calculateMatrix()
{
    int32_t accuracy = 10;
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            int64_t tmp = static_cast<int64_t>(exp(-(pow(i - mathEpx,2)/(2*variance)+ pow(j-mathEpx,2)/(2*variance))) * accuracy);
            matrix[i * size + j] = static_cast<long double>(tmp)/accuracy;
        }
    }
    normalize();
}

void GausMatrix::normalize()
{
    long double summ = getMatrixSumm();
    div(summ);
}

void GausMatrix::div(int32_t num)
{
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            matrix[i * size + j] /= num ;
        }
    }
}

long double GausMatrix::getMatrixSumm()
{
    long double summ = 0;
    for (size_t i = 0; i < size*size; i++)
    {
        summ += matrix[i];
    }

    return summ;
}

void GausMatrix::print() const
{
    long double summ = 0;
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

long double GausMatrix::operator[](int32_t it) const
{
    if(it < size*size)
        return matrix[it];
}
