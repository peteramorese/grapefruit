#pragma once

#include "TaskPlanner.h"

namespace PRL {

template <uint32_t COST_CRITERIA_M>
struct PRLQuantifier {
    public:
        float cumulative_reward = 0.0f;
        TP::Containers::FixedArray<COST_CRITERIA_M, float> cumulative_cost;
        uint32_t steps = 0u;
        uint32_t decision_instances = 0u;
        uint32_t max_decision_instances = 0u;

    public:
        PRLQuantifier() {
            for (uint32_t i = 0; i < COST_CRITERIA_M; ++i) cumulative_cost[i] = 0.0f;
        }
        void addSample(const BehaviorSample<COST_CRITERIA_M>& sample) {
            float total_reward_this_step = 0.0f;
            for (auto[contains, r] : sample.getRewards()) {
                if (contains) 
                    total_reward_this_step += r;
            }
            cumulative_reward += total_reward_this_step;
            cumulative_cost += sample.cost_sample;
            m_sample_buffer[0] += total_reward_this_step;
            for (uint32_t i = 0; i < COST_CRITERIA_M; ++i) {
                m_sample_buffer[i + 1] += sample.cost_sample[i];
            }
            ++steps;
        }
        void finishDI() {
            ++decision_instances;
            m_di_behaviors.push_back(m_sample_buffer);
            for (auto& v : m_sample_buffer)
                v = 0.0f;
        }
        float avgRewardPerDI() const {return cumulative_reward / static_cast<float>(decision_instances);}
        float avgCostPerDI(uint32_t cost_criteria_i = 0) const {return cumulative_cost[cost_criteria_i] / static_cast<float>(decision_instances);}
        const std::vector<TP::Containers::FixedArray<COST_CRITERIA_M + 1, float>>& getDIBehaviors() const {return m_di_behaviors;}
        
    private:
        std::vector<TP::Containers::FixedArray<COST_CRITERIA_M + 1, float>> m_di_behaviors;
        TP::Containers::FixedArray<COST_CRITERIA_M + 1, float> m_sample_buffer;
};

}