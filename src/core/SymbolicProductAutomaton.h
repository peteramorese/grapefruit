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
            SymbolicProductAutomaton(const std::shared_ptr<MODEL_T>& model, const std::vector<std::shared_ptr<AUTOMATON_T>>& automata, uint32_t cache_size = 1);

            // Number of graphs in the product
            inline ProductRank rank() const {return m_graph_sizes.size();};
            inline const Containers::SizedArray<std::size_t>& getGraphSizes() const {return m_graph_sizes;}

            // This method should be specialized for the desired application
            template <class EDGE_T>
            EDGE_T getEdge(const MODEL_T::edge_t& model_edge, const std::vector<typename AUTOMATON_T::edge_t>& automaton_edges);

            template <class EDGE_T>
		    std::vector<EDGE_T> getOutgoingEdges(WideNode node);

		    std::vector<WideNode> getChildren(WideNode node);

            template <class EDGE_T>
		    std::vector<EDGE_T> getIncomingEdges(WideNode node);

		    std::vector<WideNode> getParents(WideNode node);

        private:

            enum CacheElementType {Forward, Backward};
            
            template<CacheElementType TYPE>
            static inline constexpr CacheElementType getCacheElementType() {
                if constexpr (TYPE == CacheElementType::Forward) {return CacheElementType::Forward;}
                if constexpr (TYPE == CacheElementType::Backward) {return CacheElementType::Backward;}
            }

            struct AugmentedNodePermutationArray {
                public:
                    struct AugmentedNodePermutation {
                        AugmentedNodePermutation(ProductRank automata_rank) : dst_automaton_set_ind_options(automata_rank) {}
                        Node dst_ts_node;
                        Containers::SizedArray<std::vector<Node>> dst_automaton_set_ind_options;

                        void addOption(ProductRank graph_ind, uint32_t dst_set_ind) {
                            dst_automaton_set_ind_options[graph_ind].push_back(dst_set_ind);
                        }

                        Containers::SizedArray<uint32_t> getAutomatonOptionsArray() const {
                            Containers::SizedArray<uint32_t> ret_arr(dst_automaton_set_ind_options.size());
                            for (ProductRank i=0; i<dst_automaton_set_ind_options.size(); ++i) ret_arr[i] = dst_automaton_set_ind_options[i].size();
                            return ret_arr;
                        }

                        uint32_t getSetIndex(ProductRank automaton_ind, uint32_t option_index) const {return dst_automaton_set_ind_options[automaton_ind][option_index];}
                    };
                public:
                    Containers::SizedArray<Node> src_unwrapped_nodes;
                    std::vector<AugmentedNodePermutation> array;

                    AugmentedNodePermutationArray(WideNode src_node, const Containers::SizedArray<std::size_t>& graph_sizes) 
                        : src_unwrapped_nodes(AugmentedNodeIndex::unwrap(src_node, graph_sizes))
                        {}

                    void insert(AugmentedNodePermutation&& perm) {
                        ASSERT(perm.dst_automaton_set_ind_options.size() == src_unwrapped_nodes.size() - 1, "Cannot insert state permutation with incorrect size (" << perm.dst_automaton_set_ind_options.size() << ")");
                        array.push_back(perm);
                    }

                    std::size_t getPermutationSize() const {
                        std::size_t size = 0;
                        for (const auto& perm : array) {
                            size += Algorithms::Combinatorics::permutationsSizeFromOptions(perm.dst_automaton_set_ind_options);
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