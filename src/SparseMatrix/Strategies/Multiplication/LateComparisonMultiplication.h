#ifndef LATE_COMPARISON_MULTIPLICATION_H
#define LATE_COMPARISON_MULTIPLICATION_H
#include "IMultiplicationStrategy.h"

class LateComparisonMultiplication : public IMultiplicationStrategy {
   private:
    size_t m_RangeElementsPerThread = 20;

   public:
    ~LateComparisonMultiplication() {}

    ISparseMatrix* multiply(ISparseMatrix* A, ISparseMatrix* B, ISparseMatrix* C) {
        m_Multi = 0;
        SparseMatrixIterator it = A->iterator();
        SparseMatrixIterator otherIt = B->iterator();

        SparseMatrixTuple myTuple = *it;
        SparseMatrixTuple otherTuple = *otherIt;

        bool myEnd = it.ended();
        bool otherEnd = otherIt.ended();

        while (true) {
            if (myEnd && otherEnd) {
                break;
            } else if (!myEnd && otherEnd) {
                myTuple = *it;
            } else if (myEnd && !otherEnd) {
                otherTuple = *otherIt;
            }

            if (myTuple == otherTuple) {
                C->insert(myTuple.tuple, myTuple.value * otherTuple.value);
                myTuple = *it;
                otherTuple = *otherIt;
            } else if (myTuple < otherTuple) {
                myTuple = *it;
            } else {
                otherTuple = *otherIt;
            }

            myEnd = it.ended();
            otherEnd = otherIt.ended();
        }

        return C;
    }
};

#endif