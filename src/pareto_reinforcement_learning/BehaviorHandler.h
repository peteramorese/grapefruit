#pragma once

#include <memory>

#include "TaskPlanner.h"

#include "HistoryNode.h"
#include "Storage.h"

namespace PRL {

    template <uint32_t M>
    class CostBehaviorHandler : public NodeActionStorage<JointCostArray> {
        public:
            using CostVector = TP::Containers::FixedArray<M + 1, float>;
        public:
            CostBehaviorHandler(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product, uint8_t completed_tasks_horizon, float ucb_confidence)
                : NodeActionStorage<JointCostArray>(JointCostArray(ucb_confidence))
                , m_product(product)
                , m_completed_tasks_horizon(completed_tasks_horizon)
            {}

            static constexpr std::size_t size() noexcept {return M;}

            CostVector getCostVector(const TaskHistoryNode<TP::WideNode>& src_node, const TaskHistoryNode<TP::WideNode>& dst_node, const TP::DiscreteModel::Action& action) {
                TP::Node src_model_node = m_product->getUnwrappedNode(src_node.base_node).ts_node;
                return this->getNAElement(src_model_node, action).getRectifiedUCBVector(m_state_visits);
            }

            inline const std::shared_ptr<SYMBOLIC_GRAPH_T>& getProduct() const {return m_product;}

            void visit(const TaskHistoryNode<TP::WideNode>& node, const TP::DiscreteModel::Action& action, const CostVector& sample) {
                TP::Node src_model_node = m_product->getUnwrappedNode(node.base_node).ts_node;
                ++m_state_visits;
                this->getNAElement(src_model_node, action).pull(sample);
            }
            
        
        private:
            std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;
            uint8_t m_completed_tasks_horizon = 1;
            uint32_t m_state_visits = 0;

    };

    template <class SYMBOLIC_GRAPH_T, uint32_t COST_CRITERIA_M>
    class RewardCostBehaviorHandler : public TaskNodeActionStorage<RewardBehavior, IndependentCostArray<COST_CRITERIA_M>> {
        public:
            using CostVector = TP::Containers::FixedArray<COST_CRITERIA_M + 1, float>;
        public:
            BehaviorHandler(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product, uint8_t completed_tasks_horizon, float reward_ucb_confidence, float cost_ucb_confidence) 
                : TaskNodeActionStorage<RewardBehavior, IndependentCostArray<COST_CRITERIA_M>>(
                    product->rank() - 1, 
                    RewardBehavior(reward_ucb_confidence),
                    IndependentCostArray<COST_CRITERIA_M>(cost_ucb_confidence))
                , m_product(product)
                , m_max_ucb_reward(0.0f)
                , m_completed_tasks_horizon(completed_tasks_horizon)
            {
                update(0);
            }

            static constexpr std::size_t numBehaviors() noexcept {return COST_CRITERIA_M + 1;}
            static constexpr std::size_t numCostCriteria() noexcept {return COST_CRITERIA_M;}
            
            void update(uint32_t n_completed_tasks) {

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
                typename CostBehaviorArray<COST_CRITERIA_M>::CostVector costs_only_cv = this->getNAElement(src_model_node, action).getRectifiedUCBVector(m_state_visits);
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
                this->getNAElement(src_model_node, action).pull(sample);
            }
            
            void collect(TP::DiscreteModel::ProductRank task_i, float sample) {
                ++m_n_completed_tasks;
                //LOG("Collecting task " << (uint32_t)task_i << ": " << m_n_completed_tasks);
                this->getTaskElement(task_i).pull(sample);
            }

            void print() const {
                for (const auto&[nap, element] : this->m_node_action_pair_elements) {
                    TP::DiscreteModel::State s = m_product->getModel().getGenericNodeContainer()[nap.node];
                    PRINT_NAMED("State: " << s.to_str() << " Action: " << nap.action, "estimate cost mean: " << element.getEstimateDistributions()[0].mu);
                }
            }

        private:
            std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;
            float m_max_ucb_reward = 0.0f;
            uint8_t m_completed_tasks_horizon = 1;
            
            // UCB parameters
            uint32_t m_state_visits = 0;
            uint32_t m_n_completed_tasks = 0;
    };
}