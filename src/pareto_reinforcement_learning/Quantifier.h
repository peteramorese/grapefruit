#pragma once

#include "TaskPlanner.h"

#include "Regret.h"

namespace PRL {

template <uint64_t N>
struct Quantifier {
    public:
        TP::Containers::FixedArray<N, float> cumulative_cost;
        uint32_t steps = 0u;
        uint32_t instances = 0u;
        uint32_t max_instances = 0u;
        float cumulative_regret = 0.0f;
    public:
        Quantifier() {
            for (uint32_t i = 0; i < N; ++i) {
                cumulative_cost[i] = 0.0f;
                m_cost_sample_buffer[i] = 0.0f;
            }
        }

        void addSample(const TP::Containers::FixedArray<N, float>& sample) {
            cumulative_cost += sample;
            m_cost_sample_buffer += sample;
            ++steps;
        }

        virtual void finishInstance(typename SymbolicProductGraph::node_t starting_node) {
            ++instances;
            m_instance_costs.push_back(m_cost_sample_buffer);
            for (auto& v : m_cost_sample_buffer)
                v = 0.0f;

            if (m_regret) {
                float instance_regret = m_regret->getRegret(starting_node, m_cost_sample_buffer);
                cumulative_regret += instance_regret;
                m_instance_regrets.push_back(instance_regret);
            }
        }

        TP::Containers::FixedArray<N, float> avgCostPerInstance() const {
            TP::Containers::FixedArray<N, float> avg;
            for (uint32_t i = 0; i < N; ++i)
                avg[i] = cumulative_cost[i] / static_cast<float>(instances);
            return avg;
        }

        const std::vector<TP::Containers::FixedArray<N, float>>& getInstanceCosts() const {return m_instance_costs;}
        //const std::vector<float>& getInstanceRegrets() const {return m_instance_regrets;}

    private:
        std::vector<TP::Containers::FixedArray<N, float>> m_instance_costs;
        std::vector<float> m_instance_regrets;
        TP::Containers::FixedArray<N, float> m_cost_sample_buffer;
        std::shared_ptr<Regret<SymbolicProductGraph, N>> m_regret;
};

}