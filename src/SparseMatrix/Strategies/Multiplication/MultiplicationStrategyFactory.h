#ifndef MULTIPLICATION_STRATEGY_FACTORY_H
#define MULTIPLICATION_STRATEGY_FACTORY_H
//#include "IMultiplicationStrategy.h"
#include "MultiplicationTypes.h"
#include "RangedTreeThreadedMultiplication.h"
#include "TupleIteratorMultiplication.h"
#include <assert.h>
#include <memory>

class MultiplicationStrategyFactory {
  public:
    static std::unique_ptr<IMultiplicationStrategy>
    createMultiplication(MultiplicationTypes type) {
        switch (type) {
        case RANGED_TREE_THREADED: {
            auto ptr = std::make_unique<RangedTreeThreadedMultiplication>();            
            return std::move(ptr);
        }
        case BLINDLY_THREADED: {
            assert(false);
        }
        case SIMPLE_ITERATION: {
        }
        case TUPLE_ITERATION:{
            auto ptr = std::make_unique<TupleIteratorMultiplication>();            
            return std::move(ptr);

        }
        default: {
            assert(false);
        }
        }
        assert(false);
        return nullptr;
    }
};
#endif