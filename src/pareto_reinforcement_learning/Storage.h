#pragma once

#include "TaskPlanner.h"

namespace PRL {

    template <class NA_ELEMENT_T>
    class NodeActionStorage {
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
            PRLStorage(const NA_ELEMENT_T& default_nap_element)

            inline NA_ELEMENT_T& getNAElement(TP::Node node, const TP::DiscreteModel::Action& action) {
                auto it = this->m_node_action_pair_elements.find(NodeActionPair(node, action));
                if (it != this->m_node_action_pair_elements.end()) {
                    return it->second;
                } else {
                    auto result = m_node_action_pair_elements.emplace(std::make_pair(NodeActionPair(node, action), m_default_na_element));
                    return result.first->second;
                }
            }

            inline const NA_ELEMENT_T& lookupNAElement(TP::Node node, const TP::DiscreteModel::Action& action) const {
                return this->m_node_action_pair_elements.at(NodeActionPair(node, action));
            }

        protected:
            std::unordered_map<NodeActionPair, NAP_T, NodeActionPairHash> m_node_action_pair_elements;

        private:
            NA_ELEMENT_T m_default_na_element;
    };
    
    template <class TASK_ELEMENT_T, class NA_ELEMENT_T>
    class TaskNodeActionStorage : public NodeActionStorage<NA_ELEMENT_T> {
        protected:
            PRLStorage(uint32_t n_tasks, const TASK_ELEMENT_T& default_task_element, const NAP_T& default_na_element)
                : NodeActionStorage<NA_ELEMENT_T>(default_na_element)
                , m_task_elements(n_tasks, default_task_element)
            {}

        public:

            inline const TASK_T& getTaskElement(TP::DiscreteModel::ProductRank task_i) const {
                return m_task_elements[task_i];
            }

            inline TASK_T& getTaskElement(TP::DiscreteModel::ProductRank task_i) {
                return m_task_elements[task_i];
            }

        protected:
            std::vector<TASK_T> m_task_elements;
    };

}