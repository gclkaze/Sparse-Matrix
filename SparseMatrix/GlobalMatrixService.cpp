#include "GlobalMatrixService.h"

std::map<int, std::unique_ptr<Tuple>> GlobalMatrixService::m_Tuples;
std::map<int64_t, std::unique_ptr<Intersection>> GlobalMatrixService::m_Intersections;
std::map<int, ITupleMatrix*> GlobalMatrixService::m_Matrices;