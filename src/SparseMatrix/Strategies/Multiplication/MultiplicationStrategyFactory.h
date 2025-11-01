#ifndef MULTIPLICATION_STRATEGY_FACTORY_H
#define MULTIPLICATION_STRATEGY_FACTORY_H

#include <assert.h>

#include <memory>

#include "BlindlyThreadedTreeMultiplication.h"
#include "LateComparisonMultiplication.h"
#include "MultiplicationTypes.h"
#include "OffsetTreeMultiplication.h"
#include "RangedTreeThreadedMultiplication.h"
#include "TupleIteratorMultiplication.h"

class MultiplicationStrategyFactory {
   public:
    static std::unique_ptr<IMultiplicationStrategy> createMultiplication(MultiplicationTypes type) {
        switch (type) {
            case RANGED_TREE_THREADED: {
                auto ptr = std::make_unique<RangedTreeThreadedMultiplication>();
                return std::move(ptr);
            }
            case OFFSET_TREE: {
                auto ptr = std::make_unique<OFfsetTreeMultiplication>();
                return std::move(ptr);
            }
            case BLINDLY_THREADED_TREE: {
                auto ptr = std::make_unique<BlindlyThreadedTreeMultiplication>();
                return std::move(ptr);
            }
            case LATE_COMPARISON: {
                auto ptr = std::make_unique<LateComparisonMultiplication>();
                return std::move(ptr);
            }
            case TUPLE_ITERATION: {
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