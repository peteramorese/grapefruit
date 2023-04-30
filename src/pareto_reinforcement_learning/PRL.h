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

        using TrajectoryDistribution = TP::Stats::Distributions::FixedMultivariateNormal<BEHAVIOR_HANDLER_T::numBehaviors()>;

    public:
        ParetoReinforcementLearner(const std::shared_ptr<BEHAVIOR_HANDLER_T>& behavior_handler)
            : m_product(behavior_handler->getProduct())
            , m_behavior_handler(behavior_handler)
        {}

        ParetoFrontResult computePlan(uint8_t completed_tasks_horizon) {
            PRLSearchProblem<BEHAVIOR_HANDLER_T> problem(m_product, m_current_product_node, completed_tasks_horizon, m_behavior_handler);
            LOG("Planning...");
            ParetoFrontResult result = TP::GraphSearch::NAMOAStar<typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::cost_t, decltype(problem)>::search(problem);
            LOG("Done!");
            for (auto& sol : result.solution_set) {
                // Inverse transform the solutions using the negative of the price function
                LOG("prev mean reward: " << sol.path_cost.template get<0>());
                sol.path_cost.template get<0>() += m_behavior_handler->priceFunctionTransform(completed_tasks_horizon);
                // Convert back to reward function
                sol.path_cost.template get<0>() *= -1.0f;
                LOG("post mean reward: " << sol.path_cost.template get<0>());
            }
            return result;
        }

        std::list<Plan>::const_iterator select(const ParetoFrontResult& search_result, const TrajectoryDistribution& p_ev) {
            typename std::list<Plan>::const_iterator min_it = search_result.solution_set.begin();
            float min_efe = 0.0f;

            auto costToStr = [](const typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::cost_t& cv) {
                std::string s = "(reward mean: " + std::to_string(cv.template get<0>()) + " variance: " + std::to_string(cv.template get<1>()) + ")";
                
                for (uint32_t i=2; i<PRLSearchProblem<BEHAVIOR_HANDLER_T>::cost_t::size(); i += 2) {
                    s += ", (cost " + std::to_string(i/2) + " mean: " + std::to_string(cv[i]) + " variance: " + std::to_string(cv[i + 1]) + ") ";
                }
                return s;
            };

            for (auto it = search_result.solution_set.begin(); it != search_result.solution_set.end(); ++it) {
                
                LOG("Considering solution: " << costToStr(it->path_cost));
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
            LOG("solutions size: " << search_result.solution_set.size());
            LOG("Chosen solution: " << costToStr(min_it->path_cost));
            return min_it;
        }

        void execute(const Plan& plan) {
            // TODO
        }

        void run(const TrajectoryDistribution& p_ev) {
            ASSERT(m_initialized, "Must initialize before running");
            // while (true)
            ParetoFrontResult pf = computePlan(m_behavior_handler->getCompletedTasksHorizon());
            if (!pf.success) {
                LOG("Planner did not succeed!");
                return;
            }
            auto plan = select(pf, p_ev);
            execute(*plan);
            m_behavior_handler->update(plan->node_path.end()->n_completed_tasks);
        }

        static TrajectoryDistribution reconstructDistribution(const PRLSearchProblem<BEHAVIOR_HANDLER_T>::cost_t& cv) {
            TrajectoryDistribution distribution;
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

        void initialize(const TP::DiscreteModel::State& init_state) {
            TP::Containers::SizedArray<TP::Node> init_aut_nodes(m_product->rank() - 1);
            for (uint32_t i=0; i<init_aut_nodes.size(); ++i) init_aut_nodes[i] = *(m_product->getAutomaton(i).getInitStates().begin());
            m_current_product_node = m_product->getWrappedNode(m_product->getModel().getGenericNodeContainer()[init_state], init_aut_nodes);
            m_initialized = true;
        }

    private:
        std::shared_ptr<SymbolicProductGraph> m_product;
        std::shared_ptr<BEHAVIOR_HANDLER_T> m_behavior_handler;
        SymbolicProductGraph::node_t m_current_product_node;
        bool m_initialized = false;

};
}