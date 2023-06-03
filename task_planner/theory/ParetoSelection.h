#pragma once

#include "graph_search/MultiObjectiveSearchProblem.h"

namespace TP {

template <class NODE_T, class EDGE_STORAGE_T, class COST_VECTOR_T>
class ParetoSelection {
    public:
        using ParetoFront = GraphSearch::MultiObjectiveSearchResult<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>;
    public:
        static typename std::list<PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::iterator uniformRandom(const ParetoFront& pf);
        static typename std::list<PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::iterator TOPSIS(const ParetoFront& pf);
        static typename std::list<PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::iterator scalarWeights(const ParetoFront& pf);
};



    //struct MultiObjectiveSearchResult {
    //    public:
    //        MultiObjectiveSearchResult(bool retain_search_graph = true, bool retain_non_dominated_cost_map = true)
    //            : search_graph(std::make_shared<SearchGraph<SearchGraphEdge<const COST_VECTOR_T*, EDGE_STORAGE_T>, NODE_T>>())
    //            , non_dominated_cost_map(std::make_shared<NonDominatedCostMap<COST_VECTOR_T>>())
    //            , m_retain_search_graph(retain_search_graph)
    //            , m_retain_non_dominated_cost_map(retain_non_dominated_cost_map)
    //        {
    //        }

    //        bool success = false;
    //        std::list<PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>> solution_set;
    //        std::shared_ptr<SearchGraph<SearchGraphEdge<const COST_VECTOR_T*, EDGE_STORAGE_T>, NODE_T>> search_graph;
    //        std::shared_ptr<NonDominatedCostMap<COST_VECTOR_T>> non_dominated_cost_map;

    //        void package() { // Free the memory of the search tree and min cost map if the user desires
    //            if (!m_retain_search_graph) search_graph.reset();
    //            if (!m_retain_non_dominated_cost_map) non_dominated_cost_map.reset();

    //            auto lexComparison = [](const PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>& lhs, const PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>& rhs) -> bool {
    //                return lhs.path_cost.lexicographicLess(rhs.path_cost);
    //            };
    //            solution_set.sort(lexComparison);
    //        }
    //    private:
    //        bool m_retain_search_graph, m_retain_non_dominated_cost_map;
    //};
}