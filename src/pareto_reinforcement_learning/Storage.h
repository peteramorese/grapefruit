#pragma once

#include <unordered_map>

#include "TaskPlanner.h"

namespace PRL {

    template <class ELEMENT_T>
    class Storage {
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

        public:
            Storage(const ELEMENT_T& default_element)
                : m_default_na_element(default_element)
            {}

            inline ELEMENT_T& getElement(TP::Node node, const TP::DiscreteModel::Action& action) {
                auto it = this->m_node_action_pair_elements.find(NodeActionPair(node, action));
                if (it != this->m_node_action_pair_elements.end()) {
                    return it->second;
                } else {
                    auto result = m_node_action_pair_elements.emplace(std::make_pair(NodeActionPair(node, action), m_default_na_element));
                    return result.first->second;
                }
            }

            inline const ELEMENT_T& lookupElement(TP::Node node, const TP::DiscreteModel::Action& action) const {
                return this->m_node_action_pair_elements.at(NodeActionPair(node, action));
            }

        protected:
            std::unordered_map<NodeActionPair, ELEMENT_T, NodeActionPairHash> m_node_action_pair_elements;
            ELEMENT_T m_default_na_element;
    };
    
}