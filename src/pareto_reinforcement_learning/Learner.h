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
            typename SearchProblem<N>::edge_t>;

        using SearchResult = TP::GraphSearch::MultiObjectiveSearchResult<
            typename SearchProblem<N>::node_t, 
            typename SearchProblem<N>::edge_t, 
            typename SearchProblem<N>::cost_t>;

        using PreferenceDistribution = TP::Stats::Distributions::FixedMultivariateNormal<N>;

        using Plan = TP::Planner::Plan<SearchProblem<N>>;

        using ParetoFront = TP::ParetoFront<typename SearchProblem<N>::cost_t>;

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

        uint32_t selectAif(const SearchResult& search_result, const PreferenceDistribution& p_ev) {
            uint32_t min_ind = 0;
            float min_efe = 0.0f;

            for (uint32_t i = 0; i < search_result.solution_set.size(); ++i) {
                Plan plan(search_result.solution_set[i], search_result.pf[i], m_product, true);
                TrajectoryDistributionUpdaters<N> traj_updaters = getTrajectoryUpdaters(plan);
                if (m_verbose) {
                    //std::string
                    PRINT_NAMED("    Solution candidate ", i);
                    for (uint32_t d = 0; d < N; ++d) {
                        PRINT("      [Cost ucb (obj " << d <<")]:" << search_result.pf[i][d]);
                    }
                }

                float info_gain;
                float efe = GaussianEFE<N>::calculate(traj_updaters, p_ev, m_n_samples, &info_gain);
                LOG("-> efe: " << efe << " (info gain: " << info_gain << ")");
                if (i > 0) {
                    if (efe < min_efe) {
                        min_efe = efe;
                        min_ind = i;
                    }
                } else {
                    min_efe = efe;
                }
            }

            return min_ind;
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
                log("Selection Phase (2)");

                const ParetoFront& ucb_pf = result.pf;
                ParetoFront mean_pf = convertParetoFrontUCBToMean(result);
                //typename std::list<PathSolution>::const_iterator path_solution;
                std::size_t selected_ind = 0;
                switch (selector) {
                    case Selector::Aif:
                        selected_ind = selectAif(result, p_ev);
                        break;
                    case Selector::Uniform: {
                        selected_ind = TP::ParetoSelector<typename SearchProblem<N>::cost_t>::uniformRandom(mean_pf);
                        break;
                    }
                    case Selector::Topsis: {
                        selected_ind = TP::ParetoSelector<typename SearchProblem<N>::cost_t>::TOPSIS(mean_pf);
                        break;
                    }
                    case Selector::Weights: {

                        TP::Containers::FixedArray<N, float> weights;
                        float max_val = 0.0f;
                        for (uint64_t d = 0; d < N; ++d) {
                            weights[d] = p_ev.mu(d);
                            if (weights[d] > max_val)
                                max_val = weights[d];
                        }
                        ASSERT(max_val > 0.0f, "At least one weight must be greater than zero");
                        selected_ind = TP::ParetoSelector<typename SearchProblem<N>::cost_t>::scalarWeights(mean_pf, weights);
                        break;
                    }
                    default:
                        selected_ind = 0;
                        ASSERT(false, "Unknown selector type");
                }


                log("Chosen solution: " + std::to_string(selected_ind), true);

                // Hold the trajectory distributions for the animation
                std::vector<TP::Stats::Distributions::FixedMultivariateNormal<N>> estimate_traj_distributions;
                estimate_traj_distributions.reserve(result.solution_set.size());

                auto pf_it = result.pf.begin();
                for (const auto& pt : result.solution_set) {
                    Plan plan(pt, *pf_it++, m_product, true);
                    TrajectoryDistributionUpdaters<N> traj_updaters = getTrajectoryUpdaters(plan);
                    auto estimate_traj_distribution = GaussianEFE<N>::getCEQObservationDistribution(traj_updaters);
                    estimate_traj_distributions.push_back(estimate_traj_distribution);
                }
                log("Selected solution: " + std::to_string(selected_ind), true);

                // Add to animator
                if (m_data_collector)
                    m_data_collector->addInstance(result.solution_set, ucb_pf, std::move(estimate_traj_distributions), selected_ind);
                
                Plan plan(result.solution_set[selected_ind], result.pf[selected_ind], m_product, true);
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

        ParetoFront convertParetoFrontUCBToMean(const SearchResult& search_result) {
            ParetoFront mean_pf = search_result.pf;
            for (std::size_t i = 0; i < search_result.solution_set.size(); ++i) {
                Plan plan(search_result.solution_set[i], search_result.pf[i], m_product, true);
                TrajectoryDistributionUpdaters<N> traj_updaters = getTrajectoryUpdaters(plan);
                auto mvn = GaussianEFE<N>::getCEQObservationDistribution(traj_updaters);
                TP::fromColMatrix<float, N>(TP::Stats::E(mvn), mean_pf[i]);
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