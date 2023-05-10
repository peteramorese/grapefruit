#pragma once

#include <functional>

#include "EFE.h"
#include "PRLSearchProblem.h"
#include "TrueBehavior.h"

namespace PRL {

template <uint32_t COST_CRITERIA_M>
struct PRLQuantifier {
    float cumulative_reward = 0.0f;
    TP::Containers::FixedArray<COST_CRITERIA_M, float> cumulative_cost;
    uint32_t steps = 0u;
    //uint32_t max_steps = 0u;
    uint32_t decision_instances = 0u;
    uint32_t max_decision_instances = 0u;

    PRLQuantifier() {
        for (uint32_t i = 0; i < COST_CRITERIA_M; ++i) cumulative_cost[i] = 0.0f;
    }
    void addSample(const BehaviorSample<COST_CRITERIA_M>& sample) {
        float total_reward_this_step = 0.0f;
        for (auto[contains, r] : sample.getRewards()) {
            if (contains) 
                cumulative_reward += r;
        }
        cumulative_cost += sample.cost_sample;
        ++steps;
    }
    float avgRewardPerDI() const {return cumulative_reward / static_cast<float>(decision_instances);}
    float avgCostPerDI(uint32_t cost_criteria_i = 0) const {return cumulative_cost[cost_criteria_i] / static_cast<float>(decision_instances);}
};

template <class BEHAVIOR_HANDLER_T>
class ParetoReinforcementLearner {
    public:
        using SymbolicProductGraph = TP::DiscreteModel::SymbolicProductAutomaton<
            TP::DiscreteModel::TransitionSystem, 
            TP::FormalMethods::DFA, 
            TP::DiscreteModel::ModelEdgeInheritor<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA>>;

        using PathSolution = TP::GraphSearch::PathSolution<
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::node_t, 
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::edge_t, 
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::cost_t>;

        using ParetoFrontResult = TP::GraphSearch::MultiObjectiveSearchResult<
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::node_t, 
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::edge_t, 
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::cost_t>;

        using TrajectoryDistribution = TP::Stats::Distributions::FixedMultivariateNormal<BEHAVIOR_HANDLER_T::numBehaviors()>;

        using Plan = TP::Planner::Plan<PRLSearchProblem<BEHAVIOR_HANDLER_T>>;

        //typedef BehaviorSample<BEHAVIOR_HANDLER_T::numCostCriteria()>(*SamplerFunctionType)(TP::WideNode src_node, TP::WideNode dst_node, const TP::DiscreteModel::Action& action);

    public:
        ParetoReinforcementLearner(const std::shared_ptr<BEHAVIOR_HANDLER_T>& behavior_handler)
            : m_product(behavior_handler->getProduct())
            , m_behavior_handler(behavior_handler)
        {}

        ParetoFrontResult computePlan(uint8_t completed_tasks_horizon) {
            LOG("PRL current product node: " << m_current_product_node);
            PRLSearchProblem<BEHAVIOR_HANDLER_T> problem(m_product, m_current_product_node, completed_tasks_horizon, m_behavior_handler);
            LOG("Planning...");
            //ParetoFrontResult result = TP::GraphSearch::NAMOAStar<typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::cost_t, decltype(problem)>::search(problem);
            ParetoFrontResult result = TP::GraphSearch::BOAStar<typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::cost_t, decltype(problem)>::search(problem);
            LOG("Done!");
            for (auto& sol : result.solution_set) {
                // Inverse transform the solutions using the negative of the price function
                //LOG("prev mean reward: " << sol.path_cost.template get<0>());
                sol.path_cost.template get<0>() += m_behavior_handler->priceFunctionTransform(completed_tasks_horizon);
                // Convert back to reward function
                sol.path_cost.template get<0>() *= -1.0f;
                //LOG("post mean reward: " << sol.path_cost.template get<0>());
            }
            return result;
        }

        std::list<PathSolution>::const_iterator select(const ParetoFrontResult& search_result, const TrajectoryDistribution& p_ev) {
            typename std::list<PathSolution>::const_iterator min_it = search_result.solution_set.begin();
            float min_efe = 0.0f;

            auto costToStr = [](const typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::cost_t& cv) {
                std::string s = "(reward ucb: " + std::to_string(cv.template get<0>()) + " cost ucb: " + std::to_string(cv.template get<1>()) + ")";
                
                //for (uint32_t i=2; i<PRLSearchProblem<BEHAVIOR_HANDLER_T>::cost_t::size(); i += 2) {
                //    s += ", (cost " + std::to_string(i/2) + " mean: " + std::to_string(cv[i]) + " variance: " + std::to_string(cv[i + 1]) + ") ";
                //}
                return s;
            };

            uint32_t plan_i = 0;
            for (auto it = search_result.solution_set.begin(); it != search_result.solution_set.end(); ++it) {
                
                LOG("Considering solution: " << costToStr(it->path_cost));

                Plan plan(*it, m_product, true);
                //TrajectoryDistribution traj_dist = getTrajectoryDistribution(*it);
                TrajectoryDistribution traj_dist = getTrajectoryDistribution(plan);
                LOG("-> Trajectory distribution (reward mean: " << traj_dist.mu(0) << ", cost mean: " << traj_dist.mu(1) << ")");
                //float cost_mean = traj_dist.mu(1);
                //std::cout<<" cost mean: " << cost_mean;

                float efe = GuassianEFE<BEHAVIOR_HANDLER_T::numBehaviors()>::calculate(traj_dist, p_ev);
                //LOG("-> efe: " << efe);
                if (it != search_result.solution_set.begin()) {
                    if (efe < min_efe) {
                        min_efe = efe;
                        min_it = it;
                    }
                } else {
                    min_efe = efe;
                }

                plan.serialize("prl_plans/candidate_plan_" + std::to_string(plan_i++) + ".yaml", 
                    "Candidate Plan " + std::to_string(plan_i++) + " at decision instance: " + std::to_string(m_quantifier.decision_instances));
            }
            LOG("solutions size: " << search_result.solution_set.size());
            LOG("Chosen solution: " << costToStr(min_it->path_cost));
            return min_it;
        }

