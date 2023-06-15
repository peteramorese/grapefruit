#pragma once

#include "TaskPlanner.h"
#include "TrueBehavior.h"
#include "SearchProblem.h"

namespace PRL {

template <class SYMBOLIC_GRAPH_T, uint64_t N>
class Regret {
    public:
        using SearchResult = TP::GraphSearch::MultiObjectiveSearchResult<
            typename SearchProblem<N, TrueBehavior<SYMBOLIC_GRAPH_T, N>>::node_t, 
            typename SearchProblem<N, TrueBehavior<SYMBOLIC_GRAPH_T, N>>::edge_t, 
            typename SearchProblem<N, TrueBehavior<SYMBOLIC_GRAPH_T, N>>::cost_t>;

        using CostVector = SearchProblem<N, TrueBehavior<SYMBOLIC_GRAPH_T, N>>::cost_t;

    public:
        Regret(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product, const std::shared_ptr<TrueBehavior<SYMBOLIC_GRAPH_T, N>>& true_behavior)
            : m_product(product)
            , m_true_behavior(true_behavior)
        {}

        float getRegret(SYMBOLIC_GRAPH_T::node_t starting_node, const CostVector& sample) {
            auto it = m_pareto_front_cache.find(starting_node);
            if (it == m_pareto_front_cache.end()) {
                SearchProblem<N, TrueBehavior<SYMBOLIC_GRAPH_T, N>> problem(m_product, starting_node, 1, m_true_behavior);

                SearchResult result = [&] {
                    if constexpr (N == 2)
                        // Use BOA
                        return TP::GraphSearch::BOAStar<CostVector, decltype(problem)>::search(problem);
                    else
                        // Use NAMOA
                        return TP::GraphSearch::NAMOAStar<CostVector, decltype(problem)>::search(problem);
                }();

                it = m_pareto_front_cache.insert(std::make_pair(starting_node, result.pf)).first;
            }
            return it->second.regret(sample);
        }

    private:
        std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;
        std::shared_ptr<TrueBehavior<SYMBOLIC_GRAPH_T, N>> m_true_behavior;
        std::map<typename SYMBOLIC_GRAPH_T::node_t, TP::ParetoFront<CostVector>> m_pareto_front_cache;
};

}