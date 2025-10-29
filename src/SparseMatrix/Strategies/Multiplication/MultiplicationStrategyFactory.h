#ifndef MULTIPLICATION_STRATEGY_FACTORY_H
#define MULTIPLICATION_STRATEGY_FACTORY_H
//#include "IMultiplicationStrategy.h"
#include "MultiplicationTypes.h"
#include "RangedTreeThreadedMultiplication.h"
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
        default: {
            assert(false);
        }
        }
        assert(false);
        return nullptr;
    }
};
#endif