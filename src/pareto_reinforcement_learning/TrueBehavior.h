#pragma once

#include "TaskPlanner.h"
#include "BehaviorHandler.h"

namespace PRL {

template <class SYMBOLIC_GRAPH_T, uint64_t M>
class TrueBehavior : public Storage<TP::Stats::Distributions::FixedMultivariateNormalSampler<M>> {
    public:
        using Distribution = TP::Stats::Distributions::FixedMultivariateNormal<M>;
        using DistributionSampler = TP::Stats::Distributions::FixedMultivariateNormalSampler<M>;
    public:
        TrueBehavior(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product, const Distribution& default_cost_distribution)
            : Storage<DistributionSampler>(TP::Stats::Distributions::FixedMultivariateNormalSampler<M>(default_cost_distribution))
            , m_product(product)
        {}

        //void setRewardDistribution(uint32_t task_i, const TP::Stats::Distributions::Normal& dist) {this->getTaskElement(task_i) = dist;}
        //void setCostDistribution(TP::Node node, const TP::DiscreteModel::Action& action, const CostDistributionArray& dist_array) {
        //    this->getNAPElement(node, action) = dist_array;
        //}

        TP::Containers::FixedArray<M, float> sample(TP::WideNode src_node, TP::WideNode dst_node, const TP::DiscreteModel::Action& action) {
            TP::Containers::FixedArray<M, float> rectified_sample;
            TP::Node src_model_node = m_product->getUnwrappedNode(src_node).ts_node;
            Eigen::Matrix<float, M, 1> sample = TP::RNG::mvnrand(this->getElement(src_model_node, action));
            for (uint32_t i = 0; i < M; ++i) {
                rectified_sample[i] = TP::max(sample(i, 1), 0.0f);
            }
            return rectified_sample;
        }

        //void print() const {
        //    LOG("Node action pair distributions:");
        //    for (const auto&[nap, cost_array] : this->m_node_action_pair_elements) {
        //        TP::DiscreteModel::State s = m_product->getModel().getGenericNodeContainer()[m_product->getUnwrappedNode(nap.node).ts_node];
        //        PRINT_NAMED("State: " << s.to_str() << " action: " << nap.action, "Cost mean: " << cost_array[0].mu << " variance: " << cost_array[0].sigma_2);
        //    }
        //    LOG("Task distributions:");
        //    TP::DiscreteModel::ProductRank task_i;
        //    for (const auto& task_element : this->m_task_elements) {
        //        PRINT_NAMED("Task: " << (uint32_t)task_i, "Reward mean: " << task_element.mu << " variance: " << task_element.sigma_2);
        //    }
        //}

        //void compare(const RewardCostBehaviorHandler<SYMBOLIC_GRAPH_T, M>& behavior_handler) const {
        //    for (const auto&[nap, element] : this->m_node_action_pair_elements) {
        //        TP::DiscreteModel::State s = m_product->getModel().getGenericNodeContainer()[nap.node];

        //        float true_dist_mu = element[0].mu;
        //        float true_dist_sigma2 = element[0].sigma_2;

        //        auto cba = behavior_handler.lookupNAElement(nap.node, nap.action);
        //        auto estimate_dist = cba.getEstimateDistributions()[0];
        //        float estimate_dist_mu = estimate_dist.mu;
        //        float estimate_dist_sigma2 = estimate_dist.sigma_2;

        //        PRINT_NAMED("State: " << s.to_str() << " Action: " << nap.action, 
        //            "Mu: (est: " << estimate_dist_mu << " act: " << true_dist_mu << "), Var: (est: " << estimate_dist_sigma2 << " act: " << true_dist_sigma2 <<"), N samples: " << cba.nSamples());
        //    }
        //}
    private:
        std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;

};

}