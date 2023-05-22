#pragma once

#include "TaskPlanner.h"
#include "Learner.h"

namespace PRL {

template <uint32_t COST_CRITERIA_M>
class RewardCostQuantifierSet {
    public:
        using TrajectoryDistribution = TP::Stats::Distributions::FixedMultivariateNormal<COST_CRITERIA_M + 1>;
    public:
        RewardCostQuantifierSet(const TrajectoryDistribution& preference) : m_preference(preference) {}

        void push_back(RewardCostQuantifier<COST_CRITERIA_M>&& quantifier) {
            m_data.push_back(std::move(quantifier));
        }

        void push_back(const RewardCostQuantifier<COST_CRITERIA_M>& quantifier) {
            m_data.push_back(quantifier);
        }

        const RewardCostQuantifier<COST_CRITERIA_M>& operator[](uint32_t i) const {return m_data[i];}
        const RewardCostQuantifier<COST_CRITERIA_M>& back() const {return m_data.back();}

        void serializeAverageBehavior(TP::Serializer& szr, const std::string& obj_0_label, const std::string& obj_1_label) {
            static_assert(COST_CRITERIA_M == 1, "Only supports serialization of Bi-objective problems");
            
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

            std::vector<std::pair<float, float>> pareto_points;
            pareto_points.reserve(m_data.size());
            for (uint32_t i = 0; i < m_data.size(); ++i) {
                pareto_points.push_back(std::make_pair(m_data[i].avgCostPerInstance(), m_data[i].avgRewardPerInstance()));
            }
            std::array<std::string, 2> axis_labels = {{obj_0_label, obj_1_label}};
            TP::Planner::ParetoFrontSerializer::serialize2DAxes(szr, axis_labels);
            TP::Planner::ParetoFrontSerializer::serialize2D(szr, pareto_points, "test_set");
        }

        void serializeInstances(TP::Serializer& szr, const std::string& obj_0_label, const std::string& obj_1_label) {
            static_assert(COST_CRITERIA_M == 1, "Only supports serialization of Bi-objective problems");
            
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

            uint32_t trial_i = 0;
            for (const auto& quantifier : m_data) {
                std::vector<std::pair<float, float>> pareto_points;
                pareto_points.reserve(m_data.size());

                const auto& instance_costs = quantifier.getInstanceCosts();
                const auto& instance_rewards = quantifier.getInstanceRewards();
                for (uint32_t i = 0; i < instance_costs.size(); ++i) {
                    pareto_points.push_back(std::make_pair(instance_costs[i][0], instance_rewards[i]));
                }
                TP::Planner::ParetoFrontSerializer::serialize2D(szr, pareto_points, "trial_" + std::to_string(trial_i++));
            }
            std::array<std::string, 2> axis_labels = {{obj_0_label, obj_1_label}};
            TP::Planner::ParetoFrontSerializer::serialize2DAxes(szr, axis_labels);
        }
    private:
        std::vector<RewardCostQuantifier<COST_CRITERIA_M>> m_data;
        TrajectoryDistribution m_preference;
};

}