#pragma once

#include <functional>

#include "EFE.h"
#include "SearchProblem.h"
#include "TrueBehavior.h"
#include "Quantifier.h"
#include "Animator.h"

namespace PRL {

template <uint32_t M>
class CostLearner {
    public:
        using SymbolicProductGraph = TP::DiscreteModel::SymbolicProductAutomaton<
            TP::DiscreteModel::TransitionSystem, 
            TP::FormalMethods::DFA, 
            TP::DiscreteModel::ModelEdgeInheritor<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA>>;

        using BehaviorHandlerType = CostBehaviorHandler<SymbolicProductGraph, M>;

        using PathSolution = TP::GraphSearch::PathSolution<
            typename SearchProblem<BehaviorHandlerType>::node_t, 
            typename SearchProblem<BehaviorHandlerType>::edge_t, 
            typename SearchProblem<BehaviorHandlerType>::cost_t>;

        using ParetoFrontResult = TP::GraphSearch::MultiObjectiveSearchResult<
            typename SearchProblem<BehaviorHandlerType>::node_t, 
            typename SearchProblem<BehaviorHandlerType>::edge_t, 
            typename SearchProblem<BehaviorHandlerType>::cost_t>;

        using TrajectoryDistribution = TP::Stats::Distributions::FixedMultivariateNormal<BehaviorHandlerType::numBehaviors()>;

        using Plan = TP::Planner::Plan<SearchProblem<BehaviorHandlerType>>;

    public:
        Learner(const std::shared_ptr<BEHAVIOR_HANDLER_T>& behavior_handler, const std::string& write_plan_directory = std::string(), const std::shared_ptr<Animator<BEHAVIOR_HANDLER_T>>& animator = nullptr, bool verbose = false)
            : m_product(behavior_handler->getProduct())
            , m_behavior_handler(behavior_handler)
            , m_write_plan_directory(write_plan_directory)
            , m_animator(animator)
            , m_verbose(verbose)
        {}

        virtual ParetoFrontResult plan(uint8_t completed_tasks_horizon) {
            log("Planning Phase (1)");
            SearchProblem<BEHAVIOR_HANDLER_T> problem(m_product, m_current_product_node, completed_tasks_horizon, m_behavior_handler);
            log("Planning...", true);
            ParetoFrontResult result = [&] {
                if constexpr (BEHAVIOR_HANDLER_T::numBehaviors() == 2)
                    // Use BOA
                    return TP::GraphSearch::BOAStar<typename SearchProblem<BEHAVIOR_HANDLER_T>::cost_t, decltype(problem)>::search(problem);
                else
                    // Use NAMOA
                    return TP::GraphSearch::NAMOAStar<typename SearchProblem<BEHAVIOR_HANDLER_T>::cost_t, decltype(problem)>::search(problem);
            }();
            log("Done!", true);
            for (auto& sol : result.solution_set) {
                // Inverse transform the solutions using the negative of the price function
                sol.path_cost.template get<0>() += m_behavior_handler->priceFunctionTransform(completed_tasks_horizon);
                // Convert back to reward function
                sol.path_cost.template get<0>() *= -1.0f;
            }
            return result;
        }

        std::list<PathSolution>::const_iterator select(const ParetoFrontResult& search_result, const TrajectoryDistribution& p_ev) {
            log("Selection Phase (2)");
            typename std::list<PathSolution>::const_iterator min_it = search_result.solution_set.begin();
            uint32_t min_ind = 0;
            float min_efe = 0.0f;

            // Hold the trajectory distributions for the animation
            std::vector<TrajectoryDistribution> traj_distributions;
            traj_distributions.reserve(search_result.solution_set.size());

            // Hold the ucb pareto points for the animation
            std::vector<typename BEHAVIOR_HANDLER_T::CostVector> ucb_pareto_points;
            ucb_pareto_points.reserve(search_result.solution_set.size());

            uint32_t plan_i = 0;
            for (auto it = search_result.solution_set.begin(); it != search_result.solution_set.end(); ++it) {

                Plan plan(*it, m_product, true);
                TrajectoryDistribution traj_dist = getTrajectoryDistribution(plan);
                if (m_verbose) {
                    PRINT_NAMED("    Solution candidate " << plan_i, "\n" << 
                        "         [cost  : N(" << traj_dist.mu(1) <<", " << traj_dist.Sigma(1,1) <<"), ucb: " << std::to_string(it->path_cost.template get<1>()) << "]\n" <<
                        "         [reward: N(" << traj_dist.mu(0) <<", " << traj_dist.Sigma(0,0) <<"), ucb: " << std::to_string(it->path_cost.template get<0>()) << "]");
                }

                float efe = GuassianEFE<BEHAVIOR_HANDLER_T::numBehaviors()>::calculate(traj_dist, p_ev);
                //LOG("-> efe: " << efe);
                if (it != search_result.solution_set.begin()) {
                    if (efe < min_efe) {
                        min_efe = efe;
                        min_it = it;
                        min_ind = plan_i;
                    }
                } else {
                    min_efe = efe;
                }

                traj_distributions.push_back(std::move(traj_dist));
                ucb_pareto_points.push_back(it->path_cost);
                
                if (!m_write_plan_directory.empty()) {
                    TP::Serializer szr(m_write_plan_directory + "/candidate_plan_" + std::to_string(plan_i) + ".yaml");
                    plan.serialize(szr, 
                        "Candidate Plan " + std::to_string(plan_i) + " at instance: " + std::to_string(m_quantifier.instances));
                    szr.done();
                }

                ++plan_i;
            }
            log("Chosen solution: " + std::to_string(min_ind), true);

            // Add to animator
            if (m_animator)
                m_animator->addInstance(search_result, min_ind, std::move(traj_distributions), std::move(ucb_pareto_points));

            return min_it;
        }

