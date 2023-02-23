#pragma once

#include <string>

#include "core/Automaton.h"

namespace TP {

namespace FormalMethods {

    struct PartialSatisfactionEdge {
        std::string label;
        uint32_t substitution_cost;
    };

    class PartialSatisfactionDFA : public DFA {
        public:
            inline const std::vector<PartialSatisfactionEdge>& getOutgoingEdges(Node node) {
                return m_graph[node].forward.edges;
            }

            inline const std::vector<PartialSatisfactionEdge>& getIncomingEdges(Node node) {
                return m_graph[node].backward.edges;
            }

    }
}
}