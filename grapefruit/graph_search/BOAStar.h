#pragma once

#include "graph_search/SearchProblem.h"
#include "graph_search/MultiObjectiveSearchProblem.h"

#define GF_COST_VECTOR_EQUIVALENCE_TOLERANCE 0.0000000001



namespace GF {

namespace GraphSearch {

    template <typename NODE_T, typename ENUMERATED_NODE_T, class EDGE_STORAGE_T>
    class PathEnumeratedNodeMap {
        private:
            struct Value {
                Value() = delete;
                Value(NODE_T node_, ENUMERATED_NODE_T parent_, EDGE_STORAGE_T parent_edge_) : node(node_), parent(parent_), parent_edge(parent_edge_) {}
                NODE_T node;
                ENUMERATED_NODE_T parent;
                EDGE_STORAGE_T parent_edge;
            };
        public:
            inline bool isInit(ENUMERATED_NODE_T enumerated_node) const {return m_init_nodes.contains(enumerated_node);}
            inline NODE_T getNode(ENUMERATED_NODE_T enumerated_node) const {return (isInit(enumerated_node)) ? m_init_nodes.at(enumerated_node) : m_map.at(enumerated_node).node;}
            inline ENUMERATED_NODE_T getParentEnumeratedNode(ENUMERATED_NODE_T enumerated_node) const {return m_map.at(enumerated_node).parent;}

            inline const EDGE_STORAGE_T& getParentEdge(ENUMERATED_NODE_T enumerated_node) const {return m_map.at(enumerated_node).parent_edge;}
            inline ENUMERATED_NODE_T newNode(NODE_T node, ENUMERATED_NODE_T parent, const EDGE_STORAGE_T& parent_edge) {
                m_map.try_emplace(m_next_node, node, parent, parent_edge);
                return m_next_node++;
            }
            inline ENUMERATED_NODE_T newNode(NODE_T node, ENUMERATED_NODE_T parent, EDGE_STORAGE_T&& parent_edge) {
                m_map.try_emplace(m_next_node, node, parent, std::move(parent_edge));
                return m_next_node++;
            }
            inline ENUMERATED_NODE_T newInitNode(NODE_T init_node) {
                m_init_nodes.try_emplace(m_next_node, init_node);
                return m_next_node++;
            }

            void extractPath(ENUMERATED_NODE_T goal_node,std::vector<NODE_T>& node_path, std::vector<EDGE_STORAGE_T>& edge_path);

        private:
            ENUMERATED_NODE_T m_next_node = ENUMERATED_NODE_T{};
            std::map<ENUMERATED_NODE_T, Value> m_map; // Maps enum node key to the actual node and the parent enum node
            std::map<ENUMERATED_NODE_T, NODE_T> m_init_nodes;
    };

    template <class COST_VECTOR_T, class SEARCH_PROBLEM_T, typename EDGE_STORAGE_T = typename SEARCH_PROBLEM_T::edge_t>
    class BOAStar {
        public:
            using GraphNode = SEARCH_PROBLEM_T::node_t;
            typedef SEARCH_PROBLEM_T::edge_t edge_t;
            typedef SEARCH_PROBLEM_T::cost_t cost_t;
            using EnumeratedNode = Node;

        public:
            static MultiObjectiveSearchResult<GraphNode, EDGE_STORAGE_T, COST_VECTOR_T> search(const SEARCH_PROBLEM_T& problem);

            

        private:
            static void extractPath(const EnumeratedNode& goal_node, PathSolution<GraphNode, EDGE_STORAGE_T>& path_solution, const PathEnumeratedNodeMap<GraphNode, EnumeratedNode, EDGE_STORAGE_T>& node_map);

            //template <typename RETURN_T, typename... ARGS_T>
            //RETURN_T returnVal(RETURN_T(ARGS_T...));
    };

}
}

#include "BOAStar_impl.hpp"