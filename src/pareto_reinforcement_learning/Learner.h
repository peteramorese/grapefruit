#pragma once

#include <functional>

#include "TaskPlanner.h"

#include "EFE.h"
#include "SearchProblem.h"
#include "TrueBehavior.h"
#include "Quantifier.h"
#include "DataCollector.h"
#include "TrajectoryDistributionEstimator.h"

namespace PRL {

enum class Selector {
    Aif,        // Active inference preference selector
    Uniform,    // Uniformly random objective selector
    Topsis,     // TOPSIS objective selector
    Weights,    // Scalar weight inner product preference selector
};

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

        using SearchResult = TP::GraphSearch::MultiObjectiveSearchResult<
            typename SearchProblem<N>::node_t, 
            typename SearchProblem<N>::edge_t, 
            typename SearchProblem<N>::cost_t>;

        using PreferenceDistribution = TP::Stats::Distributions::FixedMultivariateNormal<N>;

        using Plan = TP::Planner::Plan<SearchProblem<N>>;

    public:
        Learner(const std::shared_ptr<BehaviorHandlerType>& behavior_handler, uint32_t n_samples = 1000, const std::shared_ptr<DataCollector<N>>& animator = nullptr, bool verbose = false)
            : m_product(behavior_handler->getProduct())
            , m_behavior_handler(behavior_handler)
            , m_n_samples(n_samples)
            , m_data_collector(animator)
            , m_verbose(verbose)
        {}

