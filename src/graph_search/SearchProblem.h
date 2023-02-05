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

    /* Search Problem requirements

    Required methods:
    
    const std::vector<NODE_T>& children(const NODE_T& node) const; (explicit)
    std::vector<NODE_T> children(const NODE_T& node) const; (symbolic)
        - Required for forward search algorithms
        - Returns a set of children/neighbor nodes to the node 'node'
        - (explicit) Must return a persistent const reference
    
    const std::vector<EDGE_T>& outgoingEdges(const NODE_T& node) const; (explicit)
    std::vector<EDGE_T> outgoingEdges(const NODE_T& node) const; (symbolic)
        - Required for forward search algorithms
        - Returns the set of edges to each respective child node in 'children(node)'
        - Both 'children()' and 'outgoingEdges()' must return containers in the same order
        - (explicit) Must return a persistent const reference
    
    const std::vector<NODE_T>& parents(const NODE_T& node) const; (explicit)
    std::vector<NODE_T> parents(const NODE_T& node) const; (symbolic)
        - Required for backwards search algorithms
        - Returns the set of parent nodes to the node 'node'
        - (explicit) Must return a persistent const reference
    
    const std::vector<EDGE_T>& incomingEdges(const NODE_T& node) const; (explicit) 
    std::vector<EDGE_T> incomingEdges(const NODE_T& node) const; (symbolic)
        - Required for backwards search algorithms
        - Returns the set of edges to each respective parent node in 'parents(node)'
        - Both 'parents()' and 'incomingEdges()' must return containers in the same order
        - (explicit) Must return a persistent const reference
    
    bool goal(const NODE_T& node) const;
        - Returns 'true' if the input 'node' satisfies the goal condition, and false otherwise
    
    COST_T gScore(const COST_T& parent_g_score, const EDGE_T& edge) const;
        - Returns the new cost of a node with cost 'parent_g_score' exploring through the edge 'edge'. For a typical search problem,
            this method would be equivalent to adding the edge cost to 'parent_g_score'
    
    COST_T hScore(const NODE_T& node) const {return heuristic.operator()(node);}
        - Returns the heuristic cost-to-go for a node 'node'
        - Must be an admissible heuristic (single objective: underestimates the min-cost-go)

    */
    template <class NODE_T, class COST_T>
    struct ZeroHeuristic {
        COST_T operator()(const NODE_T& node) const {return COST_T();}
    };

    template <class EDGE_T, class COST_T, class HEURISTIC_T = ZeroHeuristic<Node, COST_T>>
    struct QuantitativeGraphSearchProblem {
        public: // Methods & members required by any search problem
            
            // Extension methods
            inline const std::vector<Node>& children(Node node) const {return m_graph->getChildren(node);}
            inline const std::vector<EDGE_T>& outgoingEdges(Node node) const {return m_graph->getOutgoingEdges(node);}
            inline const std::vector<Node>& parents(Node node) const {return m_graph->getParents(node);}
            inline const std::vector<EDGE_T>& incomingEdges(Node node) const {return m_graph->getIncomingEdges(node);}

            // Termination goal node
            inline bool goal(const Node& node) const {return node == m_goal_node;}

            // Quantative methods
            inline COST_T gScore(const COST_T& parent_g_score, const EDGE_T& edge) const {return parent_g_score + m_edgeToCost(edge);}
            COST_T hScore(const Node& node) const {return heuristic.operator()(node);}

            // Member variables
            Node initial_node;
            HEURISTIC_T heuristic = HEURISTIC_T{}; // assumes default ctor

        public:
            typedef COST_T(*edgeToCostFunction)(const EDGE_T&);

            QuantitativeGraphSearchProblem(const std::shared_ptr<Graph<EDGE_T>>& graph, Node initial_node_, Node goal_node, edgeToCostFunction edgeToCost) 
                : initial_node(initial_node_) 
                , m_graph(graph)
                , m_goal_node(goal_node)
                , m_edgeToCost(edgeToCost)
                {}

        private:
            const std::shared_ptr<Graph<EDGE_T>> m_graph;
            Node m_goal_node;
            edgeToCostFunction m_edgeToCost;

    };
}
}