        template <typename SAMPLER_LAM_T>
        bool execute(const Plan& plan, SAMPLER_LAM_T sampler) {
            auto node_it = plan.product_node_sequence.begin();
            for (const auto& action : plan.action_sequence) {
                const auto& src_node = *node_it;
                const auto& dst_node = *(++node_it);
                BehaviorSample<BEHAVIOR_HANDLER_T::numCostCriteria()> sample = sampler(src_node.base_node, dst_node.base_node, action);
                m_quantifier.addSample(sample);
                m_behavior_handler->visit(src_node, action, sample.cost_sample);
                if (sample.hasRewards()) {
                    TP::DiscreteModel::ProductRank task_i;
                    for (auto[contains, r] : sample.getRewards()) {
                        if (contains) 
                            m_behavior_handler->collect(task_i, r);
                        ++task_i;
                    }
                }
                //if (m_quantifier.steps >= m_quantifier.max_steps)
                //    // Terminate
                //    return true;
            }
            plan.serialize("prl_plans/end_plan.yaml", "Chosen Plan");
            // Do not terminate
            return false;
        }

        template <typename SAMPLER_LAM_T>
        const PRLQuantifier<BEHAVIOR_HANDLER_T::numCostCriteria()>& run(const TrajectoryDistribution& p_ev, SAMPLER_LAM_T sampler, uint32_t max_decision_instances) {
            ASSERT(m_initialized, "Must initialize before running");
            m_quantifier.max_decision_instances = max_decision_instances;
            while (m_quantifier.decision_instances < max_decision_instances) {
                ParetoFrontResult pf = computePlan(m_behavior_handler->getCompletedTasksHorizon());
                if (!pf.success) {
                    LOG("Planner did not succeed!");
                    return m_quantifier;
                }
                auto path_solution = select(pf, p_ev);
                Plan plan(*path_solution, m_product, true);
                if (execute(plan, sampler))
                    return m_quantifier;
                m_behavior_handler->update(plan.product_node_sequence.back().n_completed_tasks);
                m_current_product_node = plan.product_node_sequence.back();
                ++m_quantifier.decision_instances;
            }
            return m_quantifier;
        }

        TrajectoryDistribution getTrajectoryDistribution(const Plan& plan) {
            constexpr uint32_t M = BEHAVIOR_HANDLER_T::numBehaviors();
            TP::Containers::FixedArray<M, TP::Stats::Distributions::Normal> individual_distributions;
            auto src_state_it = plan.begin();
            auto dst_state_it = ++plan.begin();
            for (auto action_it = plan.action_sequence.begin(); action_it != plan.action_sequence.end(); ++action_it) {
                TP::Containers::FixedArray<M - 1, TP::Stats::Distributions::Normal> cost_distributions = m_behavior_handler->getNAPElement(src_state_it.tsNode(), *action_it).getEstimateDistributions();
                for (uint32_t m = 0; m < M; ++m) {
                    if (m != 0) {
                        individual_distributions[m].convolveWith(cost_distributions[m - 1]);
                    } else {
                        for (TP::DiscreteModel::ProductRank automaton_i = 0; automaton_i < m_product->rank() - 1; ++automaton_i) {
                            if (!m_product->acc(src_state_it.productNode().base_node, automaton_i) && m_product->acc(dst_state_it.productNode().base_node, automaton_i)) {
                                // Accumulate reward for each task satisfied
                                individual_distributions[m].convolveWith(m_behavior_handler->getTaskElement(automaton_i).updater.getEstimateNormal());
                            }
                        }
                    }
                    //LOG("Individual distribution " << m << " mean: " << individual_distributions[m].mu << " sigma2: " << individual_distributions[m].sigma_2);
                }
                ++src_state_it;
                ++dst_state_it;
            }
            TrajectoryDistribution distribution;
            for (uint32_t m = 0; m < M; ++m) {
                distribution.mu(m) = individual_distributions[m].mu;
                for (uint32_t n = 0; n < M; ++n) {
                    distribution.covariance(m, n) = (m != n) ? 0.0f : individual_distributions[m].sigma_2;
                }
            }

            //Eigen::IOFormat OctaveFmt(Eigen::StreamPrecision, 0, ", ", ";\n", "", "", "[", "]");
            //LOG("TrajectoryDistribution covariance: \n" << distribution.covariance.format(OctaveFmt));
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

        PRLQuantifier<BEHAVIOR_HANDLER_T::numCostCriteria()> m_quantifier;

};
}