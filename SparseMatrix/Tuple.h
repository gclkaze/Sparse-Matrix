#ifndef TUPLE_H
#define TUPLE_H
#include <vector>
#include "ITupleMatrix.h"
class Tuple {
private:
	std::vector<int> m_Indices;
	std::vector<ITupleMatrix*> m_Owners;

	public:
	Tuple() {}
	Tuple(const std::vector<int>& indices)
		: m_Indices(indices) {
	}
	const std::vector<int>& getIndices() const {
		return m_Indices;
	}
	bool addOwner( ITupleMatrix* owner) {
		if(m_Owners.end() != std::find(m_Owners.begin(), m_Owners.end(), owner)) {
			return false;
		}
		m_Owners.emplace_back(owner);
		return true;
	}

	std::vector<ITupleMatrix*> getOwners() const {
		return m_Owners;
	}

	virtual ~Tuple() {}
};
#endif