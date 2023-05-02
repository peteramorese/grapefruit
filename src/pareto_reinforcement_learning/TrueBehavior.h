#pragma once

#include "TaskPlanner.h"
#include "BehaviorHandler.h"

namespace PRL {


template <uint32_t COST_CRITERIA_M>
struct BehaviorSample {
    public:
        TP::Containers::FixedArray<COST_CRITERIA_M, float> cost_sample;

    public:
        BehaviorSample(TP::DiscreteModel::ProductRank n_tasks)
            : m_rewards(n_tasks, std::make_pair(false, 0.0f))
        {}

        inline void addReward(TP::DiscreteModel::ProductRank task_i, float r) {
            m_rewards[task_i].first = true;
            m_rewards[task_i].second = r; 
            m_has_rewards = true;
        }
        inline bool hasRewards() const {return m_has_rewards;}
        inline const std::vector<std::pair<bool, float>>& getRewards() const {return m_rewards;}

    private:
        std::vector<std::pair<bool, float>> m_rewards;
        bool m_has_rewards = false;
};

template <class SYMBOLIC_GRAPH_T, uint32_t COST_CRITERIA_M>
class TrueBehavior : public PRLStorage<TP::Stats::Distributions::Normal, TP::Containers::FixedArray<COST_CRITERIA_M, TP::Stats::Distributions::Normal>> {
    public:
        using CostDistributionArray = TP::Containers::FixedArray<COST_CRITERIA_M, TP::Stats::Distributions::Normal>;
    public:
        TrueBehavior(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product,
            uint32_t n_tasks, 
            const TP::Stats::Distributions::Normal& default_reward, 
            const CostDistributionArray& default_cost)
            : PRLStorage<TP::Stats::Distributions::Normal, TP::Containers::FixedArray<COST_CRITERIA_M, TP::Stats::Distributions::Normal>>(n_tasks, default_reward, default_cost)
        {}

        void setRewardDistribution(uint32_t task_i, const TP::Stats::Distributions::Normal& dist) {this->getTaskElement(task_i) = dist;}
        void setCostDistribution(TP::WideNode node, const TP::DiscreteModel::Action& action, const CostDistributionArray& dist_array) {
            this->getNAPElement(node, action) = dist_array;
        }

        BehaviorSample<COST_CRITERIA_M> sample(TP::WideNode src_node, TP::WideNode dst_node, const TP::DiscreteModel::Action& action) {
            BehaviorSample<COST_CRITERIA_M> s;
            for (TP::DiscreteModel::ProductRank task_i = 0; task_i < m_product->rank() - 1; ++task_i) {
                if (!m_product->acc(src_node, task_i) && m_product->acc(dst_node, task_i)) {
                    // Accumulate reward for each task satisfied
                    s.addReward(task_i, TP::max(TP::RNG::nrand(this->getTaskElement(task_i)), 0.0f));
                }
            }
            for (uint32_t i = 0; i < COST_CRITERIA_M; ++i) {
                s.cost_sample[i] = TP::max(TP::RNG::nrand(this->getNAPElement(src_node, action)[i]), 0.0f);
            }
            return s;
        }
    private:
        std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;

};

}