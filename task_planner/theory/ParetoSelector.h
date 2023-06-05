#pragma once

#include "graph_search/ParetoFront.h"

namespace TP {

template <class COST_VECTOR_T>
class ParetoSelector {
    public:
        static typename std::size_t uniformRandom(const ParetoFront<COST_VECTOR_T>& pf);
        static typename std::size_t TOPSIS(const ParetoFront<COST_VECTOR_T>& pf);
        static typename std::size_t scalarWeights(const ParetoFront<COST_VECTOR_T>& pf, Containers::FixedArray<COST_VECTOR_T::size(), float> weights);
};

}

#include "ParetoSelector_impl.hpp"
