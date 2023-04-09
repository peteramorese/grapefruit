#pragma once

#include "EFE.h"
#include "PRLSearchProblem.h"

namespace PRL {

template <class BEHAVIOR_HANDLER_T>
class ParetoReinforcementLearner {
    public:
        using SymbolicProductGraph = TP::DiscreteModel::SymbolicProductAutomaton<
            TP::DiscreteModel::TransitionSystem, 
            TP::FormalMethods::DFA, 
            TP::DiscreteModel::ModelEdgeInheritor<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA>>;

        using Plan = TP::GraphSearch::PathSolution<
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::node_t, 
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::edge_t, 
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::cost_t>;

        using ParetoFrontResult = TP::GraphSearch::MultiObjectiveSearchResult<
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::node_t, 
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::edge_t, 
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::cost_t>;

        using Distribution = TP::Distributions::FixedMultivariateGuassian<BEHAVIOR_HANDLER_T::numBehaviors()>;

    public:
        ParetoReinforcementLearner(const std::shared_ptr<TP::DiscreteModel::TransitionSystem>& ts, const std::vector<std::shared_ptr<TP::FormalMethods::DFA>>& automata)
            : m_product(std::make_shared<SymbolicProductGraph>(ts, automata))
        {}

        ParetoFrontResult computePlan(uint32_t step_horizon) {
            PRLSearchProblem problem(m_product, step_horizon, m_behavior_handler);
            ParetoFrontResult result = TP::GraphSearch::NAMOAStar<typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::cost_t, decltype(problem)>::search(problem);
            for (auto& sol : result.solution_set) {
                // Inverse transform the solutions using the negative of the price function
                sol.path_cost.template get<0>() -= m_behavior_handler->priceFunctionTransform(step_horizon);
                // Convert back to reward function
                sol.path_cost.template get<0>() *= -1.0f;
            }
            return result;
        }

        std::list<Plan>::const_iterator select(const ParetoFrontResult& search_result, const Distribution& p_ev) {
            typename std::list<Plan>::const_iterator min_it = search_result.solution_set.begin();
            float min_efe = 0.0f;
            for (auto it = search_result.solution_set.begin(); it != search_result.solution_set.end(); ++it) {
                Distribution reconstructed_plan_dist = reconstructDistribution(it->path_cost);

                float efe = GuassianEFE<BEHAVIOR_HANDLER_T::numBehaviors()>::calculate(reconstructed_plan_dist, p_ev);
                if (it != search_result.solution_set.begin()) {
                    if (efe < min_efe) {
                        min_efe = efe;
                        min_it = it;
                    }
                } else {
                    min_efe = efe;
                }
            }
            return min_it;
        }

        void execute(const Plan& plan) {
            // TODO
        }

        void run(const Distribution& p_ev) {
            // while (true)
            ParetoFrontResult pf = computePlan(10);
            auto plan = select(pf, p_ev);
            execute(*plan);
        }

        static Distribution reconstructDistribution(const PRLSearchProblem<BEHAVIOR_HANDLER_T>::cost_t& cv) {
            Distribution distribution;
            for (uint32_t i=0; i < BEHAVIOR_HANDLER_T::cvDim(); ++i) {
                if (i % 2 == 0) {
                    distribution.mean(i / 2u) = cv[i];
                } else {
                    uint32_t index = (i - 1u) / 2u;
                    distribution.covariance(index, index) = cv[i];
                }
            }
            return distribution;
        }

    private:
        std::shared_ptr<SymbolicProductGraph> m_product;
        std::shared_ptr<BEHAVIOR_HANDLER_T> m_behavior_handler;


};
}