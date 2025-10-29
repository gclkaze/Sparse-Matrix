#ifndef IMATRIX_H
#define IMATRIX_H
#include <vector>

class IMatrix{
    public:
    virtual bool insert(const std::vector<int>& tuple, double value) = 0;
    virtual bool insert(const int *tupleContainer, int tupleSize, double value) = 0;
    virtual bool erase(const std::vector<int>& tuple) = 0;
    virtual void clear() = 0;
    virtual void reset() = 0;
    virtual const int size() = 0;
    virtual double getValue(const std::vector<int> &tuple) = 0;

    virtual ~IMatrix(){}
};
#endif