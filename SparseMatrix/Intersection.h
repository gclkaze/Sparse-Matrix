#ifndef INTERSECTION_H
#define INTERSECTION_H
#include "ITupleMatrix.h"
#include "Tuple.h"

class Intersection {

private:
	ITupleMatrix* m_MatrixA = nullptr;
	ITupleMatrix* m_MatrixB = nullptr;

	std::vector<Tuple*> m_CommonTuples;
	std::vector<float> m_ValuesA;
	std::vector<float> m_ValuesB;

public:
	Intersection(ITupleMatrix* matrixA, ITupleMatrix* matrixB)
		: m_MatrixA(matrixA), m_MatrixB(matrixB) {
	}

	Intersection(){}
	
	std::vector<Tuple*>& getCommonTuples() {
		return m_CommonTuples;
	}

	std::vector<float>& getValuesA() {
		return m_ValuesA;
	}

	std::vector<float>& getValuesB() {
		return m_ValuesB;
	}

	void putCommonTuple(Tuple* tuple, float valueA, float valueB) {
		m_CommonTuples.emplace_back(tuple);
		m_ValuesA.emplace_back(valueA);
		m_ValuesB.emplace_back(valueB);
	}

	ITupleMatrix* getMatrixA() const {
		return m_MatrixA;
	}

	ITupleMatrix* getMatrixB() const {
		return m_MatrixB;
	}
	void setMatrices(ITupleMatrix* matrixA, ITupleMatrix* matrixB) {
		m_MatrixA = matrixA;
		m_MatrixB = matrixB;
	}

	ITupleMatrix* getMatrixOtherThan(int id) {
		if(m_MatrixA->getId() == id) {
			return m_MatrixB;
		}
		return m_MatrixA;
	}

	virtual ~Intersection() {}
};
#endif
