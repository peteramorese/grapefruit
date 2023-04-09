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

    template <class...T_ARGS>
    struct CostBehavior {
        public:
            using CostVector = TP::Containers::FixedArray<TP::Containers::TypeGenericArray<T_ARGS...>::size(), float>;
        public:
            CostBehavior() {
                auto setAllToDefaultPrior = []<typename T>(const T& behavior) {behavior.setToDefaultPrior();};
                behavior_criteria.forEach(setAllToDefaultPrior);
            }

            CostBehavior(T_ARGS&&...priors) 
                : behavior_criteria(std::forward<T_ARGS>(priors)...) 
            {}

            CostVector getCostVector() const {
                CostVector cv;
                auto extract = [&cv]<typename T, uint32_t I>(const T& behavior) {
                    cv.template get<I + I>() = behavior.getExpectation();
                    cv.template get<I + I + 1>() = behavior.getVariance();
                };
                behavior_criteria.forEachWithI(extract);
                return cv;
            }
            
            TP::Containers::TypeGenericArray<T_ARGS...> behavior_criteria;
    };

    template <class SYMBOLIC_GRAPH_T, class REWARD_CRITERION_T, class...COST_CRITERIA_T>
    class BehaviorHandler {
        public:
            struct NodeActionPair {
                TP::WideNode node;
                TP::DiscreteModel::Action action;
            };
            struct NodeActionPairHash {
                std::size_t operator()(const NodeActionPair& node_action_pair) const {
                    return std::hash<TP::WideNode>{}(node_action_pair.node) ^ std::hash<TP::DiscreteModel::Action>{}(node_action_pair.action);
                }
            };
            //using CostVector = TP::Containers::FixedArray<TP::Containers::TypeGenericArray<T_ARGS...>::size(), float>;
            using CostVector = TP::Containers::FixedArray<CostBehavior<REWARD_CRITERION_T, COST_CRITERIA_T...>::Costvector::size() + 2, float>;
        public:
            BehaviorHandler(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product, TP::Containers::SizedArray<REWARD_CRITERION_T>& reward_criteria, uint8_t completed_tasks_horizon) 
                : m_product(product)
                , m_reward_criteria(reward_criteria)
                , m_max_mean_reward(0.0f)
                , m_completed_tasks_horizon(completed_tasks_horizon)
            {
                ASSERT(m_reward_criteria.size() == (m_product->rank() - 1), "Number of reward criteria does not match number of tasks");
                for (const auto& reward_criterion : m_reward_criteria) {
                    float expec = reward_criterion.getExpectation();
                    if (expec > m_max_mean_reward) m_max_mean_reward = expec;
                }
            }

            static constexpr std::size_t numBehaviors() noexcept {return sizeof...(COST_CRITERIA_T) + 1;}
            static constexpr std::size_t cvDim() noexcept {return Behavior<REWARD_CRITERION_T, COST_CRITERIA_T...>::Costvector::size() + 2;}
            
            CostVector getCostVector(const TaskHistoryNode<TP::WideNode>& src_node, const TaskHistoryNode<TP::WideNode>& dst_node, const TP::DiscreteModel::Action& action) const {
                typename Behavior<REWARD_CRITERION_T, COST_CRITERIA_T...>::Costvector costs_only_cv = m_behaviors.at(NodeActionPair{src_node.base_node, action}).getCostVector();
                CostVector cv;
                for (uint32_t i=0; i<cv.size(); ++i) {
                    if (i < 2) 
                        cv[i] = 0.0f;
                    else
                        cv[i] = costs_only_cv[i - 2];
                }
                assignReward(src_node, dst_node, cv);
                return cv;
            }

            inline float priceFunctionTransform(uint8_t n_completed_tasks) const {
                return -static_cast<float>(TP::min(n_completed_tasks, m_completed_tasks_horizon)) * m_max_mean_reward;
            }

            void assignReward(const TaskHistoryNode<TP::WideNode>& src_node, const TaskHistoryNode<TP::WideNode>& dst_node, CostVector& cv) {
                ASSERT(src_node.n_completed_tasks <= dst_node.n_completed_tasks, "Dst node has fewer completed tasks!");
                if (src_node.n_completed_tasks < dst_node.n_completed_tasks) {
                    float& reward_mean = cv.template get<0>(); // mean reward
                    float& reward_variance = cv.template get<1>(); // mean reward
                    for (TP::DiscreteModel::ProductRank automaton_i = 0; automaton_i < m_product->rank(); ++automaton_i) {
                        if (!m_product->acc(node.base_node, automaton_i) && m_product->acc(children[i], automaton_i)) {
                            // Transform by price function
                            reward_mean -= m_reward_criteria[automaton_i].getExpectation() 
                                + priceFunctionTransform(src_node.n_completed_tasks) - priceFunctionTransform(dst_node.n_completed_tasks);

                            reward_variance += m_reward_criteria[automaton_i].getVariance();
                        }
                    }
                }
            }


        private:
            std::unordered_map<NodeActionPair, typename CostBehavior<COST_CRITERIA_T...>, NodeActionPairHash> m_cost_behaviors;
            std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;
            TP::Containers::SizedArray<REWARD_CRITERION_T> m_reward_criteria;
            float m_max_mean_reward = 0.0f;
            uint8_t m_completed_tasks_horizon = 1;
    };
}