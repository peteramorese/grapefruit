#pragma once

#include "TaskPlanner.h"

#include "Quantifier.h"

namespace PRL {

template <uint64_t N>
class Animator {
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

        using TrajectoryDistribution = TP::Stats::Distributions::FixedMultivariateNormal<N>;

        using Plan = TP::Planner::Plan<SearchProblem<N>>;

        struct Instance {
            Instance(const ParetoFrontResult& search_result_, 
                    uint32_t chosen_plan_index_, 
                    std::vector<TrajectoryDistribution>&& plan_distributions_, 
                    std::vector<typename BehaviorHandlerType::CostVector>&& ucb_pareto_points_)
                : search_result(search_result_)
                , chosen_plan_index(chosen_plan_index_)
                , plan_distributions(std::move(plan_distributions_))
                , ucb_pareto_points(std::move(ucb_pareto_points_))
            {}

            ParetoFrontResult search_result;
            uint32_t chosen_plan_index;
            std::vector<TrajectoryDistribution> plan_distributions;
            std::vector<typename BehaviorHandlerType::CostVector> ucb_pareto_points;
        };

    public:
        Animator(const std::shared_ptr<SymbolicProductGraph>& product, const TrajectoryDistribution& preference)
            : m_product(product)
            , m_preference(preference)
        {}

        void addInstance(const ParetoFrontResult& search_result, 
                uint32_t chosen_plan_index, 
                std::vector<TrajectoryDistribution>&& plan_distribution,
                std::vector<typename BehaviorHandlerType::CostVector>&& ucb_pareto_points) {
            m_instances.emplace_back(search_result, chosen_plan_index, std::move(plan_distribution), std::move(ucb_pareto_points));
        }

        void serialize(TP::Serializer& szr, const Quantifier<N>& quantifier) {
            static_assert(N == 2, "Does not support serialization of more than one cost behavior");

            YAML::Emitter& out = szr.get();
            out << YAML::Key << "PRL Preference Mean";
            out << YAML::Value << YAML::BeginSeq;
            // Cost (1) is x axis, reward (0) is y axis
            out << m_preference.mu(1) << m_preference.mu(0);
            out << YAML::EndSeq;
            out << YAML::Key << "PRL Preference Variance";
            out << YAML::Value << YAML::BeginSeq;
            // Cost (1) is x axis, reward (0) is y axis
            out << m_preference.Sigma(1, 1) << m_preference.Sigma(0, 0);
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

                uint32_t plan_i = 0;
                for (auto it = instance.search_result.solution_set.begin(); it != instance.search_result.solution_set.end(); ++it) {
                    std::string title = "Candidate Plan " + std::to_string(plan_i);
                    out << YAML::Key << title << YAML::Value << YAML::BeginMap;
                    Plan plan(*it, m_product, true);
                    plan.serialize(szr, title);
                    //out << YAML::EndMap;

                    out << YAML::Key << "Plan Mean" << YAML::Value << YAML::BeginSeq;
                    out << instance.plan_distributions[plan_i].mu(1) << instance.plan_distributions[plan_i].mu(0);
                    out << YAML::EndSeq;

                    out << YAML::Key << "Plan Variance" << YAML::Value << YAML::BeginSeq;
                    out << instance.plan_distributions[plan_i].Sigma(1, 1) << instance.plan_distributions[plan_i].Sigma(0, 0);
                    out << YAML::EndSeq;

                    out << YAML::Key << "Plan Pareto UCB" << YAML::Value << YAML::BeginSeq;
                    out << instance.ucb_pareto_points[plan_i][1] << instance.ucb_pareto_points[plan_i][0];
                    out << YAML::EndSeq;

                    out << YAML::EndMap;

                    ++plan_i;
                }

                out << YAML::Key << "Sample" << YAML::Value << YAML::BeginSeq;
                out << instance_cost_samples[instance_i][0] << YAML::EndSeq;

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