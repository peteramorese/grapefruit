#pragma once

#include "graph_search/MultiObjectiveSearchProblem.h"

namespace TP {

template <class NODE_T, class EDGE_STORAGE_T, class COST_VECTOR_T>
class ParetoSelector {
    public:
        using ParetoFront = GraphSearch::MultiObjectiveSearchResult<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>;
    public:
        static typename std::list<GraphSearch::PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::const_iterator uniformRandom(const ParetoFront& pf);
        static typename std::list<GraphSearch::PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::const_iterator TOPSIS(const ParetoFront& pf);
        static typename std::list<GraphSearch::PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::const_iterator scalarWeights(const ParetoFront& pf, const Containers::FixedArray<COST_VECTOR_T::size(), float>& weights);
};

}

#include "ParetoSelector_impl.hpp"
