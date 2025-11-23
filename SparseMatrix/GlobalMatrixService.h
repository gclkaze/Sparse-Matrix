#ifndef GLOBAL_MATRIX_SERVICE_H
#define GLOBAL_MATRIX_SERVICE_H
#include <vector>
#include <map>
#include "Tuple.h"
#include <memory>
#include "Intersection.h"
#include <assert.h>
class GlobalMatrixService {
private:
    static std::map<int, std::unique_ptr<Tuple>> m_Tuples;
	static std::map<int64_t, std::unique_ptr<Intersection>> m_Intersections;
	static std::map<int, ITupleMatrix*> m_Matrices;

public:
       // Step 1: map signed integer to unique non-negative
       static inline uint64_t map_int(int x) {
           return (x >= 0) ? uint64_t(x) * 2 : uint64_t(-x * 2 - 1);
       }

       // Step 2 + 3: unordered unique pairing
       uint64_t getUniqueUnorderedKey(int a, int b) {
           uint64_t A = map_int(a);
           uint64_t B = map_int(b);

           if (A > B) std::swap(A, B); // enforce symmetry

           // Szudzik's elegant pairing (ordered)
           return A * A + B;
       }

/*       static bool tupleExists(const Tuple& t) {
           int hash = getHash(t.getIndices());
           return m_Tuples.find(hash) != m_Tuples.end();
	   }*/

       static void registerMatrix(ITupleMatrix* matrix) {
           assert(matrix);
           assert(m_Matrices.find(matrix->getId()) == m_Matrices.end());
		   m_Matrices[matrix->getId()] = matrix;
       }

       static Tuple* tupleExists(const std::vector<int>& t) {
           int hash = getHash(t);
		   auto it = GlobalMatrixService::m_Tuples.find(hash);
           if (it != GlobalMatrixService::m_Tuples.end()) {
               return GlobalMatrixService::m_Tuples[hash].get();
		   }
           return nullptr;
       }

       static Intersection* getIntersection(int id, std::vector< ITupleMatrix*> owners) {
           for (const auto& owner : owners) {
               auto otherId = owner->getId();
               auto intersection = getIntersection(id, otherId);
               if (intersection != nullptr) {
                   return intersection;
               }
           }
           return nullptr;
       }

       static Intersection* getIntersection(int idA, int idB) {
           uint64_t key = GlobalMatrixService().getUniqueUnorderedKey(idA, idB);
           auto it = GlobalMatrixService::m_Intersections.find(key);
           if (it != GlobalMatrixService::m_Intersections.end()) {
               return GlobalMatrixService::m_Intersections[key].get(); // Intersection found
           }
           return nullptr;
	   }

       /*need to do something with the value copy*/
       static Tuple* registerTuple(int id, const std::vector<int>& t,float value) {
           int hash = getHash(t);
           auto it = GlobalMatrixService::m_Tuples.find(hash);
		   //if tuple already exists, need to find the Matrix where it was registered
           if (it != GlobalMatrixService::m_Tuples.end()) {
			   //the tuple has been registered before
               auto owners = it->second.get()->getOwners();
               auto tuple = it->second.get();
			   //is there an intersection between owners and id?
               auto intersection = getIntersection(id, owners);
			   //if there is no intersection, create one between id and the owners
               if (intersection == nullptr) {
                   tuple->addOwner(m_Matrices[id]);
                   for(ITupleMatrix* owner : owners) {
                       uint64_t key = GlobalMatrixService().getUniqueUnorderedKey(id, owner->getId());
                       m_Intersections[key] = std::make_unique<Intersection>();
                       m_Intersections[key]->setMatrices(owner, m_Matrices[id]);
					   m_Intersections[key]->putCommonTuple(tuple,  owner->getValue(t), value);
				   }
                   return GlobalMatrixService::m_Tuples[hash].get();
               }
               else {
                   auto other = intersection->getMatrixOtherThan(id);
				   auto first = m_Matrices[id];
                   intersection->putCommonTuple(tuple,  other->getValue(t), value);
               }
               return GlobalMatrixService::m_Tuples[hash].get();
           }
           GlobalMatrixService::m_Tuples[hash] = std::make_unique<Tuple>(t);
		   assert(m_Matrices.find(id) != m_Matrices.end());
		   GlobalMatrixService::m_Tuples[hash]->addOwner(m_Matrices[id]);
           return GlobalMatrixService::m_Tuples[hash].get();
       }

	   static int getNextMatrixId() {
		   static int currentId = 0;
		   return currentId++;
	   }

	   static int getHash(const std::vector<int>& t) {
           const int P = 53;                // prime base
           const int M = 1'000'000'007;     // large prime modulus (fits in signed 32-bit)

           int h = 0;
           for (int x : t) {
               // Use 64-bit temporarily to avoid overflow when multiplying
               int64_t tmp = (int64_t)h * P + x;
               h = (int)(tmp % M);
               if (h < 0) h += M;           // ensure non-negative
           }
           return h;  // stays within signed int range
       }
	virtual ~GlobalMatrixService() {}
};
#endif