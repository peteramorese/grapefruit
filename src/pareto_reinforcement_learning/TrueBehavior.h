#pragma once

#include "TaskPlanner.h"
#include "BehaviorHandler.h"

namespace PRL {

template <class SYMBOLIC_GRAPH_T, uint32_t M>
class TrueBehavior : public Storage<TP::Stats::Distributions::Normal, TP::Containers::FixedArray<COST_CRITERIA_M, TP::Stats::Distributions::Normal>> {
    public:
        using CostDistributionArray = TP::Containers::FixedArray<COST_CRITERIA_M, TP::Stats::Distributions::Normal>;
    public:
        TrueBehavior(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product,
            const CostDistributionArray& default_cost)
            : TaskNodeActionStorage<TP::Stats::Distributions::Normal, TP::Containers::FixedArray<COST_CRITERIA_M, TP::Stats::Distributions::Normal>>(n_tasks, default_reward, default_cost)
            , m_product(product)
        {}

        //void setRewardDistribution(uint32_t task_i, const TP::Stats::Distributions::Normal& dist) {this->getTaskElement(task_i) = dist;}
        //void setCostDistribution(TP::Node node, const TP::DiscreteModel::Action& action, const CostDistributionArray& dist_array) {
        //    this->getNAPElement(node, action) = dist_array;
        //}

        BehaviorSample<COST_CRITERIA_M> sample(TP::WideNode src_node, TP::WideNode dst_node, const TP::DiscreteModel::Action& action) {
            BehaviorSample<COST_CRITERIA_M> s(m_product->rank() - 1);
            for (TP::DiscreteModel::ProductRank task_i = 0; task_i < m_product->rank() - 1; ++task_i) {
                if (!m_product->acc(src_node, task_i) && m_product->acc(dst_node, task_i)) {
                    // Accumulate reward for each task satisfied
                    s.addReward(task_i, TP::max(TP::RNG::nrand(this->getTaskElement(task_i)), 0.0f));
                }
            }
            for (uint32_t i = 0; i < COST_CRITERIA_M; ++i) {
                //LOG("Cost sample for src_node: " << src_node << " action: " << action << " mean: " << this->getNAPElement(src_node, action)[i].mu);
                TP::Node src_model_node = m_product->getUnwrappedNode(src_node).ts_node;
                s.cost_sample[i] = TP::max(TP::RNG::nrand(this->getNAElement(src_model_node, action)[i]), 0.0f);
            }
            return s;
        }

        void print() const {
            LOG("Node action pair distributions:");
            for (const auto&[nap, cost_array] : this->m_node_action_pair_elements) {
                TP::DiscreteModel::State s = m_product->getModel().getGenericNodeContainer()[m_product->getUnwrappedNode(nap.node).ts_node];
                PRINT_NAMED("State: " << s.to_str() << " action: " << nap.action, "Cost mean: " << cost_array[0].mu << " variance: " << cost_array[0].sigma_2);
            }
            LOG("Task distributions:");
            TP::DiscreteModel::ProductRank task_i;
            for (const auto& task_element : this->m_task_elements) {
                PRINT_NAMED("Task: " << (uint32_t)task_i, "Reward mean: " << task_element.mu << " variance: " << task_element.sigma_2);
            }
        }

        void compare(const RewardCostBehaviorHandler<SYMBOLIC_GRAPH_T, COST_CRITERIA_M>& behavior_handler) const {
            for (const auto&[nap, element] : this->m_node_action_pair_elements) {
                TP::DiscreteModel::State s = m_product->getModel().getGenericNodeContainer()[nap.node];

                float true_dist_mu = element[0].mu;
                float true_dist_sigma2 = element[0].sigma_2;

                auto cba = behavior_handler.lookupNAElement(nap.node, nap.action);
                auto estimate_dist = cba.getEstimateDistributions()[0];
                float estimate_dist_mu = estimate_dist.mu;
                float estimate_dist_sigma2 = estimate_dist.sigma_2;

                PRINT_NAMED("State: " << s.to_str() << " Action: " << nap.action, 
                    "Mu: (est: " << estimate_dist_mu << " act: " << true_dist_mu << "), Var: (est: " << estimate_dist_sigma2 << " act: " << true_dist_sigma2 <<"), N samples: " << cba.nSamples());
            }
        }
    private:
        std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;

};

}