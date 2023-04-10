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
            using CostVector = TP::Containers::FixedArray<2 * TP::Containers::TypeGenericArray<T_ARGS...>::size(), float>;
        public:
            CostBehavior() {
                auto setAllToDefaultPrior = []<typename T>(T& behavior) -> bool {behavior.setToDefaultPrior(); return false;};
                behavior_criteria.forEach(setAllToDefaultPrior);
            }

            CostBehavior(T_ARGS&&...priors) 
                : behavior_criteria(std::forward<T_ARGS>(priors)...) 
            {}

            CostVector getCostVector() const {
                CostVector cv;
                auto extract = [&cv]<typename T, uint32_t I>(const T& behavior) -> bool {
                    cv.template get<I + I>() = behavior.getExpectation();
                    cv.template get<I + I + 1>() = behavior.getVariance();
                    return false;
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
            //using CostVector = TP::Containers::FixedArray<TP::Containers::TypeGenericArray<T_ARGS...>::size(), float>;
            using CostVector = TP::Containers::FixedArray<CostBehavior<COST_CRITERIA_T...>::CostVector::size() + 2, float>;
        public:
            BehaviorHandler(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product, const TP::Containers::SizedArray<REWARD_CRITERION_T>& reward_criteria, uint8_t completed_tasks_horizon) 
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
            static constexpr std::size_t cvDim() noexcept {return CostBehavior<COST_CRITERIA_T...>::CostVector::size() + 2;}
            
            CostVector getCostVector(const TaskHistoryNode<TP::WideNode>& src_node, const TaskHistoryNode<TP::WideNode>& dst_node, const TP::DiscreteModel::Action& action) {
                typename CostBehavior<COST_CRITERIA_T...>::CostVector costs_only_cv = m_cost_behaviors[NodeActionPair(src_node.base_node, action)].getCostVector();
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

            void assignReward(const TaskHistoryNode<TP::WideNode>& src_node, const TaskHistoryNode<TP::WideNode>& dst_node, CostVector& cv) const {
                ASSERT(src_node.n_completed_tasks <= dst_node.n_completed_tasks, "Dst node has fewer completed tasks!");
                if (src_node.n_completed_tasks < dst_node.n_completed_tasks) {
                    LOG("found task completion transition diff: " << ((uint32_t)dst_node.n_completed_tasks - (uint32_t)src_node.n_completed_tasks));
                    float& reward_mean = cv.template get<0>(); // mean reward
                    float& reward_variance = cv.template get<1>(); // mean reward
                    for (TP::DiscreteModel::ProductRank automaton_i = 0; automaton_i < m_product->rank() - 1; ++automaton_i) {
                        if (!m_product->acc(src_node.base_node, automaton_i) && m_product->acc(dst_node.base_node, automaton_i)) {
                            // Transform by price function
                            reward_mean += -m_reward_criteria[automaton_i].getExpectation() 
                                + priceFunctionTransform(src_node.n_completed_tasks) - priceFunctionTransform(dst_node.n_completed_tasks);

                            reward_variance += m_reward_criteria[automaton_i].getVariance();
                        }
                    }
                    LOG("new reward mean: " << reward_mean << " var: " << reward_variance);
                }
            }

            inline const CostBehavior<COST_CRITERIA_T...>& getCostBehavior(TP::WideNode node, const TP::DiscreteModel::Action& action) {
                return m_cost_behaviors[NodeActionPair(node, action)];
            }

            inline const REWARD_CRITERION_T& getRewardCriteria(TP::DiscreteModel::ProductRank automaton_i) const {
                return m_reward_criteria[automaton_i];
            }
                
            inline const std::shared_ptr<SYMBOLIC_GRAPH_T>& getProduct() const {return m_product;}
            inline void setCompletedTasksHorizon(uint8_t horizon) const {m_completed_tasks_horizon = horizon;}
            inline uint8_t getCompletedTasksHorizon() const {return m_completed_tasks_horizon;}

        private:
            std::unordered_map<NodeActionPair, CostBehavior<COST_CRITERIA_T...>, NodeActionPairHash> m_cost_behaviors;
            std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;
            TP::Containers::SizedArray<REWARD_CRITERION_T> m_reward_criteria;
            float m_max_mean_reward = 0.0f;
            uint8_t m_completed_tasks_horizon = 1;
    };
}