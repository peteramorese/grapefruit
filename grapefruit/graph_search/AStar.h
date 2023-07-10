#pragma once

#include "graph_search/SearchProblem.h"


namespace GF {
namespace GraphSearch {


    template <class NODE_T, class EDGE_T, class COST_T, class SEARCH_PROBLEM_T, class HEURISTIC_T = ZeroHeuristic<NODE_T, COST_T>, typename EDGE_STORAGE_T = EDGE_T>
    class AStar {
        public:
            static SingleObjectiveSearchResult<NODE_T, EDGE_STORAGE_T, COST_T> search(const SEARCH_PROBLEM_T& problem);
        private:
            static void extractPath(const NODE_T& goal_node, SingleObjectiveSearchResult<NODE_T, EDGE_STORAGE_T, COST_T>& result);
    };
}
}

#include "AStar_impl.hpp"