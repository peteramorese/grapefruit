#pragma once

#include <functional>
#include <map>
#include <memory>
#include <unordered_map>

#include "core/Graph.h"

#define TP_COST_VECTOR_EQUIVALENCE_TOLERANCE 0.0000000001

namespace TP {
namespace GraphSearch {

    /* Type requirements

    NODE_T:
        - Must be ordered hashable (std::map)
        - Must be copy constructable
    EDGE_T:
        - None
    COST_T:
        - Must contain 'less than' or 'dominates' operator< 
        - Must contain 'addition' operator+ and 'subtraction' operator-
        - Default constructed value must be the respective 'zero' value
        - Must be copy constructable
        - For use with CostVector, must have a cast-from-double method (operator COST_T() const)
    HEURISTIC_T:
        - User specified
        - Must contain operator() which retrieves the heuristic COST_T value given a node
    EDGE_STORAGE_T:
        By default, edges are copied inside the search result (i.e. search tree, edge path). If the graph is explicit, and the 
        edge type is large or non-copyable, the user can specify template parameter 'EDGE_STORAGE_T' as 'const EDGE_T*' 
        to store pointers to the edges stored in the explicit graph to prevent duplicates and minimize memory usage
        - Must be equal to 'EDGE_T' or 'const EDGE_T*'

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

    // Search direction
    enum class SearchDirection {Forward, Backward};

    template <class EDGE_T, class COST_T, SearchDirection SEARCH_DIRECTION, class HEURISTIC_T = ZeroHeuristic<Node, COST_T>>
    struct QuantitativeGraphSearchProblem {
        public: // Methods & members required by any search problem
            
            // Extension methods
            inline const std::vector<Node>& neighbors(Node node) const {
                if constexpr (SEARCH_DIRECTION == SearchDirection::Forward)
                    return m_graph->getChildren(node);
                else 
                    return m_graph->getParents(node);
            }

            inline const std::vector<EDGE_T>& neighborEdges(Node node) const {
                if constexpr (SEARCH_DIRECTION == SearchDirection::Forward)
                    return m_graph->getOutgoingEdges(node);
                else
                    return m_graph->getIncomingEdges(node);
            }

            // Termination goal node
            inline bool goal(const Node& node) const {return node == m_goal_node;}

            // Quantative methods
            inline COST_T gScore(const COST_T& parent_g_score, const EDGE_T& edge) const {return parent_g_score + m_edgeToCost(edge);}
            COST_T hScore(const Node& node) const {return heuristic.operator()(node);}

            // Member variables
            std::vector<Node> initial_node_set;
            HEURISTIC_T heuristic = HEURISTIC_T{}; // assumes default ctor

        public:
            typedef COST_T(*edgeToCostFunction)(const EDGE_T&);

            QuantitativeGraphSearchProblem(const std::shared_ptr<Graph<EDGE_T>>& graph, const std::vector<Node> initial_node_set_, Node goal_node, edgeToCostFunction edgeToCost) 
                : initial_node_set(initial_node_set_) 
                , m_graph(graph)
                , m_goal_node(goal_node)
                , m_edgeToCost(edgeToCost)
                {}

        private:
            const std::shared_ptr<Graph<EDGE_T>> m_graph;
            Node m_goal_node;
            edgeToCostFunction m_edgeToCost;

    };


    // Multi-Objective tools

    using ObjectiveCount = uint8_t;

    template<ObjectiveCount M, class COST_T>
    struct CostVector {

        inline static const COST_T s_numerical_tolerance = static_cast<COST_T>(TP_COST_VECTOR_EQUIVALENCE_TOLERANCE);

        std::array<COST_T, M> values = std::array<COST_T, M>();

        CostVector() = default;
        
        // Floating point error numerical comparison for hashing/sorting
        bool operator==(const CostVector& other) const {
            for (ObjectiveCount i=0; i < M; ++i) {
                if (abs(values[i] - other.values[i]) > s_numerical_tolerance) return false;
            }
            return true;
        }
        // 'Dominates' operator
        bool dominates(const CostVector& other) const {
            bool equal = true;
            for (ObjectiveCount i=0; i < M; ++i) {
                if (values[i] > (other.values[i] + s_numerical_tolerance)) {
                    return false;
                } else {
                    if (equal && values[i] < (other.values[i] - s_numerical_tolerance)) equal = false;
                }
            }
            return !equal;
        }
        // Lexicographic ordering
        bool operator<(const CostVector& other) const {
            for (ObjectiveCount i=0; i < M; ++i) {
                if (values[i] < (other.values[i] - s_numerical_tolerance)) {
                    return true;
                } else if (values[i] > (other.values[i] + s_numerical_tolerance)) {
                    return false;
                }
            }
            return false;
        }
    };

    template <class EDGE_STORAGE_T>
    using SearchGraph = Graph<EDGE_STORAGE_T>;

    template <class COST_VECTOR_T>
    struct NonDominatedCostMap {
        public:
            class OrderedCostSet {
                public:
                    OrderedCostSet() = default;
                    inline void eraseDominated(const COST_VECTOR_T& cost_vector) {
                        for (auto it = m_set.begin(); it != m_set.end();) {
                            if (cost_vector.dominates(it->first)) {
                                m_set.erase(it++);
                            } else {
                                ++it;
                            }
                        }
                    }
                    inline void addToOpen(const COST_VECTOR_T& v) {m_set[v] = true;}
                    inline void addToClosed(const COST_VECTOR_T& v) {m_set[v] = false;}
                    inline void moveToClosed(const COST_VECTOR_T& v) {m_set[v] = false;}
                    inline bool contains(const COST_VECTOR_T& v) const {return m_set.contains(v);}
                private:
                    std::map<COST_VECTOR_T, bool> m_set;
            };

        public:
            std::map<Node, OrderedCostSet> cost_map;
    };

    template <ObjectiveCount M, class EDGE_STORAGE_T, class COST_T>
    struct MultiObjectiveSearchResult {
        public:
            MultiObjectiveSearchResult(bool retain_search_graph = true, bool retain_non_dominated_cost_map = true)
                : search_graph(std::make_shared<SearchGraph<EDGE_STORAGE_T>>(true, true))
                , non_dominated_cost_map(std::make_shared<NonDominatedCostMap<CostVector<M, COST_T>>>()) 
                , m_retain_search_graph(retain_search_graph)
                , m_retain_non_dominated_cost_map(retain_non_dominated_cost_map)
                {}

            bool success = false;
            std::vector<PathSolution<Node, EDGE_STORAGE_T, COST_T>> solution_set;
            std::shared_ptr<SearchGraph<EDGE_STORAGE_T>> search_graph;
            std::shared_ptr<NonDominatedCostMap<CostVector<M, COST_T>>> non_dominated_cost_map;

            void package() { // Free the memory of the search tree and min cost map if the user desires
                if (!m_retain_search_graph) search_graph.reset();
                if (!m_retain_non_dominated_cost_map) non_dominated_cost_map.reset();
            }
        private:
            bool m_retain_search_graph, m_retain_non_dominated_cost_map;
    };

}
}