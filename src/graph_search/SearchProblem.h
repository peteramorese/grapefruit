#pragma once

#include <functional>

#include "core/Graph.h"

namespace TP {
namespace GraphSearch {

    /* Type requirements
    NODE_T:
        - must be ordered hashable (std::map)
        - must be copy constructable
    EDGE_T:
        - none
    COST_T:
        - must contain a 'less than' or 'dominates' operator< 
        - default constructed value must be the respective 'zero' value
        - must be copy constructable
    HEURISTIC_T:
        - user specified
        - must contain operator() which retrieves the heuristic COST_T value given a node
    */

    template <class NODE_T, class COST_T>
    struct ZeroHeuristic {
        COST_T operator()(const NODE_T& node) const {return COST_T();}
    };

    // TODO Make this an interface class
    template <class NODE_T, class EDGE_T, class COST_T, class HEURISTIC_T = ZeroHeuristic<NODE_T, COST_T>>
    struct SymbolicQuantitativeSearchProblem {
        public: // Methods/members required by the search algorithms
            
            // Extension methods
            const std::vector<NODE_T>& children(const NODE_T& node) const;
            const std::vector<EDGE_T>& outgoingEdges(const NODE_T& node) const;
            const std::vector<NODE_T>& parents(const NODE_T& node) const;
            const std::vector<EDGE_T>& incomingEdges(const NODE_T& node) const;

            // Termination goal node
            bool goal(const NODE_T& node) const;

            // Quantative methods
            COST_T gScore(const COST_T& parent_g_score, const EDGE_T& edge) const;
            COST_T hScore(const NODE_T& node) const {return heuristic.operator()(node);}

            // Member variables
            NODE_T initial_node;
            HEURISTIC_T heuristic = HEURISTIC_T{}; // assumes default ctor

        public:
            SymbolicQuantitativeSearchProblem(const NODE_T& initial_node_) : initial_node(initial_node_) {}
    };
}
}