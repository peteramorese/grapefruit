#pragma once

#include "TaskPlanner.h"

namespace PRL {

template <uint32_t M>
struct CostQuantifier {
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

template <uint32_t COST_CRITERIA_M>
struct RewardCostQuantifier : CostQuantifier<COST_CRITERIA_M> {
    public:
        float cumulative_reward = 0.0f;

    public:
        void addSample(const BehaviorSample<COST_CRITERIA_M>& sample) {
            CostQuantifier<COST_CRITERIA_M>::addSample(sample.cost_sample);
            float total_reward_this_step = 0.0f;
            for (auto[contains, r] : sample.getRewards()) {
                if (contains) 
                    total_reward_this_step += r;
            }
            m_reward_sample_buffer += total_reward_this_step;
            cumulative_reward += total_reward_this_step;
        }

        virtual void finishInstance() override {
            CostQuantifier<COST_CRITERIA_M>::finishInstance();
            m_instance_rewards.push_back(m_reward_sample_buffer);
            m_reward_sample_buffer = 0.0f;
        }

        float avgRewardPerInstance() const {return cumulative_reward / static_cast<float>(this->instances);}
        const std::vector<float>& getInstanceRewards() const {return m_instance_rewards;}

    public:
        std::vector<float> m_instance_rewards;
        float m_reward_sample_buffer = 0.0f;

        
};

}