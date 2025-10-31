#ifndef ISPARSE_MATRIX_H
#define ISPARSE_MATRIX_H
//#include "../IMatrix.h"
#include <vector>
#include "FlatNode.h"
#include "SparseMatrixIterator.h"

class ISparseMatrix /*: virtual public IMatrix*/ {
  public:
  virtual SparseMatrixIterator iterator() = 0;
    virtual std::vector<FlatNode>& getNodes() = 0;
    virtual std::vector<FlatChildEntry>& getFlatChildren() = 0;
    virtual const int size() const = 0;
    virtual ~ISparseMatrix() {}

    virtual bool insert(const std::vector<int> &tuple, double value) = 0;
    virtual bool insert(const int *tupleContainer, int tupleSize,
                        double value) = 0;
    virtual bool erase(const std::vector<int> &tuple) = 0;
    virtual void clear() = 0;
    virtual void reset() = 0;
    virtual double getValue(const std::vector<int> &tuple) = 0;

  protected:
    int m_Size = 0;
};
#endif