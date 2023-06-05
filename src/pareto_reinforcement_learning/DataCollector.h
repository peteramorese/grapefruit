#pragma once

#include "TaskPlanner.h"

#include "Quantifier.h"

namespace PRL {

template <uint64_t N>
class DataCollector {
    public:
        using SymbolicProductGraph = TP::DiscreteModel::SymbolicProductAutomaton<
            TP::DiscreteModel::TransitionSystem, 
            TP::FormalMethods::DFA, 
            TP::DiscreteModel::ModelEdgeInheritor<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA>>;

        using BehaviorHandlerType = BehaviorHandler<SymbolicProductGraph, N>;

        using PathSolution = TP::GraphSearch::PathSolution<
            typename SearchProblem<N>::node_t, 
            typename SearchProblem<N>::edge_t>;

        //using ParetoFrontResult = TP::GraphSearch::MultiObjectiveSearchResult<
        //    typename SearchProblem<N>::node_t, 
        //    typename SearchProblem<N>::edge_t, 
        //    typename SearchProblem<N>::cost_t>;

        using TrajectoryDistribution = TP::Stats::Distributions::FixedMultivariateNormal<N>;

        using Plan = TP::Planner::Plan<SearchProblem<N>>;

        struct Instance {
            Instance(const std::vector<PathSolution>& paths_, 
                    const TP::ParetoFront<typename SearchProblem<N>::cost_t>& ucb_pf_,
                    std::vector<TrajectoryDistribution>&& plan_distributions_,
                    uint32_t chosen_plan_index_)
                : paths(paths_)
                , ucb_pf(ucb_pf_)
                , plan_distributions(std::move(plan_distributions_))
                , chosen_plan_index(chosen_plan_index_)
            {}

            std::vector<PathSolution> paths;
            TP::ParetoFront<typename SearchProblem<N>::cost_t> ucb_pf;
            std::vector<TrajectoryDistribution> plan_distributions;
            uint32_t chosen_plan_index;
        };

    public:
        DataCollector(const std::shared_ptr<SymbolicProductGraph>& product, const TrajectoryDistribution& preference)
            : m_product(product)
            , m_preference(preference)
        {}

        void addInstance(const std::vector<PathSolution>& paths_, 
                    const TP::ParetoFront<typename SearchProblem<N>::cost_t>& ucb_pf_,
                    std::vector<TrajectoryDistribution>&& plan_distributions_,
                    uint32_t chosen_plan_index_) {
            m_instances.emplace_back(paths_, ucb_pf_, std::move(plan_distributions_), chosen_plan_index_);
        }

        void serialize(TP::Serializer& szr, const Quantifier<N>& quantifier) {
            static_assert(N == 2, "Does not support serialization of more than two cost objectives");

            YAML::Emitter& out = szr.get();
            out << YAML::Key << "PRL Preference Mean";
            out << YAML::Value << YAML::BeginSeq;
            // Cost (1) is x axis, reward (0) is y axis
            out << m_preference.mu(0) << m_preference.mu(1);
            out << YAML::EndSeq;
            out << YAML::Key << "PRL Preference Covariance";
            out << YAML::Value << YAML::BeginSeq;
            // Cost (1) is x axis, reward (0) is y axis
            out << m_preference.Sigma(0, 0) << m_preference.Sigma(0, 1) << m_preference.Sigma(1, 1);
            out << YAML::EndSeq;
            out << YAML::Key << "Instances" << YAML::Value << m_instances.size();

            uint32_t instance_i = 0;

            const auto& instance_cost_samples = quantifier.getInstanceCosts();
            ASSERT(instance_cost_samples.size() == m_instances.size(), "Number of instances in quantifier does not match");
            LOG("Serializing " << m_instances.size() << " instances");
            for (const auto& instance : m_instances) {
                out << YAML::Key << "Instance " + std::to_string(instance_i); 
                out << YAML::Value << YAML::BeginMap;

                out << YAML::Key << "Chosen Plan" << YAML::Value << "Candidate Plan " + std::to_string(instance.chosen_plan_index);

                //out << YAML::Key << "Chosen Plan" << YAML::Value << YAML::BeginMap;
                //auto chosen_it = std::next(instance.search_result.solution_set.begin(), instance.chosen_plan_index);
                //Plan plan(*chosen_it, m_product, true);
                //plan.serialize(szr, "Chosen Plan");
                //out << YAML::EndMap;

                for (uint32_t i = 0; i < instance.paths.size(); ++i) {
                    std::string title = "Candidate Plan " + std::to_string(i);
                    out << YAML::Key << title << YAML::Value << YAML::BeginMap;
                    Plan plan(instance.paths[i], instance.ucb_pf[i], m_product, true);
                    plan.serialize(szr, title);
                    //out << YAML::EndMap;

                    out << YAML::Key << "Plan Mean Mean" << YAML::Value << YAML::BeginSeq;
                    out << instance.plan_distributions[i].mu(0) << instance.plan_distributions[i].mu(1);
                    out << YAML::EndSeq;

                    out << YAML::Key << "Plan Mean Covariance" << YAML::Value << YAML::BeginSeq;
                    out << instance.plan_distributions[i].Sigma(0, 0) << instance.plan_distributions[i].Sigma(0, 1) << instance.plan_distributions[i].Sigma(1, 1);
                    out << YAML::EndSeq;

                    out << YAML::Key << "Plan Pareto UCB" << YAML::Value << YAML::BeginSeq;
                    out << instance.ucb_pf[i][0] << instance.ucb_pf[i][1];
                    out << YAML::EndSeq;

                    out << YAML::EndMap;
                }

                out << YAML::Key << "Sample" << YAML::Value << YAML::BeginSeq;
                out << instance_cost_samples[instance_i][0] << instance_cost_samples[instance_i][1] << YAML::EndSeq;

                out << YAML::EndMap;
                ++instance_i;
            }
        }

    private:
        std::shared_ptr<SymbolicProductGraph> m_product;
        std::vector<Instance> m_instances;
        TrajectoryDistribution m_preference;
};

}