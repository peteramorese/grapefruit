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
            CostBehaviorArray(float confidence)
                : m_ucb(confidence)
            {}

            static constexpr uint32_t size() {return M;}

            CostVector getRectifiedUCBVector(uint32_t state_visits) const {
                CostVector cv;
                for (uint32_t i = 0; i < M; ++i) {
                    cv[i] = m_ucb.getRectifiedCost(TP::Stats::E(m_updaters[i].getEstimateNormal()), state_visits);
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

            void pull(const CostVector& sample) { 
                m_ucb.pull(); 
                for (uint32_t i = 0; i < M; ++i) {
                    m_updaters[i].update(sample[i]);
                }
            }

            inline uint32_t nSamples() const {return m_updaters[0].nSamples();}

        private:
            TP::Containers::FixedArray<M, TP::Stats::GaussianUpdater> m_updaters;
            TP::ML::UCB m_ucb;

    };

    struct RewardBehavior {
        RewardBehavior(float confidence) 
            : ucb(confidence)
        {}

        float getUCB(uint32_t n_tasks_completed) const {
            return ucb.getReward(getEstimateMean(), n_tasks_completed);
        }

        float getEstimateMean() const {
            return TP::Stats::E(updater.getEstimateNormal());
        }

        void pull(float sample) {
            ucb.pull();
            updater.update(sample);
        }

        TP::Stats::GaussianUpdater updater;
        TP::ML::UCB ucb;
    };

    template <class TASK_T, class NAP_T>
    class PRLStorage {
        protected:
            struct NodeActionPair {
                NodeActionPair(TP::Node node_, const TP::DiscreteModel::Action& action_) : node(node_), action(action_) {}
                TP::Node node;
                TP::DiscreteModel::Action action;

                bool operator==(const NodeActionPair& other) const {
                    return node == other.node && action == other.action;
                }
            };
            struct NodeActionPairHash {

                std::size_t operator()(const NodeActionPair& node_action_pair) const {
                    return std::hash<TP::Node>{}(node_action_pair.node) ^ (std::hash<TP::DiscreteModel::Action>{}(node_action_pair.action) << 1);
                }
            };
        protected:
            PRLStorage(uint32_t n_tasks, const TASK_T& default_task_element, const NAP_T& default_nap_element)
                : m_task_elements(n_tasks, default_task_element)
                , m_default_nap_element(default_nap_element)
            {}

        public:
            inline NAP_T& getNAPElement(TP::Node node, const TP::DiscreteModel::Action& action) {
                auto it = this->m_node_action_pair_elements.find(NodeActionPair(node, action));
                if (it != this->m_node_action_pair_elements.end()) {
                    return it->second;
                } else {
                    //LOG("MAKING NEW node: " << node << " action: " << action);
                    auto result = m_node_action_pair_elements.emplace(std::make_pair(NodeActionPair(node, action), m_default_nap_element));
                    return result.first->second;
                }
            }

            inline const NAP_T& lookupNAPElement(TP::Node node, const TP::DiscreteModel::Action& action) const {
                return this->m_node_action_pair_elements.at(NodeActionPair(node, action));
            }

            inline const TASK_T& getTaskElement(TP::DiscreteModel::ProductRank task_i) const {
                return m_task_elements[task_i];
            }

            inline TASK_T& getTaskElement(TP::DiscreteModel::ProductRank task_i) {
                return m_task_elements[task_i];
            }

        protected:
            NAP_T m_default_nap_element;
            std::unordered_map<NodeActionPair, NAP_T, NodeActionPairHash> m_node_action_pair_elements;
            std::vector<TASK_T> m_task_elements;
    };

    template <class SYMBOLIC_GRAPH_T, uint32_t COST_CRITERIA_M>
    class BehaviorHandler : public PRLStorage<RewardBehavior, CostBehaviorArray<COST_CRITERIA_M>> {
        public:
            using CostVector = TP::Containers::FixedArray<COST_CRITERIA_M + 1, float>;
        public:
            BehaviorHandler(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product, uint8_t completed_tasks_horizon, float ucb_confidence) 
                : PRLStorage<RewardBehavior, CostBehaviorArray<COST_CRITERIA_M>>(
                    product->rank() - 1, 
                    RewardBehavior(ucb_confidence),
                    CostBehaviorArray<COST_CRITERIA_M>(ucb_confidence))
                , m_product(product)
                , m_max_ucb_reward(0.0f)
                , m_completed_tasks_horizon(completed_tasks_horizon)
                , m_ucb_confidence(ucb_confidence)
            {
                update(0);
            }

            static constexpr std::size_t numBehaviors() noexcept {return COST_CRITERIA_M + 1;}
            static constexpr std::size_t numCostCriteria() noexcept {return COST_CRITERIA_M;}
            
            void update(uint32_t n_completed_tasks) {
                // Increase the number of tasks completed to update the UCB index
                m_n_completed_tasks += n_completed_tasks;

                // Determine the new max reward for the planning instance
                m_max_ucb_reward = 0.0f;
                for (const auto& reward_criterion : this->m_task_elements) {
                    float r = reward_criterion.getUCB(m_n_completed_tasks);
                    if (r > m_max_ucb_reward) m_max_ucb_reward = r;
                }
            }

            CostVector getCostVector(const TaskHistoryNode<TP::WideNode>& src_node, const TaskHistoryNode<TP::WideNode>& dst_node, const TP::DiscreteModel::Action& action) {
                TP::Node src_model_node = m_product->getUnwrappedNode(src_node.base_node).ts_node;
                //typename CostBehaviorArray<COST_CRITERIA_M>::CostVector costs_only_cv = this->getNAPElement(src_model_node, action).getRectifiedUCBVector(m_state_visits[src_model_node]);
                typename CostBehaviorArray<COST_CRITERIA_M>::CostVector costs_only_cv = this->getNAPElement(src_model_node, action).getRectifiedUCBVector(m_state_visits);
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
                    return -static_cast<float>(n_completed_tasks) * m_max_ucb_reward;
                } else {
                    return -2.0f * static_cast<float>(this->m_task_elements.size()) * m_max_ucb_reward;
                }
            }

            void assignTransformedReward(const TaskHistoryNode<TP::WideNode>& src_node, const TaskHistoryNode<TP::WideNode>& dst_node, float& reward) const {
                ASSERT(src_node.n_completed_tasks <= dst_node.n_completed_tasks, "Dst node has fewer completed tasks!");
                reward = 0.0f;
                if (src_node.n_completed_tasks < dst_node.n_completed_tasks) {
                    //LOG("found task completion transition diff: " << ((uint32_t)dst_node.n_completed_tasks - (uint32_t)src_node.n_completed_tasks));
                    for (TP::DiscreteModel::ProductRank automaton_i = 0; automaton_i < m_product->rank() - 1; ++automaton_i) {
                        if (!m_product->acc(src_node.base_node, automaton_i) && m_product->acc(dst_node.base_node, automaton_i)) {
                            // Accumulate reward for each task satisfied
                            //LOG("ucb val: " << this->getTaskElement(automaton_i).getUCB(m_n_completed_tasks));
                            reward += -this->getTaskElement(automaton_i).getUCB(m_n_completed_tasks);
                        }
                    }
                    reward += priceFunctionTransform(src_node.n_completed_tasks) - priceFunctionTransform(dst_node.n_completed_tasks);
                    //LOG("new reward: " << reward);
                }
            }

                
            inline const std::shared_ptr<SYMBOLIC_GRAPH_T>& getProduct() const {return m_product;}
            inline void setCompletedTasksHorizon(uint8_t horizon) const {m_completed_tasks_horizon = horizon;}
            inline uint8_t getCompletedTasksHorizon() const {return m_completed_tasks_horizon;}

            void visit(const TaskHistoryNode<TP::WideNode>& node, const TP::DiscreteModel::Action& action, const CostBehaviorArray<COST_CRITERIA_M>::CostVector& sample) {
                TP::Node src_model_node = m_product->getUnwrappedNode(node.base_node).ts_node;
                //++m_state_visits[src_model_node];
                ++m_state_visits;
                this->getNAPElement(src_model_node, action).pull(sample);
            }
            
            void collect(TP::DiscreteModel::ProductRank task_i, float sample) {
                ++m_n_completed_tasks;
                this->getTaskElement(task_i).pull(sample);
            }

            void print() const {
                for (const auto&[nap, element] : this->m_node_action_pair_elements) {
                    TP::DiscreteModel::State s = m_product->getModel().getGenericNodeContainer()[nap.node];
                    //LOG("here is the state" << s.to_str());
                    PRINT_NAMED("State: " << s.to_str() << " Action: " << nap.action, "estimate cost mean: " << element.getEstimateDistributions()[0].mu);
                }
            }

        private:
            std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;
            float m_max_ucb_reward = 0.0f;
            uint8_t m_completed_tasks_horizon = 1;
            float m_ucb_confidence = 1.0f;
            
            // UCB parameters
            //std::unordered_map<TP::Node, uint32_t> m_state_visits;
            uint32_t m_state_visits = 0;
            uint32_t m_n_completed_tasks = 0;
    };
}