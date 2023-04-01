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

    /*
    Specialize the product edge inerhitor for your purpose

    Specialization container must include:
    1. the custom edge type under the alias 'type'
    2. the static method 'type inherit(const MODEL_T::edge_t& model_edge, const std::vector<typename AUTOMATON_T::edge_t>& automaton_edges)'

    Common used examples can be found below
    */


    template <class MODEL_T, class AUTOMATON_T>
    struct CombinedProductEdgeInheritor {
        // Full combined edge type: (model_edge_t, (automaton_edge_1, ...))
        typedef MODEL_T::edge_t::cost_t cost_t;
        typedef MODEL_T::edge_t::action_t action_t;
        struct CombinedEdge {
            MODEL_T::edge_t model_edge; 
            Containers::SizedArray<typename AUTOMATON_T::edge_t> automaton_edge;
            
            // Cost conversion operators
            operator cost_t&() {return static_cast<cost_t&>(model_edge);}
            operator const cost_t&() const {return static_cast<const cost_t&>(model_edge);}
            operator cost_t&&() {return static_cast<cost_t&&>(std::move(model_edge));}

            // Action conversion operators
            operator action_t&() {return static_cast<action_t&>(model_edge);}
            operator const action_t&() const {return static_cast<const action_t&>(model_edge);}
            operator action_t&&() {return static_cast<action_t&&>(std::move(model_edge));}
        };
        typedef CombinedEdge type;

        static inline type inherit(const MODEL_T::edge_t& model_edge, Containers::SizedArray<typename AUTOMATON_T::edge_t>&& automaton_edges) {
            return type{model_edge, automaton_edges};
        }

        static inline cost_t toCost(const type& inherited_edge) {
            return inherited_edge.model_edge_t.toCost();
        }

        static inline cost_t&& toCost(type&& inherited_edge) {
            return inherited_edge.model_edge_t.toCost();
        }
    };

    template <class MODEL_T, class AUTOMATON_T>
    struct ModelEdgeInheritor {
        // Full combined edge type: (model_edge_t, (automaton_edge_1, ...))
        typedef MODEL_T::edge_t type;

        static inline type inherit(const MODEL_T::edge_t& model_edge, Containers::SizedArray<typename AUTOMATON_T::edge_t>&& automaton_edges) {
            return model_edge;
        }

    };

    struct UnwrappedNode {
        UnwrappedNode() = delete;
        UnwrappedNode(ProductRank rank) : automata_nodes(rank - 1) {}
        Node ts_node;
        Containers::SizedArray<Node> automata_nodes;
    };


    template <class MODEL_T, class AUTOMATON_T, class EDGE_INHERITOR = ModelEdgeInheritor<MODEL_T, AUTOMATON_T>>
    class SymbolicProductAutomaton {
        public:

            typedef MODEL_T model_t;
            typedef EDGE_INHERITOR edge_inheritor_t;
            typedef EDGE_INHERITOR::type edge_t;
            typedef WideNode node_t;

        public:
            SymbolicProductAutomaton() = delete;
            SymbolicProductAutomaton(const std::shared_ptr<MODEL_T>& model, const std::vector<std::shared_ptr<AUTOMATON_T>>& automata, uint32_t cache_size = 1);

            // Number of graphs in the product
            inline ProductRank rank() const {return m_graph_sizes.size();};
            inline const Containers::SizedArray<std::size_t>& getGraphSizes() const {return m_graph_sizes;}

            // Graph movement methods
		    std::vector<typename EDGE_INHERITOR::type> getOutgoingEdges(WideNode node);

		    std::vector<WideNode> getChildren(WideNode node);

		    std::vector<typename EDGE_INHERITOR::type> getIncomingEdges(WideNode node);

		    std::vector<WideNode> getParents(WideNode node);

            // Access the graphs
            const MODEL_T& getModel() const {return *m_model;}
            const std::shared_ptr<MODEL_T>& extractModel() const {return m_model;}
            const AUTOMATON_T& getAutomaton(ProductRank i) const {return *(m_automata[i]);}
            const std::vector<std::shared_ptr<AUTOMATON_T>>& extractAutomata() const {return m_automata;}

            // Init and accepting states
            std::vector<WideNode> getInitNodes(Node init_model_node) const;
            std::set<WideNode> getAcceptingNodes() const; 

            // Convert between augmented nodes
            WideNode getWrappedNode(Node ts_node, const Containers::SizedArray<Node>& automata_nodes) const;
            UnwrappedNode getUnwrappedNode(WideNode wrapped_node) const ;

        private:

            enum CacheElementType {Forward, Backward};
            
            template<CacheElementType TYPE>
            static inline constexpr CacheElementType getCacheElementType() {
                if constexpr (TYPE == CacheElementType::Forward) {return CacheElementType::Forward;}
                if constexpr (TYPE == CacheElementType::Backward) {return CacheElementType::Backward;}
            }

            struct AugmentedNodePermutation {
                AugmentedNodePermutation(ProductRank automata_rank) : automaton_set_ind_options(automata_rank) {}
                uint32_t ts_ind_option;
                Containers::SizedArray<std::vector<Node>> automaton_set_ind_options;

                void addOption(ProductRank graph_ind, uint32_t dst_set_ind) {
                    automaton_set_ind_options[graph_ind].push_back(dst_set_ind);
                }

                Containers::SizedArray<uint32_t> getAutomatonOptionsArray() const {
                    Containers::SizedArray<uint32_t> ret_arr(automaton_set_ind_options.size());
                    for (ProductRank i=0; i<automaton_set_ind_options.size(); ++i) ret_arr[i] = automaton_set_ind_options[i].size();
                    return ret_arr;
                }

                // Automaton
                uint32_t getSetIndex(ProductRank automaton_ind, uint32_t option_index) const {return automaton_set_ind_options[automaton_ind][option_index];}

            };

            struct AugmentedNodePermutationArray {
                public:
                    Containers::SizedArray<Node> src_unwrapped_nodes;
                    std::vector<AugmentedNodePermutation> array;

                    AugmentedNodePermutationArray(WideNode src_node, const Containers::SizedArray<std::size_t>& graph_sizes) 
                        : src_unwrapped_nodes(AugmentedNodeIndex::unwrap(src_node, graph_sizes))
                        {}

                    void insert(AugmentedNodePermutation&& perm) {
                        ASSERT(perm.automaton_set_ind_options.size() == src_unwrapped_nodes.size() - 1, "Cannot insert state permutation with incorrect size (" << perm.automaton_set_ind_options.size() << ")");
                        array.push_back(perm);
                    }

                    std::size_t getPermutationSize() const {
                        std::size_t size = 0;
                        for (const auto& perm : array) {
                            size += Algorithms::Combinatorics::permutationsSizeFromOptions(perm.automaton_set_ind_options);
                        }
                        return size; 
                    }

                    inline bool empty() const {return array.empty();}
                    inline void clear() {array.clear();}
            };

            // Symbolic method cache (i.e. for calling 'getOutgoingEdges' right before/after 'children' for the same node)
            class SymbolicCache {
                private:
                    struct CacheElementKey {
                        CacheElementKey(WideNode node_, CacheElementType type_) : node(node_), type(type_) {}
                        WideNode node;
                        CacheElementType type;
                        bool operator<(const CacheElementKey& other) const {return node < other.node;}
                    };
                public:
                    SymbolicCache() = delete;
                    SymbolicCache(const Containers::SizedArray<std::size_t>& graph_sizes, uint32_t max_size = 1) 
                        : m_graph_sizes(graph_sizes)
                        , m_max_size(max_size) {};

                    void clear() {m_cache.clear();}
                    
                    template <CacheElementType CACHE_ELEMENT_T>
                    inline bool has(WideNode node) const {
                        return m_cache.contains(CacheElementKey(node, CACHE_ELEMENT_T));
                    }

                    template <CacheElementType CACHE_ELEMENT_T>
                    inline const AugmentedNodePermutationArray& get(WideNode node) const {return m_cache.at(CacheElementKey(node, CACHE_ELEMENT_T));}

                    template <CacheElementType CACHE_ELEMENT_T>
                    AugmentedNodePermutationArray& make(WideNode node) {
                        // If the cache is at size, erase an element (for simplicity erase the first)
                        if (m_cache.size() >= m_max_size)
                            m_cache.erase(m_cache.begin());
                        auto [it, inserted] = m_cache.try_emplace(CacheElementKey(node, CACHE_ELEMENT_T), node, m_graph_sizes);

                        // If the cache happened to have a value that matched the node, clear it
                        if (!inserted)
                            it->second.clear();

                        return it->second;
                    }

                private:
                    uint32_t m_max_size = 1;
                    std::map<CacheElementKey, AugmentedNodePermutationArray> m_cache;
                    const Containers::SizedArray<std::size_t>& m_graph_sizes;
            };

        private:
            const AugmentedNodePermutationArray& pre(WideNode node) const;
            const AugmentedNodePermutationArray& post(WideNode node) const;

        private:
            const std::shared_ptr<MODEL_T> m_model;
            const std::vector<std::shared_ptr<AUTOMATON_T>> m_automata;
            Containers::SizedArray<std::size_t> m_graph_sizes;

            mutable SymbolicCache m_cache;
    };

} // namespace DiscreteModel
} // namespace TP

#include "SymbolicProductAutomaton_impl.hpp"