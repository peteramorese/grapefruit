#pragma once

#include "Grapefruit.h"
#include "TrueBehavior.h"
#include "SearchProblem.h"
#include "TrajectoryDistributionEstimator.h"

namespace PRL {

struct Biases {
    float coverage = 0.0f;
    float containment = 0.0f;
    float worst_outlier = 0.0f;
};

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

        std::vector<GF::Stats::Distributions::FixedMultivariateNormal<N>> getTruePlanDistributions(SYMBOLIC_GRAPH_T::node_t starting_node) {
            const SearchResult& result = queryCache(starting_node)->second;
            ASSERT(result.solution_set.size() == result.pf.size(), "Corrupted search result (pf size does not match solution set size)");
            
            auto pf_it = result.pf.begin();
            std::vector<GF::Stats::Distributions::FixedMultivariateNormal<N>> true_plan_dists;
            true_plan_dists.reserve(result.solution_set.size());
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
                true_plan_dists.push_back(std::move(true_plan_dist));
            }
            return true_plan_dists;
        }

        Biases getBias(SYMBOLIC_GRAPH_T::node_t starting_node, const std::vector<GF::Stats::Distributions::FixedMultivariateNormal<N>>& candidate_plan_distributions) {
            // Calculate the bias types
            Biases biases;

            // Forward bias (coverage)
            std::vector<GF::Stats::Distributions::FixedMultivariateNormal<N>> true_plan_dists = getTruePlanDistributions(starting_node);

            Eigen::Matrix<float, N, 1> dbg_mu_avg = Eigen::Matrix<float, N, 1>::Zero();

            for (const auto& true_plan_dist : true_plan_dists) {
                dbg_mu_avg += true_plan_dist.mu;
                float min_diff = -1.0f;
                for (const auto& candidate_dist : candidate_plan_distributions) {
                    float diff = GF::Stats::wasserstein2(true_plan_dist, candidate_dist);
                    //float diff = GF::Stats::KLD(true_plan_dist, candidate_dist);
                    //float diff = (true_plan_dist.mu - candidate_dist.mu).norm();
                    if (diff < min_diff || min_diff < 0.0f) {
                        min_diff = diff;
                    }
                }

                biases.coverage += min_diff;
                true_plan_dists.push_back(std::move(true_plan_dist));
            }
            //LOG("True pf size: " << true_plan_dists.size());
            dbg_mu_avg /= true_plan_dists.size();
            LOG(" -- Avg mu: " << dbg_mu_avg(0) << ", " << dbg_mu_avg(1));
            if (dbg_mu_avg(0) < 8.0f && dbg_mu_avg(1) < 8.0f)
                WARN(">>>>>>>>>>>>> EXCLUDE <<<<<<<<<<<<<");
            biases.coverage /= static_cast<float>(true_plan_dists.size());

            // Backward bias (containment)
            for (const auto& candidate_dist : candidate_plan_distributions) {
                float min_diff = -1.0f;
                for (const auto& true_plan_dist : true_plan_dists) {
                    float diff = GF::Stats::wasserstein2(true_plan_dist, candidate_dist);
                    //float diff = GF::Stats::KLD(true_plan_dist, candidate_dist);
                    //float diff = (true_plan_dist.mu - candidate_dist.mu).norm();
                    if (diff < min_diff || min_diff < 0.0f) {
                        min_diff = diff;
                    }
                }
                biases.containment += min_diff;
                // Calclulate the worse outlier
                if (min_diff > biases.worst_outlier)
                    biases.worst_outlier = min_diff;
            }
            //LOG("Estimate pf size: " << candidate_plan_distributions.size());
            biases.containment /= static_cast<float>(candidate_plan_distributions.size());

            //LOG(" - Total bias: " << biases.containment + biases.coverage);
            return biases;
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
