#ifndef GPU_SPARSE_MATRIX_ITERATOR_H
#define GPU_SPARSE_MATRIX_ITERATOR_H
#include <assert.h>

#include <vector>
#include "SparseMatrix/SparseMatrixTuple.h"

class GPUSparseMatrixIterator {
private:
    std::map<Tuple*, float> *m_Data;
    std::map<int, Tuple*> *m_TupleMap;

    SparseMatrixTuple m_Tuple;
	int m_CurrentPosition = 0;
    int m_Size = 0;

    std::map<int, Tuple*>::iterator m_Iterator;

public:
    GPUSparseMatrixIterator(std::map<Tuple*, float> *m, std::map<int, Tuple*> *p)
        : m_Data(m), m_TupleMap(p) {
    }

    GPUSparseMatrixIterator(std::map<Tuple*, float>* m, std::map<int, Tuple*>* p,int size)
        : m_Data(m), m_TupleMap(p),m_Size(size) {
    }

    GPUSparseMatrixIterator begin() { return GPUSparseMatrixIterator(m_Data,m_TupleMap, m_Size); }

    bool ended() const { return m_Size == 0; }

    GPUSparseMatrixIterator end() { return GPUSparseMatrixIterator(m_Data, m_TupleMap); }

    SparseMatrixTuple& getTuple() { return m_Tuple; }


    SparseMatrixTuple operator*() {
        if (m_Size == 0) {
			assert(m_Iterator == m_TupleMap->end());
            return m_Tuple;
        }

        if (m_CurrentPosition == 0) {
            //we start
            m_Iterator = m_TupleMap->begin();
        }

        auto pos = m_Iterator->first;
        auto tuple = m_Iterator->second;

        auto r = m_Data->at(tuple);

        m_Tuple.tuple = tuple->getIndices();
        m_Tuple.value = r;
        m_CurrentPosition++;
		m_Size--;
        m_Iterator++;
        return m_Tuple;       
    }

    const SparseMatrixTuple* operator->() { return &m_Tuple; }

    friend bool operator==(const GPUSparseMatrixIterator& a, const GPUSparseMatrixIterator& b) {
        return a.m_Size == b.m_Size;
    }

    friend bool operator!=(const GPUSparseMatrixIterator& a, const GPUSparseMatrixIterator& b) {
        return a.m_Size != b.m_Size;
    }

    GPUSparseMatrixIterator& operator++() { return *this; }

    // Postfix increment
    GPUSparseMatrixIterator operator++(int) {
        GPUSparseMatrixIterator tmp = *this;
        ++(*this);
        return tmp;
    }
};
#endif
