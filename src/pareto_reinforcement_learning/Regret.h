#pragma once

#include "TaskPlanner.h"
#include "TrueBehavior.h"
#include "SearchProblem.h"

namespace PRL {

template <class SYMBOLIC_GRAPH_T, uint64_t N>
class Regret {
    public:
        using SearchResult = TP::GraphSearch::MultiObjectiveSearchResult<
            typename SearchProblem<N>::node_t, 
            typename SearchProblem<N>::edge_t, 
            typename SearchProblem<N>::cost_t>;

        using PathSolution = TP::GraphSearch::PathSolution<
            typename SearchProblem<N>::node_t, 
            typename SearchProblem<N>::edge_t, 
            typename SearchProblem<N>::cost_t>;
        


    public:
        Regret(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product, const std::shared_ptr<TrueBehavior<SYMBOLIC_GRAPH_T, N>>& true_behavior)
            : m_product(product)
            , m_true_behavior(true_behavior)
        {}

        float getRegret(SYMBOLIC_GRAPH_T::node_t starting_node) {
            if (m_pareto_front_cache)
        }

        static float paretoRegret(const std::list<PathSolution>& true_pf, const SearchProblem<N>::cost_t& point_sample) {
            float min_epsilon = 0.0f;
            for (const auto& true_path_solution : true_pf) {
                float min_epsilon_i = 0.0f;
                for (uint32_t d = 0; d < N; ++d) {
                    float epsilon = true_path_solution.path_cost[d] - point_sample[d];
                    if (epsilon < min_epsion_i || d == 0) {
                        min_epsilon_i
                    }
                }
            }
        }

    private:
        struct LineSegment {
            Eigen::Matrix<float, N, 1> point_1, point_2;
            bool

        };
    private:
        std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;
        std::shared_ptr<TrueBehavior<SYMBOLIC_GRAPH_T, N>> m_true_behavior
        std::map<SYMBOLIC_GRAPH_T::node_t, std::list<PathSolution>> m_pareto_front_cache;
};

}