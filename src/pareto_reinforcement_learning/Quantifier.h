#pragma once

#include "TaskPlanner.h"

namespace PRL {

template <uint32_t M>
struct Quantifier {
    public:
        TP::Containers::FixedArray<M, float> cumulative_cost;
        uint32_t steps = 0u;
        uint32_t instances = 0u;
        uint32_t max_instances = 0u;
    public:
        CostQuantifier() {
            for (uint32_t i = 0; i < M; ++i) {
                cumulative_cost[i] = 0.0f;
                m_cost_sample_buffer[i] = 0.0f;
            }
        }

        void addSample(const TP::Containers::FixedArray<M, float>& sample) {
            cumulative_cost += sample;
            m_cost_sample_buffer += sample;
            ++steps;
        }

        virtual void finishInstance() {
            ++instances;
            m_instance_costs.push_back(m_cost_sample_buffer);
            for (auto& v : m_cost_sample_buffer)
                v = 0.0f;
        }

        float avgCostPerInstance(uint32_t cost_criteria_i = 0) const {return cumulative_cost[cost_criteria_i] / static_cast<float>(instances);}
        const std::vector<TP::Containers::FixedArray<M, float>>& getInstanceCosts() const {return m_instance_costs;}

    private:
        std::vector<TP::Containers::FixedArray<M, float>> m_instance_costs;
        TP::Containers::FixedArray<M, float> m_cost_sample_buffer;
};

}