#pragma once

#include <functional>

#include "EFE.h"
#include "SearchProblem.h"
#include "TrueBehavior.h"
#include "Quantifier.h"
#include "Animator.h"
#include "TrajectoryDistributionEstimator.h"

namespace PRL {

template <uint64_t N>
class Learner {
    public:
        using SymbolicProductGraph = TP::DiscreteModel::SymbolicProductAutomaton<
            TP::DiscreteModel::TransitionSystem, 
            TP::FormalMethods::DFA, 
            TP::DiscreteModel::ModelEdgeInheritor<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA>>;

        using BehaviorHandlerType = BehaviorHandler<SymbolicProductGraph, N>;

        using PathSolution = TP::GraphSearch::PathSolution<
            typename SearchProblem<N>::node_t, 
            typename SearchProblem<N>::edge_t, 
            typename SearchProblem<N>::cost_t>;

        using ParetoFrontResult = TP::GraphSearch::MultiObjectiveSearchResult<
            typename SearchProblem<N>::node_t, 
            typename SearchProblem<N>::edge_t, 
            typename SearchProblem<N>::cost_t>;

        using PreferenceDistribution = TP::Stats::Distributions::FixedMultivariateNormal<N>;

        using Plan = TP::Planner::Plan<SearchProblem<N>>;

    public:
        Learner(const std::shared_ptr<BehaviorHandlerType>& behavior_handler, uint32_t n_samples = 1000, const std::shared_ptr<Animator<N>>& animator = nullptr, bool verbose = false)
            : m_product(behavior_handler->getProduct())
            , m_behavior_handler(behavior_handler)
            , m_n_samples(n_samples)
            , m_animator(animator)
            , m_verbose(verbose)
        {}

        virtual ParetoFrontResult plan(uint8_t completed_tasks_horizon) {
            log("Planning Phase (1)");
            SearchProblem<N> problem(m_product, m_current_product_node, completed_tasks_horizon, m_behavior_handler);
            log("Planning...", true);

            ParetoFrontResult result = [&] {
                if constexpr (N == 2)
                    // Use BOA
                    return TP::GraphSearch::BOAStar<typename SearchProblem<N>::cost_t, decltype(problem)>::search(problem);
                else
                    // Use NAMOA
                    return TP::GraphSearch::NAMOAStar<typename SearchProblem<N>::cost_t, decltype(problem)>::search(problem);
            }();
            log("Done!", true);
            return result;
        }

        std::list<PathSolution>::const_iterator select(const ParetoFrontResult& search_result, const PreferenceDistribution& p_ev) {
            log("Selection Phase (2)");
            typename std::list<PathSolution>::const_iterator min_it = search_result.solution_set.begin();
            uint32_t min_ind = 0;
            float min_efe = 0.0f;

            // Hold the trajectory distributions for the animation
            std::vector<TP::Stats::Distributions::FixedMultivariateNormal<N>> estimate_traj_distributions;
            estimate_traj_distributions.reserve(search_result.solution_set.size());

            // Hold the ucb pareto points for the animation
            std::vector<typename BehaviorHandlerType::CostVector> ucb_pareto_points;
            ucb_pareto_points.reserve(search_result.solution_set.size());

            uint32_t plan_i = 0;
            for (auto it = search_result.solution_set.begin(); it != search_result.solution_set.end(); ++it) {

                Plan plan(*it, m_product, true);
                TrajectoryDistributionUpdaters<N> traj_updaters = getTrajectoryUpdaters(plan);
                if (m_verbose) {
                    PRINT_NAMED("    Solution candidate " << plan_i, "\n" << 
                        "         [cost ucb....:" << std::to_string(it->path_cost.template get<1>()) << "]\n" <<
                        "         [reward ucb..:" << std::to_string(it->path_cost.template get<0>()) << "]");
                }

                float efe = GaussianEFE<N>::calculate(traj_updaters, p_ev, m_n_samples);
                LOG("-> efe: " << efe);
                if (it != search_result.solution_set.begin()) {
                    if (efe < min_efe) {
                        min_efe = efe;
                        min_it = it;
                        min_ind = plan_i;
                    }
                } else {
                    min_efe = efe;
                }

                auto estimate_traj_distribution = GaussianEFE<N>::getCEQObservationDistribution(traj_updaters);
                //LOG("ceq dist mean: \n" << estimate_traj_distribution.mu << "\n");
                //PAUSE;
                estimate_traj_distributions.push_back(estimate_traj_distribution);
                ucb_pareto_points.push_back(it->path_cost);
                
                ++plan_i;
            }
            log("Chosen solution: " + std::to_string(min_ind), true);

            // Add to animator
            if (m_animator)
                m_animator->addInstance(search_result, min_ind, std::move(estimate_traj_distributions), std::move(ucb_pareto_points));

            return min_it;
        }

        template <typename SAMPLER_LAM_T>
        bool execute(const Plan& plan, SAMPLER_LAM_T sampler) {
            log("Execution Phase (3)");
            auto node_it = plan.product_node_sequence.begin();
            for (const auto& action : plan.action_sequence) {
                const auto& src_node = *node_it;
                const auto& dst_node = *(++node_it);
                TP::Containers::FixedArray<N, float> sample = sampler(src_node.base_node, dst_node.base_node, action);
                m_quantifier.addSample(sample);
                m_behavior_handler->visit(src_node, action, sample);
            }
            m_quantifier.finishInstance();

            // Do not terminate
            return false;
        }

        template <typename SAMPLER_LAM_T>
        const Quantifier<N>& run(const PreferenceDistribution& p_ev, SAMPLER_LAM_T sampler, uint32_t max_instances) {
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
                m_current_product_node = plan.product_node_sequence.back();
            }
            return m_quantifier;
        }

        TrajectoryDistributionUpdaters<N> getTrajectoryUpdaters(const Plan& plan) {
            TrajectoryDistributionUpdaters<N> updaters;
            auto state_it = plan.begin();
            for (auto action_it = plan.action_sequence.begin(); action_it != plan.action_sequence.end(); ++action_it) {
                updaters.add(m_behavior_handler->getElement(state_it.tsNode(), *action_it).getUpdater());
                ++state_it;
            }

            return updaters;
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
        std::shared_ptr<BehaviorHandlerType> m_behavior_handler;
        SymbolicProductGraph::node_t m_current_product_node;
        bool m_initialized = false;
        uint32_t m_n_samples;

        Quantifier<N> m_quantifier;

        std::shared_ptr<Animator<N>> m_animator;

        bool m_verbose;
};
}