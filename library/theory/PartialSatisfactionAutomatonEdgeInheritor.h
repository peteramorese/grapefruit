#pragma once

#include "theory/PartialSatisfactionAutomaton.h"

#include "tools/Containers.h"

namespace TP {
namespace DiscreteModel {

    template <class MODEL_T>
    struct PartialSatisfactionAutomataEdgeInheritor {



        struct CombinedEdge {
            public:
                // Cost is inherited from both the automata and the model
                typedef Containers::SizedArray<FormalMethods::SubstitutionCost> automaton_cost_t;
                typedef MODEL_T::edge_t::cost_t cost_t;
                
                // Action is inherited from the transition system
                typedef MODEL_T::edge_t::action_t action_t;

            public:
                CombinedEdge(uint32_t size) : automaton_cost(size) {}
                // Action conversion operators
                operator action_t&() {return action;}
                operator const action_t&() const {return action;}
                operator action_t&&() {return std::move(action);}

                // Cost conversion operators
                operator cost_t() const {return model_cost;}
                operator cost_t&&() {return std::move(model_cost);}

                // Automaton cost conversion operators
                automaton_cost_t getAutomatonCost() {return automaton_cost;}
                const automaton_cost_t& getAutomatonCost() const {return automaton_cost;}

            public: 
                action_t action; 
                cost_t model_cost;
                automaton_cost_t automaton_cost;

        };
        typedef CombinedEdge type;

        static type inherit(const MODEL_T::edge_t& model_edge, Containers::SizedArray<typename FormalMethods::PartialSatisfactionDFA::edge_t>&& automaton_edges) {
            CombinedEdge combined_edge(automaton_edges.size());
            combined_edge.action = model_edge.action;
            combined_edge.model_cost = model_edge.cost;
            for (uint32_t i=0; i<automaton_edges.size(); ++i) combined_edge.automaton_cost[i] = automaton_edges[i].substitution_cost;
            return combined_edge;
        }
    };

} // namespace DiscreteModel
} // namespace TP
