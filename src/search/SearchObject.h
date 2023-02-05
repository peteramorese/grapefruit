#pragma once

#include <functional>

#include "core/Graph.h"

namespace TP {
namespace GraphSearch {

    template <class NODE_T, class EDGE_T, class COST_T>
    struct SymbolicQuantitativeSearchObject {

        // Extension methods
        const std::vector<NODE_T>& children(const NODE_T& node);
        const std::vector<EDGE_T>& outgoingEdges(const NODE_T& node);
        const std::vector<NODE_T>& parents(const NODE_T& node);
        const std::vector<EDGE_T>& incomingEdges(const NODE_T& node);

        // Quantative methods
        const COST_T& propagateCost(const NODE_T& node, const EDGE_T& edge);
        // Note: COST_T must have a '<' operator
    };
}
}