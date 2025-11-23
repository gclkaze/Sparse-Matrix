#ifndef TUPLE_ITERATOR_MULTIPLICATION_H
#define TUPLE_ITERATOR_MULTIPLICATION_H

#include <thread>
#include <vector>

#include "../../CommonOffset.h"
#include "../../FlatNode.h"
#include "../../SparseMatrixIterator.h"
#include "IMultiplicationStrategy.h"


class TupleIteratorMultiplication : public IMultiplicationStrategy {
   public:
    ~TupleIteratorMultiplication() {}

    ISparseMatrix* multiply(ISparseMatrix* A, ISparseMatrix* B, ISparseMatrix* C) {
        SparseMatrixIterator it = A->iterator();
        SparseMatrixIterator otherIt = B->iterator();

        SparseMatrixTuple myTuple;
        SparseMatrixTuple otherTuple;

        bool myEnd = false;
        bool otherEnd = false;
        bool inserted = false;

        myTuple = *it;
        otherTuple = *otherIt;

        while (true) {
            if (myTuple == otherTuple) {
                m_Multi++;
                C->insert(myTuple.tuple, myTuple.value * otherTuple.value);
                inserted = true;
            }

            myEnd = it.ended();
            otherEnd = otherIt.ended();

            if (!myEnd || !otherEnd) {
                if (!inserted) {
                    if (!myEnd && !otherEnd) {
                        if (myTuple < otherTuple) {
                            myTuple = *it;
                        } else {
                            otherTuple = *otherIt;
                        }
                    } else if (myEnd && !otherEnd) {
                        otherTuple = *otherIt;
                    } else if (!myEnd && otherEnd) {
                        myTuple = *it;
                    }
                } else {
                    myTuple = *it;
                    otherTuple = *otherIt;
                    inserted = false;
                }
            } else {
                break;
            }
        }
        return C;
    }
};
#endif