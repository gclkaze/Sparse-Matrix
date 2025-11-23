#ifndef GPU_SPARSE_MATRIX_H
#define GPU_SPARSE_MATRIX_H
#include "GlobalMatrixService.h"
#include <map>
#include "Tuple.h"
#include "GPUSparseMatrixIterator.h"

extern float* gpuMultiplySparseMatrices(Intersection*);

class GPUSparseMatrix : ITupleMatrix {
private:
	int m_Id;
	std::map<Tuple*, float> m_Data;
    std::map<int, Tuple*> m_TupleMap;
   
public:
	GPUSparseMatrix():m_Id(GlobalMatrixService::getNextMatrixId()) {
		GlobalMatrixService::registerMatrix(this);
	}

	std::map<int, Tuple*>& getTupleMap() {
		return m_TupleMap;
	}

	std::map<Tuple*,float>& getData() {
		return m_Data;
	}
	
	virtual int getId() const override {
		return m_Id;
	}

	virtual int getSize() const override {
		return (int)m_Data.size();
	}

	GPUSparseMatrixIterator iterator() {
		return GPUSparseMatrixIterator(&m_Data,&m_TupleMap,m_Data.size());
	}

	float getValue(const std::vector<int>& tuple) override{
		Tuple* t = GlobalMatrixService::tupleExists(tuple);
		if(t != nullptr) {
			auto it = m_Data.find(t);
			if(it != m_Data.end()) {
				return it->second;
			}
		}
		return 0.0;
	}

	 GPUSparseMatrix operator*(const GPUSparseMatrix& B)  {
		GPUSparseMatrix result;
		auto intersection = GlobalMatrixService::getIntersection(m_Id,B.getId());
		if(intersection == nullptr) {
			//no common tuples
			return result;
		}
		float* results = gpuMultiplySparseMatrices(intersection);
		result.loadMatrix(intersection,results);
		return result;
	}

	void loadMatrix(Intersection* intersection, float* values) {
		auto& commonTuples = intersection->getCommonTuples();
		for(size_t i = 0; i < commonTuples.size(); i++) {
			Tuple* t = commonTuples[i];
			float value = values[i];
			m_Data[t] = value;
			t->addOwner(this);

			int cnt = GlobalMatrixService::getHash(t->getIndices());
			m_TupleMap[cnt] = t;
		}
	}

	bool insert(const std::vector<int>& tuple, float value) override{
		Tuple* insertion = GlobalMatrixService::registerTuple(m_Id, tuple, value);
		m_Data[insertion] = value;

		insertion->addOwner(this);

		int cnt = GlobalMatrixService::getHash(tuple);
		m_TupleMap[cnt] = insertion;
		return true;
	}
	virtual ~GPUSparseMatrix() {}
};
#endif