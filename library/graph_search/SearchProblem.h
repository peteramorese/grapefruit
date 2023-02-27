#pragma once

#include <functional>
#include <map>
#include <set>
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
        - For use with Containers::FixedArray, must have a cast-from-double method (operator COST_T() const)
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

    template <class EXPLICIT_GRAPH_T, class COST_T, SearchDirection SEARCH_DIRECTION, class HEURISTIC_T = ZeroHeuristic<Node, COST_T>>
    struct QuantitativeGraphSearchProblem {
        public: // Methods & members required by any search problem
            
            // Extension methods
            inline const std::vector<typename EXPLICIT_GRAPH_T::node_t>& neighbors(Node node) const {
                if constexpr (SEARCH_DIRECTION == SearchDirection::Forward)
                    return m_graph->getChildren(node);
                else 
                    return m_graph->getParents(node);
            }

            inline const std::vector<typename EXPLICIT_GRAPH_T::edge_t>& neighborEdges(Node node) const {
                if constexpr (SEARCH_DIRECTION == SearchDirection::Forward)
                    return m_graph->getOutgoingEdges(node);
                else
                    return m_graph->getIncomingEdges(node);
            }

            // Termination goal node
            virtual inline bool goal(const Node& node) const {return m_goal_node_set.contains(node);}

            // Quantative methods
            inline COST_T gScore(const COST_T& parent_g_score, const EXPLICIT_GRAPH_T::edge_t& edge) const {return parent_g_score + static_cast<COST_T>(edge);}
            //inline COST_T gScore(const COST_T& parent_g_score, const EXPLICIT_GRAPH_T::edge_t& edge) const {return parent_g_score + m_edgeToCost(edge);}
            COST_T hScore(const EXPLICIT_GRAPH_T::node_t& node) const {return heuristic.operator()(node);}

            // Member variables
            std::set<typename EXPLICIT_GRAPH_T::node_t> initial_node_set;
            HEURISTIC_T heuristic = HEURISTIC_T{}; // assumes default ctor

        public:
            //typedef COST_T(*edgeToCostFunction)(const EXPLICIT_GRAPH_T::edge_t&);

            //QuantitativeGraphSearchProblem(const std::shared_ptr<EXPLICIT_GRAPH_T>& graph, const std::vector<typename EXPLICIT_GRAPH_T::node_t>& initial_node_set_, const std::set<typename EXPLICIT_GRAPH_T::node_t>& goal_node_set, edgeToCostFunction edgeToCost) 
            QuantitativeGraphSearchProblem(const std::shared_ptr<EXPLICIT_GRAPH_T>& graph, const std::set<typename EXPLICIT_GRAPH_T::node_t>& initial_node_set_, const std::set<typename EXPLICIT_GRAPH_T::node_t>& goal_node_set) 
                : initial_node_set(initial_node_set_) 
                , m_graph(graph)
                , m_goal_node_set(goal_node_set)
                //, m_edgeToCost(edgeToCost)
                {}

        private:
            const std::shared_ptr<EXPLICIT_GRAPH_T> m_graph;
            std::set<typename EXPLICIT_GRAPH_T::node_t> m_goal_node_set;
            //edgeToCostFunction m_edgeToCost;

    };


    // Single-Objective tools

    template <class NODE_T, class EDGE_STORAGE_T>
    struct Connection {
        Connection() = delete;
        Connection(const NODE_T& node_, const EDGE_STORAGE_T& edge_) : node(node_), edge(edge_) {}
        Connection(const NODE_T& node_, EDGE_STORAGE_T&& edge_) : node(node_), edge(std::move(edge_)) {}
        Connection(const Connection& other) : node(other.node), edge(other.edge) {}
        NODE_T node;
        EDGE_STORAGE_T edge;
    };

    template <class NODE_T, class EDGE_STORAGE_T, class COST_T>
    struct PathSolution {
        PathSolution() = default;
        PathSolution(PathSolution&& other) 
            : node_path(std::move(other.node_path)), edge_path(std::move(other.edge_path)), path_cost(std::move(other.path_cost)) {}
        PathSolution(std::vector<NODE_T>&& node_path_, std::vector<EDGE_STORAGE_T>&& edge_path_, const COST_T& path_cost_) 
            : node_path(std::move(node_path_)), edge_path(std::move(edge_path_)), path_cost(path_cost_) {}
        std::vector<NODE_T> node_path;
        std::vector<EDGE_STORAGE_T> edge_path;
        COST_T path_cost = COST_T{};
    };

    template <class NODE_T, class EDGE_STORAGE_T>
    using SearchTree = std::map<NODE_T, Connection<NODE_T, EDGE_STORAGE_T>>;
    
    template <class NODE_T, class COST_T>
    using MinCostMap = std::map<NODE_T, COST_T>;

    template <class NODE_T, class EDGE_STORAGE_T, class COST_T>
    struct SingleObjectiveSearchResult {
        public:
            SingleObjectiveSearchResult(bool retain_search_tree = true, bool retain_min_cost_map = true)
                : search_tree(std::make_shared<SearchTree<NODE_T, EDGE_STORAGE_T>>())
                , min_cost_map(std::make_shared<MinCostMap<NODE_T, COST_T>>()) 
                , m_retain_search_tree(retain_search_tree)
                , m_retain_min_cost_map(retain_min_cost_map)
                {}

            bool success = false;
            PathSolution<NODE_T, EDGE_STORAGE_T, COST_T> solution;
            std::shared_ptr<SearchTree<NODE_T, EDGE_STORAGE_T>> search_tree;
            std::shared_ptr<MinCostMap<NODE_T, COST_T>> min_cost_map;

            void package() { // Free the memory of the search tree and min cost map if the user desires
                if (!m_retain_search_tree) search_tree.reset();
                if (!m_retain_min_cost_map) min_cost_map.reset();
            }
        private:
            bool m_retain_search_tree, m_retain_min_cost_map;
    };

}
}