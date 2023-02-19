#pragma once

#include <memory>

#include "tools/Containers.h"
#include "tools/Algorithms.h"
#include "core/Automaton.h"
#include "core/Graph.h"
#include "core/TransitionSystem.h"

namespace TP {
namespace DiscreteModel {

    using ProductRank = uint8_t;

    template <class MODEL_T, class AUTOMATON_T>
    class SymbolicProductAutomaton {
        // Dependent types
        public:
            // Full combined edge type: (model_edge_t, (automaton_edge_1, ...))
            typedef struct {MODEL_T::edge_t model_edge_t; std::vector<typename AUTOMATON_T::edge_t> automaton_edge_t;} combined_edge_t;

        public:
            SymbolicProductAutomaton() = delete;
            SymbolicProductAutomaton(const std::shared_ptr<MODEL_T>& model, const std::vector<std::shared_ptr<AUTOMATON_T>>& automata, uint32_t cache_size = 1) 
                : m_model(model)
                , m_automata(automata)
                , m_graph_sizes(m_automata.size() + 1)
                , m_cache(cache_size)
            {
                for (ProductRank i=0; i < rank(); ++i) m_graph_sizes[i] = (i != 0) ? m_automata[i]->size() : model->size();
            }

            // Number of graphs in the product
            inline ProductRank rank() const {return m_automata.size() + 1;};
            inline const Containers::SizedArray<std::size_t>& getGraphSizes() const {return m_graph_sizes;}

            // This method should be specialized for the desired application
            template <class EDGE_T>
            EDGE_T getEdge(const MODEL_T::edge_t& model_edge, const std::vector<typename AUTOMATON_T::edge_t>& automaton_edges);

            template <class EDGE_T>
		    std::vector<EDGE_T> getOutgoingEdges(WideNode node) {

		    }

		    std::vector<WideNode> getChildren(WideNode node) {
                const StatePermutaton& perm = post(node);
                const auto& n_options = perm.getOptionsArray();
                const auto& unwrapped_nodes = perm.unwrapped_nodes;

                std::vector<WideNode> children(Algorithms::Combinatorics::permutationsSize(n_options));

                uint32_t child_ind = 0;
                auto onPermutation = [this] (const Containers::SizedArray<uint32_t>& option_indices) {
                    Containers::SizedArray<Node> unwrapped_pp(rank());

                    for (ProductRank i=0; i<rank(); ++i) {
                        if (i != 0)  {
                            const auto& children = m_automata[i - 1]->getChildren(unwrapped_nodes[i]);
                            unwrapped_pp[i] == children[option_indices[i]];
                        } else {
                            const auto& children = m_model->getChildren(unwrapped_nodes[i]);
                            unwrapped_pp[i] == children[option_indices[i]];
                        }
                    }

                    children[child_ind++] = AugmentedNodeIndex::wrap(unwrapped_pp, this->m_graph_sizes);
                };

                Algorithms::Combinatorics::permutations(n_options, onPermutation);
                
                return children;
		    }
		
            template <class EDGE_T>
		    std::vector<EDGE_T> getIncomingEdges(WideNode node) {
		    }

		    std::vector<WideNode> getParents(WideNode node) {
		    }

        private:
            struct StatePermutaton {
                Containers::SizedArray<Node> unwrapped_nodes;
                Containers::SizedArray<std::vector<Node>> dst_set_ind_options;

                StatePermutaton(Node node) : unwrapped_nodes(AugmentedNodeIndex::unwrap(node, m_graph_sizes)), dst_set_ind_options(rank()) {}

                Containers::SizedArray<uint32_t> getOptionsArray() const {
                    Containers::SizedArray<uint32_t> ret_arr(dst_set_ind_options.size());
                    for (ProductRank i=0; i<dst_set_ind_options.size(); ++i) ret_arr[i] = dst_set_ind_options[i].size();
                    return ret_arr;
                }

                void addOption(ProductRank graph_ind, uint32_t dst_set_ind) {
                    dst_set_ind_options[graph_ind].push_back(dst_set_ind);
                }
            };

            // Symbolic method cache (i.e. for calling 'getOutgoingEdges' right before/after 'children' for the same node)
            class SymbolicCache {
                public:
                    SymbolicCache(uint32_t max_size = 1) : m_max_size(max_size) {};
                    void clear() {m_cache.clear();}
                    bool has(WideNode node) const {return m_cache.contains(node);}
                    const StatePermutaton& get(WideNode node) const {
                        ASSERT(has(node), "Undefined behavior calling 'get' when 'has' returns false for a node");
                        return m_cache.at(node);
                    }

                    StatePermutaton& make(WideNode node) {
                        // If the cache is at size, erase an element (for simplicity erase the first)
                        if (m_cache.size() >= m_max_size) m_cache.erase(m_cache.begin());
                        StatePermutaton& fresh_value = *(m_cache.try_emplace(node, node).first);

                        //TODO
                        //// If the cache happened to have a value that matched the node, clear it
                        //fresh_value.clear();
                        return fresh_value;
                    }
                private:
                    uint32_t m_max_size = 1;
                    std::map<WideNode, StatePermutaton> m_cache;
            };

        private:
            const StatePermutaton& post(WideNode node) const {
                // Retrieve from cache if it has the node
                if (m_cache.has(node)) return m_cache.get(node);

                StatePermutaton& perm = m_cache.make(node);

                // Unwrap the product node (done automatically when constructing cache element)
                const Containers::SizedArray<Node>& unwrapped_nodes = perm.unwrapped_nodes;

                // Get the children of the model
                const auto& model_children = m_model->getChildren(unwrapped_nodes[0]);
                const auto& model_outgoing_edges = m_model->getOutgoingEdges(unwrapped_nodes[0]);

                uint32_t sp_ind = 0;
                for (auto sp : model_children) {
                    bool enabled = true;


                    ProductRank automaton_ind = 0;
                    for (const auto& automaton : m_automata) {
                        bool transition_enabled = false;
                        const auto &automaton_children = automaton->getChildren(unwrapped_nodes[automaton_ind]);
                        const auto &automaton_outgoing_edges = automaton->getOutgoingEdges(unwrapped_nodes[automaton_ind]);

                        for (uint32_t i=0; i<automaton_children.size(); ++i) {
                            if (m_model->observe(sp, automaton_outgoing_edges[i])) {
                                perm.addOption(automaton_ind + 1, i);
                                transition_enabled = true;
                            }
                        }
                        
                        if (!transition_enabled) {
                            enabled = false;
                            break;
                        }

                        ++automaton_ind;
                    }

                    if (enabled) {
                        perm.addOption(0, sp_ind);
                    } 

                    ++sp_ind;
                }

                return perm;
            }

        private:
            const std::shared_ptr<MODEL_T> m_model;
            const std::vector<std::shared_ptr<AUTOMATON_T>> m_automata;
            Containers::SizedArray<std::size_t> m_graph_sizes;

            mutable SymbolicCache m_cache;
    };

} // namespace DiscreteModel
} // namespace TP