        virtual SearchResult plan(uint8_t completed_tasks_horizon) {
            log("Planning Phase (1)");
            SearchProblem<N> problem(m_product, m_current_product_node, completed_tasks_horizon, m_behavior_handler);
            log("Planning...", true);

            SearchResult result = [&] {
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

        std::list<PathSolution>::const_iterator selectAif(const std::list<PathSolution>& ucb_pf, const PreferenceDistribution& p_ev) {
            log("Selection Phase (2)");
            typename std::list<PathSolution>::const_iterator min_it = ucb_pf.begin();
            uint32_t min_ind = 0;
            float min_efe = 0.0f;

            // Hold the trajectory distributions for the animation
            std::vector<TP::Stats::Distributions::FixedMultivariateNormal<N>> estimate_traj_distributions;
            estimate_traj_distributions.reserve(ucb_pf.size());

            // Hold the ucb pareto points for the animation
            std::vector<typename BehaviorHandlerType::CostVector> ucb_pareto_points;
            ucb_pareto_points.reserve(ucb_pf.size());

            uint32_t plan_i = 0;
            for (auto it = ucb_pf.begin(); it != ucb_pf.end(); ++it) {

                Plan plan(*it, m_product, true);
                TrajectoryDistributionUpdaters<N> traj_updaters = getTrajectoryUpdaters(plan);
                if (m_verbose) {
                    //std::string
                    PRINT_NAMED("    Solution candidate ", plan_i);
                    for (uint32_t i = 0; i < N; ++i) {
                        PRINT("      [Cost ucb (obj " << i <<")]:" << it->path_cost[i]);
                    }
                }

                float info_gain;
                float efe = GaussianEFE<N>::calculate(traj_updaters, p_ev, m_n_samples, &info_gain);
                LOG("-> efe: " << efe << " (info gain: " << info_gain << ")");
                if (it != ucb_pf.begin()) {
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
            if (m_data_collector)
                m_data_collector->addInstance(ucb_pf, min_ind, std::move(estimate_traj_distributions), std::move(ucb_pareto_points));

            return min_it;
        }

        std::list<PathSolution>::const_iterator selectUniform(const std::list<PathSolution>& ucb_pf, const std::list<PathSolution>& mean_pf) {
            typename std::list<PathSolution>::const_iterator chosen_plan = TP::ParetoSelector<typename SearchProblem<N>::node_t, typename SearchProblem<N>::edge_t, typename SearchProblem<N>::cost_t>::uniformRandom(mean_pf);

            // Hold the trajectory distributions for the animation
            std::vector<TP::Stats::Distributions::FixedMultivariateNormal<N>> estimate_traj_distributions;
            estimate_traj_distributions.reserve(ucb_pf.size());

            // Hold the ucb pareto points for the animation
            std::vector<typename BehaviorHandlerType::CostVector> ucb_pareto_points;
            ucb_pareto_points.reserve(ucb_pf.size());

            for (const auto& pt : mean_pf) {
                Plan plan(pt, m_product, true);
                TrajectoryDistributionUpdaters<N> traj_updaters = getTrajectoryUpdaters(plan);
                auto estimate_traj_distribution = GaussianEFE<N>::getCEQObservationDistribution(traj_updaters);
                estimate_traj_distributions.push_back(estimate_traj_distribution);
                ucb_pareto_points.push_back(pt.path_cost);
            }
            uint32_t selected_ind = getListIndex(mean_pf, chosen_plan);
            log("Chosen solution: " + std::to_string(selected_ind), true);

            // Add to animator
            if (m_data_collector)
                m_data_collector->addInstance(mean_pf, selected_ind, std::move(estimate_traj_distributions), std::move(ucb_pareto_points));
            return chosen_plan;
        }

        std::list<PathSolution>::const_iterator selectTopsis(const std::list<PathSolution>& ucb_pf, const std::list<PathSolution>& mean_pf) {
            typename std::list<PathSolution>::const_iterator chosen_plan = TP::ParetoSelector<typename SearchProblem<N>::node_t, typename SearchProblem<N>::edge_t, typename SearchProblem<N>::cost_t>::TOPSIS(mean_pf);

            // Hold the trajectory distributions for the animation
            std::vector<TP::Stats::Distributions::FixedMultivariateNormal<N>> estimate_traj_distributions;
            estimate_traj_distributions.reserve(ucb_pf.size());

            // Hold the ucb pareto points for the animation
            std::vector<typename BehaviorHandlerType::CostVector> ucb_pareto_points;
            ucb_pareto_points.reserve(ucb_pf.size());

            for (const auto& pt : mean_pf) {
                Plan plan(pt, m_product, true);
                TrajectoryDistributionUpdaters<N> traj_updaters = getTrajectoryUpdaters(plan);
                auto estimate_traj_distribution = GaussianEFE<N>::getCEQObservationDistribution(traj_updaters);
                estimate_traj_distributions.push_back(estimate_traj_distribution);
                ucb_pareto_points.push_back(pt.path_cost);
            }

            uint32_t selected_ind = getListIndex(mean_pf, chosen_plan);
            log("Chosen solution: " + std::to_string(selected_ind), true);

            // Add to animator
            if (m_data_collector)
                m_data_collector->addInstance(mean_pf, selected_ind, std::move(estimate_traj_distributions), std::move(ucb_pareto_points));
            return chosen_plan;
        }

        std::list<PathSolution>::const_iterator selectWeights(const std::list<PathSolution>& ucb_pf, const std::list<PathSolution>& mean_pf, const PreferenceDistribution& p_ev) {
            TP::Containers::FixedArray<N, float> weights;
            float max_val = 0.0f;
            for (uint64_t d = 0; d < N; ++d) {
                weights[d] = p_ev.mu(d);
                if (weights[d] > max_val)
                    max_val = weights[d];
            }
            ASSERT(max_val > 0.0f, "At least one weight must be greater than zero");

            typename std::list<PathSolution>::const_iterator chosen_plan = TP::ParetoSelector<typename SearchProblem<N>::node_t, typename SearchProblem<N>::edge_t, typename SearchProblem<N>::cost_t>::scalarWeights(mean_pf, weights);

            // Hold the trajectory distributions for the animation
            std::vector<TP::Stats::Distributions::FixedMultivariateNormal<N>> estimate_traj_distributions;
            estimate_traj_distributions.reserve(ucb_pf.size());

            // Hold the ucb pareto points for the animation
            std::vector<typename BehaviorHandlerType::CostVector> ucb_pareto_points;
            ucb_pareto_points.reserve(ucb_pf.size());

            for (const auto& pt : mean_pf) {
                Plan plan(pt, m_product, true);
                TrajectoryDistributionUpdaters<N> traj_updaters = getTrajectoryUpdaters(plan);
                auto estimate_traj_distribution = GaussianEFE<N>::getCEQObservationDistribution(traj_updaters);
                estimate_traj_distributions.push_back(estimate_traj_distribution);
                ucb_pareto_points.push_back(pt.path_cost);
            }

            uint32_t selected_ind = getListIndex(mean_pf, chosen_plan);
            log("Chosen solution: " + std::to_string(selected_ind), true);

            // Add to animator
            if (m_data_collector)
                m_data_collector->addInstance(mean_pf, selected_ind, std::move(estimate_traj_distributions), std::move(ucb_pareto_points));
            return chosen_plan;
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
        const Quantifier<N>& run(const PreferenceDistribution& p_ev, SAMPLER_LAM_T sampler, uint32_t max_instances, Selector selector) {
            ASSERT(m_initialized, "Must initialize before running");
            m_quantifier.max_instances = max_instances;
            while (m_quantifier.instances < max_instances) {
                SearchResult result = plan(m_behavior_handler->getCompletedTasksHorizon());
                if (!result.success) {
                    ERROR("Planner did not succeed!");
                    return m_quantifier;
                }

                const std::list<PathSolution>& ucb_pf = result.solution_set;
                std::list<PathSolution> mean_pf = convertParetoFrontUCBToMean(result.solution_set);
                typename std::list<PathSolution>::const_iterator path_solution;
                switch (selector) {
                    case Selector::Aif:
                        path_solution = selectAif(ucb_pf, p_ev);
                        break;
                    case Selector::Uniform: {
                        path_solution = selectUniform(ucb_pf, mean_pf);
                        break;
                    }
                    case Selector::Topsis: {
                        path_solution = selectTopsis(ucb_pf, mean_pf);
                        break;
                    }
                    case Selector::Weights: {
                        path_solution = selectWeights(ucb_pf, mean_pf, p_ev);
                        break;
                    }
                }
                
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

        std::list<PathSolution> convertParetoFrontUCBToMean(const std::list<PathSolution>& ucb_pf) {
            std::list<PathSolution> mean_pf = ucb_pf;
            auto mean_pf_it = mean_pf.begin();
            for (auto ucb_pf_it = ucb_pf.begin(); ucb_pf_it != ucb_pf.end(); ++ucb_pf_it) {
                Plan plan(*ucb_pf_it, m_product, true);
                TrajectoryDistributionUpdaters<N> traj_updaters = getTrajectoryUpdaters(plan);
                auto mvn = GaussianEFE<N>::getCEQObservationDistribution(traj_updaters);
                TP::fromColMatrix<float, N>(TP::Stats::E(mvn), mean_pf_it->path_cost);
                ++mean_pf_it;
            }
            return mean_pf;
        }

        void initialize(const TP::DiscreteModel::State& init_state) {
            TP::Containers::SizedArray<TP::Node> init_aut_nodes(m_product->rank() - 1);
            for (uint32_t i=0; i<init_aut_nodes.size(); ++i) init_aut_nodes[i] = *(m_product->getAutomaton(i).getInitStates().begin());
            LOG("Init model state: " <<  m_product->getModel().getGenericNodeContainer()[init_state]);
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

        template <typename T>
        uint32_t getListIndex(const std::list<T>& list, std::list<T>::const_iterator target_it) {
            uint32_t ind = 0;
            for (auto it = list.begin(); it != list.end(); ++it) {
                if (it == target_it)
                    return ind;
                ++ind;
            }
            ASSERT(false, "Target iterator not found in list");
        }

    protected:
        std::shared_ptr<SymbolicProductGraph> m_product;
        std::shared_ptr<BehaviorHandlerType> m_behavior_handler;
        SymbolicProductGraph::node_t m_current_product_node;
        bool m_initialized = false;
        uint32_t m_n_samples;

        Quantifier<N> m_quantifier;

        std::shared_ptr<DataCollector<N>> m_data_collector;

        bool m_verbose;
};
}