#pragma once

#include "core/Graph.h"

namespace TP {

template<class EDGE_T, typename NATIVE_NODE_T = Node>
class DirectedAcyclicGraph : public Graph<EDGE_T, NATIVE_NODE_T> {
    public:
        DirectedAcyclicGraph(Graph<EDGE_T, NATIVE_NODE_T>::EdgeToStrFunction edgeToStr = nullptr) : Graph<EDGE_T, NATIVE_NODE_T>(true, true, edgeToStr) {}

        bool testConnection(NATIVE_NODE_T src, NATIVE_NODE_T dst) const {
            if (!m_test_node_cache.empty && src == m_test_node_cache.src && dst == m_test_node_cache.dst) {
                return m_test_node_cache.result;
            }
            if (src < this->m_graph.size()) {
                std::vector<NATIVE_NODE_T> stack = this->getParents(src);

                while (!stack.empty()) {
                    NATIVE_NODE_T p = stack.back();
                    if (p == dst) {
                        m_test_node_cache.set(false, src, dst, false);
                        return false;
                    }
                    stack.pop_back();
                    
                    const auto& parents = this->getParents(p);
                    
                    stack.insert(stack.begin(), parents.begin(), parents.end());
                }
            }
            m_test_node_cache.set(false, src, dst, true);
            return true;
        }

        virtual bool connect(NATIVE_NODE_T src, NATIVE_NODE_T dst, const EDGE_T& edge) override{
            if (!testConnection(src, dst)) return false;
            this->Graph<EDGE_T, NATIVE_NODE_T>::connect(src, dst, edge);
            return true;
        }

        void removeDeadLeaves(NATIVE_NODE_T dead_leaf) {
            ASSERT(this->m_reversible, "Cannot remove dead leaves on irreversible graphs");

            std::vector<NATIVE_NODE_T> dead_leaf_stack = {dead_leaf};
            while (!dead_leaf_stack.empty()) {
                
                // Child is dead leaf
                NATIVE_NODE_T curr_dead_leaf = dead_leaf_stack.back();
                dead_leaf_stack.pop_back();

                for (auto par : this->getParents(curr_dead_leaf)) {

                    // If the parent is only leads to the dead leaf, it itself is dead
                    if (this->getChildren(par).size() == 1) {
                        dead_leaf_stack.push_back(par);
                    }

                    // Disconnect the dead leaf from the search graph
                    this->disconnect(par, curr_dead_leaf);
                }
            }
        }

    private:
        struct CacheEntry {
            bool empty = true;
            NATIVE_NODE_T src;
            NATIVE_NODE_T dst;
            bool result;
            void set(bool empty_, NATIVE_NODE_T src_, NATIVE_NODE_T dst_, bool result_) {empty = empty_; src = src_; dst = dst_; result = result_;}
        };
        mutable CacheEntry m_test_node_cache;
};

// TODO
//template<class NODE_T, class EDGE_T>
//class NodeGenericDirectedAcyclicGraph : public NodeGenericGraph<NODE_T, EDGE_T> {
//    public: 
//        NodeGenericDirectedAcyclicGraph(Graph<EDGE_T>::EdgeToStrFunction edgeToStr = nullptr, NodeGenericGraph<NODE_T, EDGE_T>::NodeToStrFunction nodeToStr = nullptr) 
//            : Graph<EDGE_T>(true, true, edgeToStr, nodeToStr) {}
//
//
//        // This method checks connection using the native nodes
//        virtual bool connect(Node src, Node dst, const EDGE_T& edge) override{
//
//            std::vector<Node> stack = this->getParents(dst);
//
//            while (!stack.empty()) {
//                Node p = stack.back();
//                if (p == src) return false;
//                stack.pop_back();
//                
//                const auto& parents = this->getParents(p);
//                
//                stack.insert(stack.begin(), parents.begin(), parents.end());
//            }
//            Graph<EDGE_T>::connect(src, dst, edge);
//            return true;
//        }
//
//        // This method checks connection using the generic nodes
//        virtual bool connect(NODE_T src, NODE_T dst, const EDGE_T& edge) override{
//
//            std::vector<NODE_T> stack = this->getParentsGenericNodes(src);
//
//            while (!stack.empty()) {
//                NODE_T p = stack.back();
//                if (p == src) return false;
//                stack.pop_back();
//                
//                const auto& parents = this->getParentsGenericNodes(p);
//                
//                stack.insert(stack.begin(), parents.begin(), parents.end());
//            }
//            NodeGenericGraph<NODE_T, EDGE_T>::connect(src, dst, edge);
//            return true;
//        }
//
//};
    
} // namespace TP
