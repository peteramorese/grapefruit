#pragma once

#include <memory>

#include "TaskPlanner.h"

#include "HistoryNode.h"

namespace PRL {

    // Behavior Criterion
    /*
    struct BehaviorCriterion {
        void setToDefaultPrior();
        float getExpectation();
        float getVariance();
    };
    */

    template <uint32_t M>
    class CostBehaviorArray {
        public:
            using CostVector = TP::Containers::FixedArray<M, float>;
        public:
            CostBehaviorArray() = default;

            static constexpr uint32_t size() {return M;}

            CostVector getUCBVector(uint32_t state_visits) const {
                CostVector cv;
                for (uint32_t i = 0; i < M; ++i) {
                    cv[i] = m_ucb.get(TP::Stats::E(m_updaters.getEstimateNormal()), state_visits);
                }
                return cv;
            }

            TP::Containers::FixedArray<M, TP::Stats::Distributions::Normal> getEstimateDistributions() const {
                TP::Containers::FixedArray<M, TP::Stats::Distributions::Normal> distributions;
                for (uint32_t i = 0; i < M; ++i) {
                    distributions[i] = m_updaters[i].getEstimateNormal();
                }
                return distributions;
            }

            void pull() { m_ucb.pull(); }

        private:
            TP::Containers::FixedArray<M, TP::Stats::GaussianUpdater> m_updaters;
            TP::ML::UCB m_ucb;

    };

    struct RewardBehavior {
        RewardBehavior() = default;

        float getUCB(uint32_t n_tasks_completed) const {
            return ucb.get(TP::Stats::E(updater.getEstimateNormal()), n_tasks_completed);
        }

        float getEstimateMean() const {
            return TP::Stats::E(updater.getEstimateNormal());
        }

        TP::Stats::GaussianUpdater updater;
        TP::ML::UCB ucb;
    };

    template <class SYMBOLIC_GRAPH_T, uint32_t COST_CRITERIA_M>
    class BehaviorHandler {
        public:
            struct NodeActionPair {
                NodeActionPair(TP::WideNode node_, const TP::DiscreteModel::Action& action_) : node(node_), action(action_) {}
                TP::WideNode node;
                TP::DiscreteModel::Action action;

                bool operator==(const NodeActionPair& other) const {
                    return node == other.node && action == other.action;
                }
            };
            struct NodeActionPairHash {

                std::size_t operator()(const NodeActionPair& node_action_pair) const {
                    return std::hash<TP::WideNode>{}(node_action_pair.node) ^ (std::hash<TP::DiscreteModel::Action>{}(node_action_pair.action) << 1);
                }
            };

            using CostVector = TP::Containers::FixedArray<COST_CRITERIA_M + 1, float>;
        public:
            BehaviorHandler(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product, uint8_t completed_tasks_horizon) 
                : m_product(product)
                , m_reward_criteria(product.rank() - 1)
                , m_max_mean_reward(0.0f)
                , m_completed_tasks_horizon(completed_tasks_horizon)
            {
                update(0);
            }

            static constexpr std::size_t numBehaviors() noexcept {return COST_CRITERIA_M + 1;}
            
            void update(uint32_t n_completed_tasks) {
                // Increase the number of tasks completed to update the UCB index
                m_n_completed_tasks += n_completed_tasks;

                // Determine the new max reward for the planning instance
                m_max_mean_reward = 0.0f;
                for (const auto& reward_criterion : m_reward_criteria) {
                    float r = reward_criterion.getEstimateMean();
                    if (r > m_max_mean_reward) m_max_mean_reward = r;
                }
            }

            CostVector getCostVector(const TaskHistoryNode<TP::WideNode>& src_node, const TaskHistoryNode<TP::WideNode>& dst_node, const TP::DiscreteModel::Action& action) {
                typename CostBehaviorArray<COST_CRITERIA_M>::CostVector costs_only_cv = m_cost_behaviors[NodeActionPair(src_node.base_node, action)].getUCBVector();
                CostVector cv;
                cv[0] = 0.0f;
                for (uint32_t i=1; i<cv.size(); ++i) {
                    cv[i] = costs_only_cv[i - 1];
                }
                assignTransformedReward(src_node, dst_node, cv[0]);
                return cv;
            }

            inline float priceFunctionTransform(uint8_t n_completed_tasks) const {
                if (n_completed_tasks < m_completed_tasks_horizon) {
                    return -static_cast<float>(n_completed_tasks) * m_max_mean_reward;
                } else {
                    return -2.0f * static_cast<float>(m_reward_criteria.size()) * m_max_mean_reward;
                }
            }

            void assignTransformedReward(const TaskHistoryNode<TP::WideNode>& src_node, const TaskHistoryNode<TP::WideNode>& dst_node, float& reward) const {
                ASSERT(src_node.n_completed_tasks <= dst_node.n_completed_tasks, "Dst node has fewer completed tasks!");
                if (src_node.n_completed_tasks < dst_node.n_completed_tasks) {
                    LOG("found task completion transition diff: " << ((uint32_t)dst_node.n_completed_tasks - (uint32_t)src_node.n_completed_tasks));
                    for (TP::DiscreteModel::ProductRank automaton_i = 0; automaton_i < m_product->rank() - 1; ++automaton_i) {
                        if (!m_product->acc(src_node.base_node, automaton_i) && m_product->acc(dst_node.base_node, automaton_i)) {
                            // Accumulate reward for each task satisfied
                            reward += -m_reward_criteria[automaton_i].getEstimateMean();
                        }
                    
                                //+ priceFunctionTransform(src_node.n_completed_tasks) - priceFunctionTransform(dst_node.n_completed_tasks);
                    }
                    reward += priceFunctionTransform(src_node.n_completed_tasks) - priceFunctionTransform(dst_node.n_completed_tasks);
                    LOG("new reward: " << reward);
                }
            }

            inline const CostBehaviorArray<COST_CRITERIA_M>& getCostBehaviorArray(TP::WideNode node, const TP::DiscreteModel::Action& action) {
                return m_cost_behaviors[NodeActionPair(node, action)];
            }

            inline const RewardBehavior& getRewardBehavior(TP::DiscreteModel::ProductRank automaton_i) const {
                return m_reward_criteria[automaton_i];
            }
                
            inline const std::shared_ptr<SYMBOLIC_GRAPH_T>& getProduct() const {return m_product;}
            inline void setCompletedTasksHorizon(uint8_t horizon) const {m_completed_tasks_horizon = horizon;}
            inline uint8_t getCompletedTasksHorizon() const {return m_completed_tasks_horizon;}

            inline void visit(TP::WideNode node, const TP::DiscreteModel::Action& action) {
                ++m_state_visits[node];
                m_cost_behaviors[NodeActionPair(node, action)].pull();
            }

        private:
            std::unordered_map<NodeActionPair, CostBehaviorArray<COST_CRITERIA_M>, NodeActionPairHash> m_cost_behaviors;
            std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;
            TP::Containers::SizedArray<RewardBehavior> m_reward_criteria;
            float m_max_mean_reward = 0.0f;
            uint8_t m_completed_tasks_horizon = 1;
            
            // UCB parameters
            std::unordered_map<TP::WideNode, uint32_t> m_state_visits;
            uint32_t m_n_completed_tasks = 0;
    };
}