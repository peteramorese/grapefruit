#pragma once

#include <functional>
#include <map>
#include <set>
#include <memory>
#include <unordered_map>

#include "core/DirectedAcyclicGraph.h"
#include "graph_search/SearchProblem.h"

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

    using ObjectiveCount = uint8_t;

    
    // Default zero heuristic for multi-objective problems
    template <class NODE_T, class COST_VECTOR_T>
    struct MOZeroHeuristic {
        COST_VECTOR_T operator()(const NODE_T& node) const {return COST_VECTOR_T{};}
    };

    template <class EXPLICIT_GRAPH_T, class COST_VECTOR_T, SearchDirection SEARCH_DIRECTION, class HEURISTIC_T = MOZeroHeuristic<Node, COST_VECTOR_T>>
    struct MOQuantitativeGraphSearchProblem {
        public: // Dependent types required by any search problem

            typedef EXPLICIT_GRAPH_T graph_t;
            typedef EXPLICIT_GRAPH_T::node_t node_t;
            typedef COST_VECTOR_T cost_t;
            typedef EXPLICIT_GRAPH_T::edge_t edge_t;

        public: // Methods & members required by any search problem
            
            // Extension methods
            inline const std::vector<node_t>& neighbors(Node node) const {
                if constexpr (SEARCH_DIRECTION == SearchDirection::Forward)
                    return m_graph->getChildren(node);
                else 
                    return m_graph->getParents(node);
            }

            inline const std::vector<edge_t>& neighborEdges(Node node) const {
                if constexpr (SEARCH_DIRECTION == SearchDirection::Forward)
                    return m_graph->getOutgoingEdges(node);
                else
                    return m_graph->getIncomingEdges(node);
            }

            // Termination goal node
            inline bool goal(const Node& node) const {return m_goal_node_set.contains(node);}

            // Quantative methods
            inline COST_VECTOR_T gScore(const COST_VECTOR_T& parent_g_score, const edge_t& edge) const {return parent_g_score + m_edgeToCostVector(edge);}
            COST_VECTOR_T hScore(const Node& node) const {return heuristic.operator()(node);}

            // Member variables
            std::set<Node> initial_node_set;
            HEURISTIC_T heuristic = HEURISTIC_T{}; // assumes default ctor

        public:
            typedef COST_VECTOR_T(*edgeToCostVectorFunction)(const edge_t&);

            MOQuantitativeGraphSearchProblem(const std::shared_ptr<EXPLICIT_GRAPH_T>& graph, const std::set<Node> initial_node_set_, const std::set<Node>& goal_node_set, edgeToCostVectorFunction edgeToCostVector) 
                : initial_node_set(initial_node_set_) 
                , m_graph(graph)
                , m_goal_node_set(goal_node_set)
                , m_edgeToCostVector(edgeToCostVector)
                {}

        private:
            const std::shared_ptr<EXPLICIT_GRAPH_T> m_graph;
            std::set<Node> m_goal_node_set;
            edgeToCostVectorFunction m_edgeToCostVector;

    };


    // Multi-Objective tools

    template <class CV_STORAGE_T, class EDGE_STORAGE_T>
    struct SearchGraphEdge {
        SearchGraphEdge(const CV_STORAGE_T& cv_, const EDGE_STORAGE_T& edge_) : cv(cv_), edge(edge_) {}
        SearchGraphEdge(const CV_STORAGE_T& cv_, EDGE_STORAGE_T&& edge_) : cv(cv_), edge(std::move(edge_)) {}
        SearchGraphEdge(CV_STORAGE_T&& cv_, EDGE_STORAGE_T&& edge_) : cv(std::move(cv_)), edge(std::move(edge_)) {}
        //SearchGraphEdge(const SearchGraphEdge&) = default;
        //SearchGraphEdge(SearchGraphEdge&&) = default;
        
        // Comaprison only uses cv since the cost makes an edge unique (the edge is just tied for ease of extracting a plan)
        bool operator==(const SearchGraphEdge& other) const {return cv == other.cv;}

        CV_STORAGE_T cv;
        EDGE_STORAGE_T edge;
    };

    template <class SG_EDGE_T, typename NATIVE_NODE_T>
    using SearchGraph = Graph<SG_EDGE_T, NATIVE_NODE_T>;

    template <class COST_VECTOR_T>
    struct NonDominatedCostMap {
        public:
            struct Item {
                Item(COST_VECTOR_T&& cv_, bool in_open_) : cv(std::move(cv_)), in_open(in_open_) {}
                
                // Const Item pointer should not change cv
                COST_VECTOR_T cv;

                // Const Item pointer can edit these properties
                mutable bool in_open;
            };
        public:
            class CostSet {
                public:
                    CostSet() = default;

                    // Erase all elements in the set that are dominated by v (call a signal onErase before erasing the element)
                    template <typename LAM>
                    inline void eraseDominated(const COST_VECTOR_T& v, LAM onErase) {
                        for (auto it = m_set.begin(); it != m_set.end();) {
                            if (v.dominates(it->cv) == Containers::ArrayComparison::Dominates) {
                                onErase(*it);
                                m_set.erase(it++);
                            } else {
                                ++it;
                            }
                        }
                    }
                    inline const Item* addToOpen(COST_VECTOR_T&& v) {return &m_set.emplace_back(std::move(v), true);}
                    inline const Item* addToClosed(COST_VECTOR_T&& v) {return &m_set.emplace_back(std::move(v), false);}
                    static inline void moveToClosed(const Item* item) {item->in_open = false;}
                    static inline void moveToOpen(const Item* item) {item->in_open = true;}
                    static inline bool isOpen(const Item* item) {return item->in_open;}
                    
                    // Checks if any element in the set dominates v
                    inline Containers::ArrayComparison dominates(const COST_VECTOR_T& v) const {
                        for (const auto& item : m_set) {
                            auto result = item.cv.dominates(v);
                            if (result == Containers::ArrayComparison::Dominates) {
                                // The set dominates v
                                return Containers::ArrayComparison::Dominates;
                            } else if (result == Containers::ArrayComparison::Equal) {
                                return Containers::ArrayComparison::Equal;
                            }
                        }
                        return Containers::ArrayComparison::DoesNotDominate;
                    }

                private:
                    // List is used for pointer stability
                    std::list<Item> m_set;
            };

        public:
            std::map<Node, CostSet> cost_map;
    };

    template <class NODE_T, class EDGE_STORAGE_T, class COST_VECTOR_T>
    struct MultiObjectiveSearchResult {
        public:
            MultiObjectiveSearchResult(bool retain_search_graph = true, bool retain_non_dominated_cost_map = true)
                : search_graph(std::make_shared<SearchGraph<SearchGraphEdge<const COST_VECTOR_T*, EDGE_STORAGE_T>, NODE_T>>(true, true))
                , non_dominated_cost_map(std::make_shared<NonDominatedCostMap<COST_VECTOR_T>>())
                , m_retain_search_graph(retain_search_graph)
                , m_retain_non_dominated_cost_map(retain_non_dominated_cost_map)
            {
            }

            bool success = false;
            std::vector<PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>> solution_set;
            std::shared_ptr<SearchGraph<SearchGraphEdge<const COST_VECTOR_T*, EDGE_STORAGE_T>, NODE_T>> search_graph;
            std::shared_ptr<NonDominatedCostMap<COST_VECTOR_T>> non_dominated_cost_map;

            void package() { // Free the memory of the search tree and min cost map if the user desires
                if (!m_retain_search_graph) search_graph.reset();
                if (!m_retain_non_dominated_cost_map) non_dominated_cost_map.reset();
            }
        private:
            bool m_retain_search_graph, m_retain_non_dominated_cost_map;
    };

}
}