        template <typename SAMPLER_LAM_T>
        bool execute(const Plan& plan, SAMPLER_LAM_T sampler) {
            log("Execution Phase (3)");
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
            m_quantifier.finishInstance();
            TP::Serializer szr(m_write_plan_directory + "/chosen_plan.yaml");
            plan.serialize(szr, "Chosen Plan");
            szr.done();
            // Do not terminate
            return false;
        }

        template <typename SAMPLER_LAM_T>
        const RewardCostQuantifier<BEHAVIOR_HANDLER_T::numCostCriteria()>& run(const TrajectoryDistribution& p_ev, SAMPLER_LAM_T sampler, uint32_t max_instances) {
            ASSERT(m_initialized, "Must initialize before running");
            m_quantifier.max_instances = max_instances;
            while (m_quantifier.instances < max_instances) {
                ParetoFrontResult pf = plan(m_behavior_handler->getCompletedTasksHorizon());
                if (!pf.success) {
                    ERROR("Planner did not succeed!");
                    return m_quantifier;
                }
                auto path_solution = select(pf, p_ev);
                Plan plan(*path_solution, m_product, true);
                if (execute(plan, sampler))
                    return m_quantifier;
                log("Update Phase (4)");
                m_behavior_handler->update(plan.product_node_sequence.back().n_completed_tasks);
                m_current_product_node = plan.product_node_sequence.back();
            }
            return m_quantifier;
        }

