#pragma once

#include <map>

#include "core/Graph.h"
//#include "Graph.h"

namespace TP {

template<class EDGE_T, typename NATIVE_NODE_T = Node>
class DirectedAcyclicGraph : public Graph<EDGE_T, NATIVE_NODE_T, true> {
    public:
        DirectedAcyclicGraph() {}

        inline bool testConnection(NATIVE_NODE_T src, NATIVE_NODE_T dst) const {
            return hasParent(src, dst);
        }

        virtual bool connect(NATIVE_NODE_T src, NATIVE_NODE_T dst, const EDGE_T& edge) override {
            if (!testConnection(src, dst)) return false;
            this->Graph<EDGE_T, NATIVE_NODE_T>::connect(src, dst, edge);
            m_parents[dst] = m_parents.at(src);
            return true;
        }

        void removeDeadLeaves(NATIVE_NODE_T dead_leaf) {

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
        bool hasParent(NATIVE_NODE_T src, NATIVE_NODE_T parent) const {
            auto it = m_parents.find(src);
            if (it == m_parents.end()) return false;

            const std::vector<bool>& has_parent = it->second;

            if (parent < has_parent.size()) {
                return has_parent[parent];
            }
            return false;
        }

        void addParent(NATIVE_NODE_T node, NATIVE_NODE_T parent) {
            std::vector<bool>& pars = m_parents.at(node);
            if (parent < pars.size()) {
                pars[parent] = true;
            } else {
                pars.resize(parent + 1, false);
                pars[parent] = true;
            }
        }

    private:
        std::map<NATIVE_NODE_T, std::vector<bool>> m_parents;
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
