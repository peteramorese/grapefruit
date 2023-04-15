#pragma once

#include "tools/Logging.h"
#include "core/SymbolicProductAutomaton.h"

namespace TP {
namespace DiscreteModel {

class ProductAutomatonProjection {
    public: 
        // Automatically calculates projection using addresses of the corresponding automata
        template <class SYMBOLIC_PRODUCT_T> 
        static SYMBOLIC_PRODUCT_T::node_t project(const SYMBOLIC_PRODUCT_T& from_product, const SYMBOLIC_PRODUCT_T& onto_product, SYMBOLIC_PRODUCT_T::node_t from_node_wrapped) {
            ASSERT(from_product.rank() > onto_product.rank(), "From product must have a higher rank than onto product");
            ASSERT(from_product.extractModel().get() == onto_product.extractModel().get(), "Must project between products with the same base model");
            UnwrappedNode from_node = from_product.getUnwrappedNode(from_node_wrapped);
            UnwrappedNode onto_node(onto_product.rank());
            
            // Match the automata
            const auto& onto_automata = onto_product.extractAutomata();
            const auto& from_automata = from_product.extractAutomata();
            for (ProductRank onto_aut_ind = 0; onto_aut_ind < onto_automata.size(); ++onto_aut_ind) {
                bool found = false;
                for (ProductRank from_aut_ind = 0; from_aut_ind < from_automata.size(); ++from_aut_ind) {
                    if (onto_automata[onto_aut_ind].get() == from_automata[from_aut_ind].get()) {
                        found = true; 
                        onto_node.automata_nodes[onto_aut_ind] = from_node.automata_nodes[from_aut_ind];
                        break;
                    }
                }
                ASSERT(found, "Did not find 'from' automaton corresponding to 'onto' automaton" << onto_aut_ind);
            }
            return onto_product.getWrappedNode(from_node.ts_node, onto_node.automata_nodes);
        }

        template <class SYMBOLIC_PRODUCT_T> 
        static UnwrappedNode project(const SYMBOLIC_PRODUCT_T& from_product, const SYMBOLIC_PRODUCT_T& onto_product, const UnwrappedNode& from_node_unwrapped) {
            ASSERT(from_product.rank() > onto_product.rank(), "From product must have a higher rank than onto product");
            ASSERT(from_product.extractModel().get() == onto_product.extractModel().get(), "Must project between products with the same base model");
            UnwrappedNode onto_node(onto_product.rank());
            onto_node.ts_node = from_node_unwrapped.ts_node;
            
            // Match the automata
            const auto& onto_automata = onto_product.extractAutomata();
            const auto& from_automata = from_product.extractAutomata();
            for (ProductRank onto_aut_ind = 0; onto_aut_ind < onto_automata.size(); ++onto_aut_ind) {
                bool found = false;
                for (ProductRank from_aut_ind = 0; from_aut_ind < from_automata.size(); ++from_aut_ind) {
                    if (onto_automata[onto_aut_ind].get() == from_automata[from_aut_ind].get()) {
                        found = true; 
                        onto_node.automata_nodes[onto_aut_ind] = from_node_unwrapped.automata_nodes[from_aut_ind];
                        break;
                    }
                }
                ASSERT(found, "Did not find 'from' automaton corresponding to 'onto' automaton" << onto_aut_ind);
            }
            return onto_node;
        }
};



}
}