#ifndef SPARSE_MATRIX_TUPLE_H
#define SPARSE_MATRIX_TUPLE_H
#include <vector>
struct SparseMatrixTuple
{
    std::vector<int> tuple;
    double value;

    bool operator<(const SparseMatrixTuple &other) const
    {
        int size = (int)tuple.size();
        for (int i = 0; i < size; i++)
        {
            if (tuple[i] < other.tuple[i])
            {
                return true;
            }
            else if (tuple[i] > other.tuple[i])
            {
                return false;
            }
        }
        return false;
    }

    bool operator!=(const SparseMatrixTuple &other) const
    {
        int size = (int)tuple.size();
        for (int i = 0; i < size; i++)
        {
            if (tuple[i] == other.tuple[i])
            {
                return false;
            }
        }
        return true;
    }

    bool operator==(const SparseMatrixTuple &other) const
    {
        int size = (int)tuple.size();
        for (int i = 0; i < size; i++)
        {
            if (tuple[i] != other.tuple[i])
            {
                return false;
            }
        }
        return true;
    }
};

#endif