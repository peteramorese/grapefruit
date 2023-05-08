#pragma once

#include "TaskPlanner.h"
#include "PRL.h"

namespace PRL {

template <uint32_t COST_CRITERIA_M>
class QuantifierSet {
    public:
        using TrajectoryDistribution = TP::Stats::Distributions::FixedMultivariateNormal<COST_CRITERIA_M + 1>;
    public:
        QuantifierSet(const TrajectoryDistribution& preference);
        void push_back(PRLQuantifier<COST_CRITERIA_M>&& quantifier) {
            m_data.push_back(std::move(quantifier));
        }
        void push_back(const PRLQuantifier<COST_CRITERIA_M>& quantifier) {
            m_data.push_back(quantifier);
        }
        void serialize(TP::Serializer& szr, const std::string& obj_0_label, const std::string& obj_1_label) {
            static_assert(COST_CRITERIA_M == 1, "Only supports serialization of Bi-objective problems");
            YAML::Emitter& out = szr.get();
            std::vector<std::pair<float, float>> pareto_points;
            pareto_points.reserve(m_data.size());
            for (uint32_t i = 0; i < m_data.size(); ++i) {
                pareto_points.push_back(std::make_pair(m_data[i].avgCostPerDI(), m_data[i].avgRewardPerDI()));
            }
            std::array<std::string, 2> axis_labels = {{obj_0_label, obj_1_label}};
            TP::ParetoFrontSerializer::serialize2DParetoFront(szr, pareto_points, axis_labels);
        }
    private:
        std::vector<PRLQuantifier<COST_CRITERIA_M>> m_data;
};

}