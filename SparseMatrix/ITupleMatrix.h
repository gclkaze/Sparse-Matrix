#ifndef ITUPLE_MATRIX_H
#define ITUPLE_MATRIX_H
#include <vector>
class ITupleMatrix {
public:
	virtual bool insert(const std::vector<int>& tuple, float value) = 0;
	virtual int getId() const = 0;
	virtual float getValue(const std::vector<int>& tuple) = 0;
	virtual int getSize() const = 0;
	//virtual ITupleMatrix* operator*(const ITupleMatrix* B) = 0;
	virtual ~ITupleMatrix() {}
};
#endif