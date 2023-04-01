#pragma once

#include <memory>

#include "SymbolicProductAutomaton.h"

#include "tools/Containers.h"
#include "tools/Algorithms.h"
#include "core/Automaton.h"
#include "core/Graph.h"
#include "core/TransitionSystem.h"

namespace TP {
namespace DiscreteModel {

    template <class MODEL_T, class AUTOMATON_T, class EDGE_INHERITOR>
    SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>::SymbolicProductAutomaton(const std::shared_ptr<MODEL_T>& model, const std::vector<std::shared_ptr<AUTOMATON_T>>& automata, uint32_t cache_size) 
        : m_model(model)
        , m_automata(automata)
        , m_graph_sizes(m_automata.size() + 1)
        , m_cache(m_graph_sizes, cache_size)
    {
        for (ProductRank i=0; i < rank(); ++i) m_graph_sizes[i] = (i != 0) ? m_automata[i-1]->size() : model->size();
    }

    template <class MODEL_T, class AUTOMATON_T, class EDGE_INHERITOR>
    WideNode SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>::getWrappedNode(Node ts_node, const Containers::SizedArray<Node>& automata_nodes) const {
        Containers::SizedArray<Node> unwrapped_node(rank());
        for (uint32_t i=0; i<unwrapped_node.size(); ++i) {
            if (i != 0) {
                unwrapped_node[i] = automata_nodes[i - 1];
            } else {
                unwrapped_node[0] = ts_node;
            }
        }
        return AugmentedNodeIndex::wrap(unwrapped_node, m_graph_sizes);
    }

    template <class MODEL_T, class AUTOMATON_T, class EDGE_INHERITOR>
    SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>::UnwrappedNode SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>::getUnwrappedNode(WideNode wrapped_node) const {
        UnwrappedNode unwrapped_node_converted(*this);
        auto unwrapped_node = AugmentedNodeIndex::unwrap(wrapped_node, m_graph_sizes);
        for (uint32_t i=0; i<unwrapped_node.size(); ++i) {
            if (i != 0) {
                unwrapped_node_converted.automata_nodes[i - 1] = unwrapped_node[i];
            } else {
                unwrapped_node_converted.ts_node = unwrapped_node[0];
            }
        }
        return unwrapped_node_converted;
    }

    template <class MODEL_T, class AUTOMATON_T, class EDGE_INHERITOR>
    std::vector<WideNode> SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>::getInitNodes(Node init_model_node) const {
        AugmentedNodePermutation perm(rank() - 1);
        for (ProductRank i=0; i<m_automata.size(); ++i) {
            for (Node n : m_automata[i]->getInitStates()) {
                perm.addOption(i, n);
            }
        }
        std::vector<WideNode> init_nodes;
        init_nodes.reserve(Algorithms::Combinatorics::permutationsSizeFromOptions(perm.automaton_set_ind_options));
        auto onPermutation = [&] (const Containers::SizedArray<uint32_t>& option_indices) {
            Containers::SizedArray<Node> unwrapped_p(rank());
            unwrapped_p[0] = init_model_node;

            for (ProductRank i=1; i<rank(); ++i) {
                unwrapped_p[i] = perm.getSetIndex(i - 1, option_indices[i - 1]);
            }
            init_nodes.push_back(AugmentedNodeIndex::wrap(unwrapped_p, this->m_graph_sizes));
        };

        Algorithms::Combinatorics::permutations(perm.getAutomatonOptionsArray(), onPermutation);

        return init_nodes;
    }

    template <class MODEL_T, class AUTOMATON_T, class EDGE_INHERITOR>
    std::set<WideNode> SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>::getAcceptingNodes() const {
        Containers::SizedArray<std::vector<Node>> options(rank());
        options[0] = m_model->nodes();
        for (ProductRank i=0; i<rank()-1; ++i) {
            const auto& accepting_states = m_automata[i]->getAcceptingStates();
            options[i + 1].reserve(accepting_states.size());
            for (auto s : accepting_states) { options[i + 1].push_back(s); }
        }
        
        std::set<WideNode> accepting_states;
        auto onPermutation = [&] (const Containers::SizedArray<uint32_t>& option_indices) {
            Containers::SizedArray<Node> unwrapped_p(rank());
            unwrapped_p[0] = options[0][option_indices[0]];

            for (ProductRank i=1; i<rank(); ++i) {
                unwrapped_p[i] = options[i][option_indices[i]];
            }
            accepting_states.insert(AugmentedNodeIndex::wrap(unwrapped_p, this->m_graph_sizes));
        };

        Containers::SizedArray<uint32_t> n_options_arr(rank());
        for (uint32_t i=0; i< rank(); ++i) n_options_arr[i] = options[i].size();
        Algorithms::Combinatorics::permutations(n_options_arr, onPermutation);

        return accepting_states;
    }

