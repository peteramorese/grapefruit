#pragma once

#include "Grapefruit.h"
#include "TrueBehavior.h"
#include "SearchProblem.h"
#include "TrajectoryDistributionEstimator.h"

namespace PRL {

template <class SYMBOLIC_GRAPH_T, uint64_t N>
class Regret {
    public:
        using SearchResult = GF::GraphSearch::MultiObjectiveSearchResult<
            typename SearchProblem<N, TrueBehavior<SYMBOLIC_GRAPH_T, N>>::node_t, 
            typename SearchProblem<N, TrueBehavior<SYMBOLIC_GRAPH_T, N>>::edge_t, 
            typename SearchProblem<N, TrueBehavior<SYMBOLIC_GRAPH_T, N>>::cost_t>;

        using CostVector = SearchProblem<N, TrueBehavior<SYMBOLIC_GRAPH_T, N>>::cost_t;

        using Plan = GF::Planner::Plan<SearchProblem<N, TrueBehavior<SYMBOLIC_GRAPH_T, N>>>;

    public:
        Regret(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product, const std::shared_ptr<TrueBehavior<SYMBOLIC_GRAPH_T, N>>& true_behavior)
            : m_product(product)
            , m_true_behavior(true_behavior)
        {}

        float getRegret(SYMBOLIC_GRAPH_T::node_t starting_node, const CostVector& sample) {
            return queryCache(starting_node)->second.pf.regret(sample);
        }

        float getBias(SYMBOLIC_GRAPH_T::node_t starting_node, const std::vector<GF::Stats::Distributions::FixedMultivariateNormal<N>>& candidate_plan_distributions) {
            const SearchResult& result = queryCache(starting_node)->second;

            // Calculate the bias
            float bias = 0.0f;

            ASSERT(result.solution_set.size() == result.pf.size(), "Corrupted search result (pf size does not match solution set size)");
            auto pf_it = result.pf.begin();
            for (const auto& path_soln : result.solution_set) {
                // Calculate the true Pareto point distribution

                Plan plan(path_soln, *pf_it++, m_product, true);

                auto state_it = plan.begin();

                GF::Stats::Distributions::FixedMultivariateNormal<N> true_plan_dist;
                for (auto action_it = plan.action_sequence.begin(); action_it != plan.action_sequence.end(); ++action_it) {
                    const GF::Stats::Distributions::FixedMultivariateNormal<N>& true_state_action_dist = m_true_behavior->getElement(state_it.tsNode(), *action_it).dist();
                    true_plan_dist.convolveWith(true_state_action_dist);
                    ++state_it;
                }

                float min_kld = -1.0;
                for (const auto& candidate_dist : candidate_plan_distributions) {
                    float kld = GF::Stats::KLD(true_plan_dist, candidate_dist);
                    if (kld < min_kld || min_kld < 0.0f) {
                        min_kld = kld;
                    }
                }

                bias += min_kld;
            }

            return bias / static_cast<float>(result.solution_set.size());
        }

    private:
        std::map<typename SYMBOLIC_GRAPH_T::node_t, SearchResult>::iterator queryCache(SYMBOLIC_GRAPH_T::node_t starting_node) {
            auto it = m_search_result_cache.find(starting_node);
            if (it == m_search_result_cache.end()) {
                SearchProblem<N, TrueBehavior<SYMBOLIC_GRAPH_T, N>> problem(m_product, starting_node, 1, m_true_behavior);

                SearchResult result = [&] {
                    if constexpr (N == 2)
                        // Use BOA
                        return GF::GraphSearch::BOAStar<CostVector, decltype(problem)>::search(problem);
                    else
                        // Use NAMOA
                        return GF::GraphSearch::NAMOAStar<CostVector, decltype(problem)>::search(problem);
                }();

                it = m_search_result_cache.insert(std::make_pair(starting_node, result)).first;
            }
            return it;
        }

    private:
        std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;
        std::shared_ptr<TrueBehavior<SYMBOLIC_GRAPH_T, N>> m_true_behavior;
        std::map<typename SYMBOLIC_GRAPH_T::node_t, SearchResult> m_search_result_cache;
};

}