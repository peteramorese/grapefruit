#pragma once

#include "TaskPlanner.h"

namespace PRL {

template <class BEHAVIOR_HANDLER_T>
class Animator {
    public:
        using ParetoFrontResult = TP::GraphSearch::MultiObjectiveSearchResult<
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::node_t, 
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::edge_t, 
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::cost_t>;

        using SymbolicProductGraph = TP::DiscreteModel::SymbolicProductAutomaton<
            TP::DiscreteModel::TransitionSystem, 
            TP::FormalMethods::DFA, 
            TP::DiscreteModel::ModelEdgeInheritor<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA>>;

        using PathSolution = TP::GraphSearch::PathSolution<
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::node_t, 
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::edge_t, 
            typename PRLSearchProblem<BEHAVIOR_HANDLER_T>::cost_t>;

        using TrajectoryDistribution = TP::Stats::Distributions::FixedMultivariateNormal<BEHAVIOR_HANDLER_T::numBehaviors()>;

        struct Instance {
            Instance(const ParetoFrontResult& search_result_, uint32_t chosen_plan_index_, std::vector<TrajectoryDistribution>&& plan_distributions_)
                : search_result(search_result_)
                , chosen_plan_index(chosen_plan_index_)
                , plan_distributions(std::move(plan_distributions_))
            {}

            ParetoFrontResult search_result;
            uint32_t chosen_plan_index;
            std::vector<TrajectoryDistribution> plan_distributions;
        };

        using Plan = TP::Planner::Plan<PRLSearchProblem<BEHAVIOR_HANDLER_T>>;

    public:
        Animator(const std::shared_ptr<SymbolicProductGraph>& product, const TrajectoryDistribution& preference)
            : m_product(product)
            , m_preference(preference)
        {}

        void addInstance(const ParetoFrontResult& search_result, uint32_t chosen_plan_index, std::vector<TrajectoryDistribution>&& plan_distribution) {
            m_instances.emplace_back(search_result, chosen_plan_index, std::move(plan_distribution));
        }

        void serialize(TP::Serializer& szr) {
            static_assert(BEHAVIOR_HANDLER_T::numBehaviors() == 2, "Does not support serialization of more than one cost behavior");

            YAML::Emitter& out = szr.get();
            out << YAML::Key << "PRL Preference Mean";
            out << YAML::Value << YAML::BeginSeq;
            // Cost (1) is x axis, reward (0) is y axis
            out << m_preference.mu(1) << m_preference.mu(0);
            out << YAML::EndSeq;
            out << YAML::Key << "PRL Preference Variance";
            out << YAML::Value << YAML::BeginSeq;
            // Cost (1) is x axis, reward (0) is y axis
            out << m_preference.covariance(1, 1) << m_preference.covariance(0, 0);
            out << YAML::EndSeq;
            out << YAML::Key << "Instances" << YAML::Value << m_instances.size();

            uint32_t instance_i = 0;
            for (const auto& instance : m_instances) {
                out << YAML::Key << "Instance " + std::to_string(instance_i++); 
                out << YAML::Value << YAML::BeginMap;

                out << YAML::Key << "Chosen Plan" << YAML::Value << "Candidate Plan " + std::to_string(instance.chosen_plan_index);

                //out << YAML::Key << "Chosen Plan" << YAML::Value << YAML::BeginMap;
                //auto chosen_it = std::next(instance.search_result.solution_set.begin(), instance.chosen_plan_index);
                //Plan plan(*chosen_it, m_product, true);
                //plan.serialize(szr, "Chosen Plan");
                //out << YAML::EndMap;

                LOG("num plan distributions: " << instance.plan_distributions.size());
                uint32_t plan_i = 0;
                for (auto it = instance.search_result.solution_set.begin(); it != instance.search_result.solution_set.end(); ++it) {
                    std::string title = "Candidate Plan " + std::to_string(plan_i);
                    LOG("SERIALIZNG plan title " << title);
                    out << YAML::Key << title << YAML::Value << YAML::BeginMap;
                    Plan plan(*it, m_product, true);
                    plan.serialize(szr, title);
                    //out << YAML::EndMap;

                    out << YAML::Key << "Plan Mean" << YAML::Value << YAML::BeginSeq;
                    out << instance.plan_distributions[plan_i].mu(1) << instance.plan_distributions[plan_i].mu(0);
                    out << YAML::EndSeq;

                    out << YAML::Key << "Plan Variance" << YAML::Value << YAML::BeginSeq;
                    out << instance.plan_distributions[plan_i].covariance(1, 1) << instance.plan_distributions[plan_i].covariance(0, 0);
                    out << YAML::EndSeq;

                    out << YAML::EndMap;

                    ++plan_i;
                }
                out << YAML::EndMap;
            }
        }

    private:
        std::shared_ptr<SymbolicProductGraph> m_product;
        std::vector<Instance> m_instances;
        TrajectoryDistribution m_preference;
};

}