        TrajectoryDistribution getTrajectoryDistribution(const Plan& plan) {
            constexpr uint32_t M = BEHAVIOR_HANDLER_T::numBehaviors();
            TP::Containers::FixedArray<M, TP::Stats::Distributions::Normal> individual_distributions;
            auto src_state_it = plan.begin();
            auto dst_state_it = plan.begin();
            ++dst_state_it;
            for (auto action_it = plan.action_sequence.begin(); action_it != plan.action_sequence.end(); ++action_it) {
                TP::Containers::FixedArray<M - 1, TP::Stats::Distributions::Normal> cost_distributions = m_behavior_handler->getNAElement(src_state_it.tsNode(), *action_it).getEstimateDistributions();
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
                }
                ++src_state_it;
                ++dst_state_it;
            }
            TrajectoryDistribution distribution;
            for (uint32_t m = 0; m < M; ++m) {
                distribution.mu(m) = individual_distributions[m].mu;
                for (uint32_t n = 0; n < M; ++n) {
                    distribution.Sigma(m, n) = (m != n) ? 0.0f : individual_distributions[m].sigma_2;
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

    protected:
        void log(const std::string& msg, bool sub_msg = false) {
            if (m_verbose) {
                std::string spc = sub_msg ? "   " : "";
                LOG("[Instance: " << m_quantifier.instances << "] " << spc << msg);
            }
        }

    protected:
        std::shared_ptr<SymbolicProductGraph> m_product;
        std::shared_ptr<BEHAVIOR_HANDLER_T> m_behavior_handler;
        SymbolicProductGraph::node_t m_current_product_node;
        bool m_initialized = false;

        RewardCostQuantifier<BEHAVIOR_HANDLER_T::numCostCriteria()> m_quantifier;

        std::shared_ptr<Animator<BEHAVIOR_HANDLER_T>> m_animator;
        std::string m_write_plan_directory = "";

        bool m_verbose;

};
template <uint32_t COST_CRITERIA_M>
class RewardCostLearner {
    public:
        using SymbolicProductGraph = TP::DiscreteModel::SymbolicProductAutomaton<
            TP::DiscreteModel::TransitionSystem, 
            TP::FormalMethods::DFA, 
            TP::DiscreteModel::ModelEdgeInheritor<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA>>;

        using PathSolution = TP::GraphSearch::PathSolution<
            typename SearchProblem<BEHAVIOR_HANDLER_T>::node_t, 
            typename SearchProblem<BEHAVIOR_HANDLER_T>::edge_t, 
            typename SearchProblem<BEHAVIOR_HANDLER_T>::cost_t>;

        using ParetoFrontResult = TP::GraphSearch::MultiObjectiveSearchResult<
            typename SearchProblem<BEHAVIOR_HANDLER_T>::node_t, 
            typename SearchProblem<BEHAVIOR_HANDLER_T>::edge_t, 
            typename SearchProblem<BEHAVIOR_HANDLER_T>::cost_t>;

        using TrajectoryDistribution = TP::Stats::Distributions::FixedMultivariateNormal<RewardCostBehaviorHandler<SymbolicProductGraph, COST_CRITERIA_M>::numBehaviors()>;

        using Plan = TP::Planner::Plan<SearchProblem<BEHAVIOR_HANDLER_T>>;

    public:
        Learner(const std::shared_ptr<RewardCostBehaviorHandler<SymbolicProductGraph, COST_CRITERIA_M>>& behavior_handler, const std::string& write_plan_directory = std::string(), const std::shared_ptr<Animator<BEHAVIOR_HANDLER_T>>& animator = nullptr, bool verbose = false)
            : m_product(behavior_handler->getProduct())
            , m_behavior_handler(behavior_handler)
            , m_write_plan_directory(write_plan_directory)
            , m_animator(animator)
            , m_verbose(verbose)
        {}

        virtual ParetoFrontResult plan(uint8_t completed_tasks_horizon) {
            log("Planning Phase (1)");
            SearchProblem<BEHAVIOR_HANDLER_T> problem(m_product, m_current_product_node, completed_tasks_horizon, m_behavior_handler);
            log("Planning...", true);
            ParetoFrontResult result = [&] {
                if constexpr (BEHAVIOR_HANDLER_T::numBehaviors() == 2)
                    // Use BOA
                    return TP::GraphSearch::BOAStar<typename SearchProblem<BEHAVIOR_HANDLER_T>::cost_t, decltype(problem)>::search(problem);
                else
                    // Use NAMOA
                    return TP::GraphSearch::NAMOAStar<typename SearchProblem<BEHAVIOR_HANDLER_T>::cost_t, decltype(problem)>::search(problem);
            }();
            log("Done!", true);
            for (auto& sol : result.solution_set) {
                // Inverse transform the solutions using the negative of the price function
                sol.path_cost.template get<0>() += m_behavior_handler->priceFunctionTransform(completed_tasks_horizon);
                // Convert back to reward function
                sol.path_cost.template get<0>() *= -1.0f;
            }
            return result;
        }

        std::list<PathSolution>::const_iterator select(const ParetoFrontResult& search_result, const TrajectoryDistribution& p_ev) {
            log("Selection Phase (2)");
            typename std::list<PathSolution>::const_iterator min_it = search_result.solution_set.begin();
            uint32_t min_ind = 0;
            float min_efe = 0.0f;

            // Hold the trajectory distributions for the animation
            std::vector<TrajectoryDistribution> traj_distributions;
            traj_distributions.reserve(search_result.solution_set.size());

            // Hold the ucb pareto points for the animation
            std::vector<typename BEHAVIOR_HANDLER_T::CostVector> ucb_pareto_points;
            ucb_pareto_points.reserve(search_result.solution_set.size());

            uint32_t plan_i = 0;
            for (auto it = search_result.solution_set.begin(); it != search_result.solution_set.end(); ++it) {

                Plan plan(*it, m_product, true);
                TrajectoryDistribution traj_dist = getTrajectoryDistribution(plan);
                if (m_verbose) {
                    PRINT_NAMED("    Solution candidate " << plan_i, "\n" << 
                        "         [cost  : N(" << traj_dist.mu(1) <<", " << traj_dist.Sigma(1,1) <<"), ucb: " << std::to_string(it->path_cost.template get<1>()) << "]\n" <<
                        "         [reward: N(" << traj_dist.mu(0) <<", " << traj_dist.Sigma(0,0) <<"), ucb: " << std::to_string(it->path_cost.template get<0>()) << "]");
                }

                float efe = GuassianEFE<BEHAVIOR_HANDLER_T::numBehaviors()>::calculate(traj_dist, p_ev);
                //LOG("-> efe: " << efe);
                if (it != search_result.solution_set.begin()) {
                    if (efe < min_efe) {
                        min_efe = efe;
                        min_it = it;
                        min_ind = plan_i;
                    }
                } else {
                    min_efe = efe;
                }

                traj_distributions.push_back(std::move(traj_dist));
                ucb_pareto_points.push_back(it->path_cost);
                
                if (!m_write_plan_directory.empty()) {
                    TP::Serializer szr(m_write_plan_directory + "/candidate_plan_" + std::to_string(plan_i) + ".yaml");
                    plan.serialize(szr, 
                        "Candidate Plan " + std::to_string(plan_i) + " at instance: " + std::to_string(m_quantifier.instances));
                    szr.done();
                }

                ++plan_i;
            }
            log("Chosen solution: " + std::to_string(min_ind), true);

            // Add to animator
            if (m_animator)
                m_animator->addInstance(search_result, min_ind, std::move(traj_distributions), std::move(ucb_pareto_points));

            return min_it;
        }

        template <typename SAMPLER_LAM_T>
        bool execute(const Plan& plan, SAMPLER_LAM_T sampler) {
            log("Execution Phase (3)");
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
            m_quantifier.finishInstance();
            TP::Serializer szr(m_write_plan_directory + "/chosen_plan.yaml");
            plan.serialize(szr, "Chosen Plan");
            szr.done();
            // Do not terminate
            return false;
        }

        template <typename SAMPLER_LAM_T>
        const RewardCostQuantifier<BEHAVIOR_HANDLER_T::numCostCriteria()>& run(const TrajectoryDistribution& p_ev, SAMPLER_LAM_T sampler, uint32_t max_instances) {
            ASSERT(m_initialized, "Must initialize before running");
            m_quantifier.max_instances = max_instances;
            while (m_quantifier.instances < max_instances) {
                ParetoFrontResult pf = plan(m_behavior_handler->getCompletedTasksHorizon());
                if (!pf.success) {
                    ERROR("Planner did not succeed!");
                    return m_quantifier;
                }
                auto path_solution = select(pf, p_ev);
                Plan plan(*path_solution, m_product, true);
                if (execute(plan, sampler))
                    return m_quantifier;
                log("Update Phase (4)");
                m_behavior_handler->update(plan.product_node_sequence.back().n_completed_tasks);
                m_current_product_node = plan.product_node_sequence.back();
            }
            return m_quantifier;
        }

        TrajectoryDistribution getTrajectoryDistribution(const Plan& plan) {
            constexpr uint32_t M = BEHAVIOR_HANDLER_T::numBehaviors();
            TP::Containers::FixedArray<M, TP::Stats::Distributions::Normal> individual_distributions;
            auto src_state_it = plan.begin();
            auto dst_state_it = plan.begin();
            ++dst_state_it;
            for (auto action_it = plan.action_sequence.begin(); action_it != plan.action_sequence.end(); ++action_it) {
                TP::Containers::FixedArray<M - 1, TP::Stats::Distributions::Normal> cost_distributions = m_behavior_handler->getNAElement(src_state_it.tsNode(), *action_it).getEstimateDistributions();
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
                }
                ++src_state_it;
                ++dst_state_it;
            }
            TrajectoryDistribution distribution;
            for (uint32_t m = 0; m < M; ++m) {
                distribution.mu(m) = individual_distributions[m].mu;
                for (uint32_t n = 0; n < M; ++n) {
                    distribution.Sigma(m, n) = (m != n) ? 0.0f : individual_distributions[m].sigma_2;
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

    protected:
        void log(const std::string& msg, bool sub_msg = false) {
            if (m_verbose) {
                std::string spc = sub_msg ? "   " : "";
                LOG("[Instance: " << m_quantifier.instances << "] " << spc << msg);
            }
        }

    protected:
        std::shared_ptr<SymbolicProductGraph> m_product;
        std::shared_ptr<RewardCostBehaviorHandler<SymbolicProductGraph, COST_CRITERIA_M>> m_behavior_handler;
        SymbolicProductGraph::node_t m_current_product_node;
        bool m_initialized = false;

        RewardCostQuantifier<RewardCostBehaviorHandler<SymbolicProductGraph, COST_CRITERIA_M>::numCostCriteria()> m_quantifier;

        std::shared_ptr<Animator<BEHAVIOR_HANDLER_T>> m_animator;
        std::string m_write_plan_directory = "";

        bool m_verbose;

};
}