    template <class MODEL_T, class AUTOMATON_T, class EDGE_INHERITOR>
    std::vector<typename EDGE_INHERITOR::type> SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>::getOutgoingEdges(WideNode node) {
        const AugmentedNodePermutationArray& perm_array = post(node);

        std::vector<typename EDGE_INHERITOR::type> product_outgoing_edges;
        if (perm_array.empty()) return product_outgoing_edges;

        product_outgoing_edges.reserve(perm_array.getPermutationSize());

        const auto& unwrapped_nodes = perm_array.src_unwrapped_nodes;

        for (const auto& perm : perm_array.array) {
            auto onPermutation = [&, this] (const Containers::SizedArray<uint32_t>& option_indices) {
                const auto& model_edge = m_model->getOutgoingEdges(unwrapped_nodes[0])[perm.ts_ind_option];

                Containers::SizedArray<typename AUTOMATON_T::edge_t> automaton_edges(rank() - 1);
                for (ProductRank i=1; i<rank(); ++i) {
                    const auto& outgoing_edges = m_automata[i - 1]->getOutgoingEdges(unwrapped_nodes[i]);
                    automaton_edges[i-1] = outgoing_edges[perm.getSetIndex(i - 1, option_indices[i - 1])];
                }

                product_outgoing_edges.push_back(EDGE_INHERITOR::inherit(model_edge, std::move(automaton_edges)));
            };

            const auto& n_options = perm.getAutomatonOptionsArray();
            Algorithms::Combinatorics::permutations(n_options, onPermutation);
        }
        
        return product_outgoing_edges;
    }
	
    template <class MODEL_T, class AUTOMATON_T, class EDGE_INHERITOR>
    std::vector<WideNode> SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>::getChildren(WideNode node) {
        const AugmentedNodePermutationArray& perm_array = post(node);

        std::vector<WideNode> product_children;
        if (perm_array.empty()) return product_children;

        product_children.reserve(perm_array.getPermutationSize());

        const auto& unwrapped_nodes = perm_array.src_unwrapped_nodes;

        for (const auto& perm : perm_array.array) {
            auto onPermutation = [&, this] (const Containers::SizedArray<uint32_t>& option_indices) {
                Containers::SizedArray<Node> unwrapped_pp(rank());
                unwrapped_pp[0] = m_model->getChildren(unwrapped_nodes[0])[perm.ts_ind_option];

                for (ProductRank i=1; i<rank(); ++i) {
                    const auto& children = m_automata[i - 1]->getChildren(unwrapped_nodes[i]);
                    unwrapped_pp[i] = children[perm.getSetIndex(i - 1, option_indices[i - 1])];
                }

                product_children.push_back(AugmentedNodeIndex::wrap(unwrapped_pp, this->m_graph_sizes));
            };

            const auto& n_options = perm.getAutomatonOptionsArray();
            Algorithms::Combinatorics::permutations(n_options, onPermutation);
        }
        
        return product_children;
    }
	
    template <class MODEL_T, class AUTOMATON_T, class EDGE_INHERITOR>
    std::vector<typename EDGE_INHERITOR::type> SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>::getIncomingEdges(WideNode node) {
        const AugmentedNodePermutationArray& perm_array = pre(node);

        std::vector<typename EDGE_INHERITOR::type> product_incoming_edges;
        if (perm_array.empty()) return product_incoming_edges;

        ASSERT(perm_array.array.size() == 1, "Cache element for pre contains does not contain one AugmentedNodePermutation");
        const auto perm = perm_array.array[0];

        product_incoming_edges.reserve(perm_array.getPermutationSize());

        const auto& unwrapped_nodes = perm_array.src_unwrapped_nodes;

        const auto& model_parents = m_model->getParents(unwrapped_nodes[0]);

        uint32_t model_parent_ind = 0;
        for (auto model_parent : model_parents) {

            const auto& model_edge = m_model->getIncomingEdges(unwrapped_nodes[0])[model_parent_ind++];

            auto onPermutation = [&, this] (const Containers::SizedArray<uint32_t>& option_indices) {
                Containers::SizedArray<Node> unwrapped_pp(rank());
                unwrapped_pp[0] = model_parent;

                Containers::SizedArray<typename AUTOMATON_T::edge_t> automaton_edges(rank() - 1);
                for (ProductRank i=1; i<rank(); ++i) {
                    const auto& incoming_edges = m_automata[i - 1]->getIncomingEdges(unwrapped_nodes[i]);
                    automaton_edges[i-1] = incoming_edges[perm.getSetIndex(i - 1, option_indices[i - 1])];
                }

                product_incoming_edges.push_back(EDGE_INHERITOR::inherit(model_edge, std::move(automaton_edges)));
            };

            const auto& n_options = perm.getAutomatonOptionsArray();
            Algorithms::Combinatorics::permutations(n_options, onPermutation);
        }
        
        return product_incoming_edges;
    }
		
