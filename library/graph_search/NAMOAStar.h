#pragma once

#include "tools/Logging.h"
#include "graph_search/SearchProblem.h"
#include "graph_search/MultiObjectiveSearchProblem.h"
#include "core/Graph.h"


namespace TP {


namespace GraphSearch {

    using NormType = double;

    template <class COST_VECTOR_T, class COMBINED_COST_T = NormType>
    struct ObjectiveL1Norm {
        typedef COMBINED_COST_T combined_cost_t;
        static COMBINED_COST_T norm(const COST_VECTOR_T& cv) {
            //Containers::FixedArray<COST_VECTOR_T::size(), COMBINED_COST_T> input;
            COMBINED_COST_T cumulative_norm;
            auto extract_combined_cost = [&cumulative_norm]<typename T>(const T& element) {
                cumulative_norm += static_cast<COMBINED_COST_T>(element);
                return true;
            };
            return cumulative_norm;
        }
    };

    template <class COST_VECTOR_T, class SEARCH_PROBLEM_T, class HEURISTIC_T = ZeroHeuristic<Node, COST_VECTOR_T>, typename EDGE_STORAGE_T = typename SEARCH_PROBLEM_T::edge_t, class OBJECTIVE_NORM_T = ObjectiveL1Norm<COST_VECTOR_T>>
    class NAMOAStar {
        public:
            using GraphNode = SEARCH_PROBLEM_T::node_t;
            typedef SEARCH_PROBLEM_T::edge_t edge_t;
            typedef SEARCH_PROBLEM_T::cost_t cost_t;

        private:
            using CostMapItem = typename NonDominatedCostMap<COST_VECTOR_T>::Item;
            struct GSetEntry {
                GSetEntry(GSetEntry&&) = default;
                GSetEntry(GraphNode node_, const CostMapItem* g_score_) 
                    : node(node_), g_score(g_score_) {}
                bool operator==(const GSetEntry& other) const {return node == other.node && g_score == other.g_score;}
                const CostMapItem* g_score;
                GraphNode node;
            };

        public:
            static MultiObjectiveSearchResult<GraphNode, EDGE_STORAGE_T, COST_VECTOR_T> search(const SEARCH_PROBLEM_T& problem);

        private:
            static void extractPaths(std::vector<GSetEntry> goal_set, MultiObjectiveSearchResult<GraphNode, EDGE_STORAGE_T, COST_VECTOR_T>& result, const SEARCH_PROBLEM_T& problem);
    };

}
}

#include "NAMOAStar_impl.hpp"