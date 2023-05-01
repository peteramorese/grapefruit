#pragma once

#include "TaskPlanner.h"
#include "BehaviorHandler.h"

namespace PRL {

template <class SYMBOLIC_GRAPH_T, uint32_t COST_CRITERIA_M>
class TrueBehavior : public PRLStorage<Normal, TP::Containers::FixedArray<COST_CRITERIA_M, TP::Stats::Distributions::Normal>> {
    public:
        using CostDistributionArray = TP::Containers::FixedArray<COST_CRITERIA_M, TP::Stats::Distributions::Normal>;
    public:
        TrueBehavior(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product,
            uint32_t n_tasks, 
            const TP::Stats::Distributions::Normal& default_reward, 
            const CostDistributionArray& default_cost)
            : PRLStorage<Normal, Normal>(n_tasks, default_reward, default_cost)
        {}

        void setRewardDistribution(uint32_t task_i, const TP::Stats::Distributions::Normal& dist) {this->getTaskElement(task_i) = dist;}
        void setCostDistribution(TP::WideNode node, const TP::DiscreteModel::Action& action, const CostDistributionArray& dist_array) {
            this->getNAPElement(node, action) = dist_array;
        }

        TP::Containers::FixedArray<COST_CRITERIA_M + 1, float> sample(TP::WideNode src_node, TP::WideNode dst_node, const TP::DiscreteModel::Action& action) {
            TP::Containers::FixedArray<COST_CRITERIA_M + 1, float> s;
            s[0] = 0.0f;
            for (TP::DiscreteModel::ProductRank automaton_i = 0; automaton_i < m_product->rank() - 1; ++automaton_i) {
                if (!m_product->acc(src_node, automaton_i) && m_product->acc(dst_node, automaton_i)) {
                    // Accumulate reward for each task satisfied
                    s[0] += TP::RNG::nrand(this->getTaskElement(automaton_i));
                }
            }
            for (uint32_t i = 0; i < COST_CRITERIA_M; ++i) {
                s[i + 1] = TP::RNG::nrand(this->getNAPElement(src_node, action)[i]);
            }
            return s;
        }
    private:
        std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;

};

}