    template <class MODEL_T, class AUTOMATON_T, class EDGE_INHERITOR>
    std::vector<WideNode> SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>::getParents(WideNode node) {
        const AugmentedNodePermutationArray& perm_array = pre(node);

        std::vector<WideNode> product_parents;
        if (perm_array.empty()) return product_parents;

        ASSERT(perm_array.array.size() == 1, "Cache element for pre contains does not contain one AugmentedNodePermutation");
        const auto perm = perm_array.array[0];

        product_parents.reserve(perm_array.getPermutationSize());

        const auto& unwrapped_nodes = perm_array.src_unwrapped_nodes;

        const auto& model_parents = m_model->getParents(unwrapped_nodes[0]);

        for (auto model_parent : model_parents) {
            auto onPermutation = [&, this] (const Containers::SizedArray<uint32_t>& option_indices) {
                Containers::SizedArray<Node> unwrapped_pp(rank());
                unwrapped_pp[0] = model_parent;

                for (ProductRank i=1; i<rank(); ++i) {
                    const auto& automaton_parents = m_automata[i - 1]->getParents(unwrapped_nodes[i]);
                    unwrapped_pp[i] = automaton_parents[perm.getSetIndex(i - 1, option_indices[i - 1])];
                }

                product_parents.push_back(AugmentedNodeIndex::wrap(unwrapped_pp, this->m_graph_sizes));
            };

            const auto& n_options = perm.getAutomatonOptionsArray();
            Algorithms::Combinatorics::permutations(n_options, onPermutation);
        }
        
        return product_parents;
    }
		
    template <class MODEL_T, class AUTOMATON_T, class EDGE_INHERITOR>
    const SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>::AugmentedNodePermutationArray& SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>::post(WideNode node) const {
        // Retrieve from cache if it has the node
        if (m_cache.template has<CacheElementType::Forward>(node)) return m_cache.template get<CacheElementType::Forward>(node);

        AugmentedNodePermutationArray& perm_array = m_cache.template make<CacheElementType::Forward>(node);

        // Unwrap the product node (done automatically when constructing cache element)
        const Containers::SizedArray<Node>& unwrapped_nodes = perm_array.src_unwrapped_nodes;

        // Get the children of the model
        const auto& model_children = m_model->getChildren(unwrapped_nodes[0]);
        const auto& model_outgoing_edges = m_model->getOutgoingEdges(unwrapped_nodes[0]);

        uint32_t sp_ind = 0;
        for (auto sp : model_children) {
            bool enabled = true;

            // Construct a temporary permutation
            AugmentedNodePermutation perm(rank() - 1);

            //LOG("state: " << m_model->getGenericNodeContainer()[sp].to_str());
            ProductRank automaton_ind = 1;
            for (const auto& automaton : m_automata) {
                bool transition_enabled = false;
                const auto& automaton_children = automaton->getChildren(unwrapped_nodes[automaton_ind]);
                const auto& automaton_outgoing_edges = automaton->getOutgoingEdges(unwrapped_nodes[automaton_ind]);

                //LOG("working automaton " << automaton_ind);
                for (uint32_t i=0; i<automaton_children.size(); ++i) {
                    //LOG("observing edge: " << static_cast<const FormalMethods::Observation&>(automaton_outgoing_edges[i]));
                    if (m_model->observe(sp, static_cast<const FormalMethods::Observation&>(automaton_outgoing_edges[i]))) {
                        //LOG(" found! adding option for child: " << automaton_children[i]);
                        perm.addOption(automaton_ind - 1, i);
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
                perm.ts_ind_option = sp_ind;
                perm_array.insert(std::move(perm));
            } 
            ++sp_ind;
        }

        return perm_array;
    }

    template <class MODEL_T, class AUTOMATON_T, class EDGE_INHERITOR>
    const SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>::AugmentedNodePermutationArray& SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>::pre(WideNode node) const {
        // Retrieve from cache if it has the node
        if (m_cache.template has<CacheElementType::Backward>(node)) return m_cache.template get<CacheElementType::Backward>(node);

        AugmentedNodePermutationArray& perm_array = m_cache.template make<CacheElementType::Backward>(node);

        // Unwrap the product node (done automatically when constructing cache element)
        const Containers::SizedArray<Node>& unwrapped_nodes = perm_array.src_unwrapped_nodes;
        Node sp = unwrapped_nodes[0];

        // Get the children of the model
        const auto& model_parents = m_model->getParents(unwrapped_nodes[0]);
        const auto& model_incoming_edges = m_model->getIncomingEdges(unwrapped_nodes[0]);

        // Observation only depends current state, so no iteration over model parents is needed
        bool enabled = true;
        
        // Construct a temporary permutation
        AugmentedNodePermutation perm(rank() - 1);

        ProductRank automaton_ind = 1;
        for (const auto& automaton : m_automata) {
            bool transition_enabled = false;
            const auto& automaton_parents = automaton->getParents(unwrapped_nodes[automaton_ind]);
            const auto& automaton_incoming_edges = automaton->getIncomingEdges(unwrapped_nodes[automaton_ind]);

            for (uint32_t i=0; i<automaton_parents.size(); ++i) {
                if (m_model->observe(sp, automaton_incoming_edges[i])) {
                    perm.addOption(automaton_ind - 1, i);
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
            perm_array.insert(std::move(perm));
        } 

        return perm_array;
    }


} // namespace DiscreteModel
} // namespace TP