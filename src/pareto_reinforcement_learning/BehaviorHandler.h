#pragma once

#include <memory>

#include "TaskPlanner.h"

namespace PRL {

    template <class...T_ARGS>
    struct Behavior {
        public:
            using CostVector = TP::Containers::FixedArray<TP::Containers::TypeGenericArray<T_ARGS...>::size(), float>;
        public:
            Behavior() {
                auto setAllToDefaultPrior = []<typename T>(const T& behavior) {behavior.setToDefaultPrior();};
                m_behavior_criteria.forEach(setAllToDefaultPrior);
            }

            Behavior(T_ARGS&&...priors) 
                : m_behavior_criteria(std::forward<T_ARGS>(priors)...) 
            {}

            CostVector getCostVector() const {
                CostVector cv;
                auto extract = [&cv]<typename T, uint32_t I>(const T& behavior) {
                    cv.template get<I + I>() = behavior.getExpectation();
                    cv.template get<I + I + 1>() = behavior.getVariance();
                };
                m_behavior_criteria.forEachWithI(extract);
                return cv;
            }
            
            TP::Containers::TypeGenericArray<T_ARGS...> m_behavior_criteria;

    };

    template <class...T_ARGS>
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
            using CostVector = TP::Containers::FixedArray<TP::Containers::TypeGenericArray<T_ARGS...>::size(), float>;
        public:
            BehaviorHandler(const std::shared_ptr<TP::DiscreteModel::TransitionSystem>& ts);
            
            CostVector getCostVector(TP::WideNode node, const TP::DiscreteModel::Action& action) const {return m_behaviors.at(NodeActionPair{node, action}).getCostVector();}

        private:
            std::shared_ptr<TP::DiscreteModel::TransitionSystem> m_ts;
            std::unordered_map<NodeActionPair, Behavior<T_ARGS...>, NodeActionPairHash> m_behaviors;
    };
}