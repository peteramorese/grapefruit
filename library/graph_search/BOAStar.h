#pragma once

#include "graph_search/SearchProblem.h"
#include "graph_search/MultiObjectiveSearchProblem.h"

#define TP_COST_VECTOR_EQUIVALENCE_TOLERANCE 0.0000000001



namespace TP {

namespace GraphSearch {


    template <class COST_VECTOR_T, class SEARCH_PROBLEM_T, class HEURISTIC_T = ZeroHeuristic<Node, COST_VECTOR_T>, typename EDGE_STORAGE_T = typename SEARCH_PROBLEM_T::edge_t>
    class BOAStar {
        public:
            using GraphNode = SEARCH_PROBLEM_T::node_t;
            typedef SEARCH_PROBLEM_T::edge_t edge_t;
            typedef SEARCH_PROBLEM_T::cost_t cost_t;

        public:
            static MultiObjectiveSearchResult<GraphNode, EDGE_STORAGE_T, COST_VECTOR_T> search(const SEARCH_PROBLEM_T& problem);

        private:
            using EnumeratedNode = GraphNode;
            
            class PathEnumeratedNodeMap {
                private:
                    struct Value {
                        Value() = delete;
                        Value(GraphNode node_, EnumeratedNode parent_, EDGE_STORAGE_T parent_edge_) : node(node_), parent(parent_), parent_edge(parent_edge_) {}
                        GraphNode node;
                        EnumeratedNode parent;
                        EDGE_STORAGE_T parent_edge;
                    };
                public:
                    inline bool isInit(EnumeratedNode enumerated_node) const {return m_init_nodes.contains(enumerated_node);}
                    inline GraphNode getNode(EnumeratedNode enumerated_node) const {
                        return (isInit(enumerated_node)) ? m_init_nodes.at(enumerated_node) : m_map.at(enumerated_node).node;
                    }
                    inline EnumeratedNode getParentEnumeratedNode(EnumeratedNode enumerated_node) const {return m_map.at(enumerated_node).parent;}
                    inline const EDGE_STORAGE_T& getParentEdge(EnumeratedNode enumerated_node) const {return m_map.at(enumerated_node).parent_edge;}
                    inline EnumeratedNode newNode(GraphNode node, EnumeratedNode parent, const EDGE_STORAGE_T& parent_edge) {
                        m_map.try_emplace(m_next_node, node, parent, parent_edge);
                        return m_next_node++;
                    }
                    inline EnumeratedNode newInitNode(GraphNode init_node) {
                        m_init_nodes.try_emplace(m_next_node, init_node);
                        return m_next_node++;
                    }
                private:
                    EnumeratedNode m_next_node = EnumeratedNode{};
                    std::map<EnumeratedNode, Value> m_map; // Maps enum node key to the actual node and the parent enum node
                    std::map<EnumeratedNode, GraphNode> m_init_nodes;
            };

        private:
            static void extractPath(const EnumeratedNode& goal_node, PathSolution<GraphNode, EDGE_STORAGE_T, COST_VECTOR_T>& path_solution, const PathEnumeratedNodeMap& node_map);

            //template <typename RETURN_T, typename... ARGS_T>
            //RETURN_T returnVal(RETURN_T(ARGS_T...));
    };

}
}

#include "BOAStar_impl.hpp"