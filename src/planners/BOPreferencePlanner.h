#pragma once

#include <memory>

#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Automaton.h"
#include "core/SymbolicProductAutomaton.h"

#include "graph_search/MultiObjectiveSearchProblem.h"

#include "planners/PlanningProblem.h"

#include "planners/PreferenceCostObjective.h"

namespace TP {
namespace Planner {

    using DiscreteModel::TransitionSystem;
    using FormalMethods::DFA;


    template <class PREFERENCE_COST_OBJECTIVE>
    class BOPreferencePlanner {
        public:
            using SymbolicProductGraph = DiscreteModel::SymbolicProductAutomaton<TransitionSystem, DFA, EDGE_INHERITOR>;

            struct 
        public:
            BOPreferencePlanner(const std::shared_ptr<TransitionSystem>& ts, const std::vector<std::shared_ptr<DFA>>& automata);

            Plan plan(const DiscreteModel::State& init_state) const;
        private:
            const std::shared_ptr<SymbolicProductGraph> m_sym_graph;
    };

    // Bi-objective Preference Cost Set Edge Converter
    template <class PCS_T, class OBJ_COST_T, class OBJ_1_T, class OBJ_2_T>
    struct BOPCSEdgeConverter {
        BOPCSEdgeConverter() = delete;
        BOPCSEdgeConverter(const OBJ_1_T& objective_1_, const OBJ_2_T& objective_2_) = delete;
        
        // Conversion from PCS to CV (Edges are temporarily constructed)
        CostVector<2, OBJ_COST_T> operator()(WideNode node, PCS_T&& edge) {
            return {{objective_1(node, edge), objective_2(node, edge)}}
        }

        OBJ_1_T objective_1;
        OBJ_2_T objective_2;
    };

    template <class SYMBOLIC_GRAPH_T, class COST_T, class EDGE_CONVERTER, class HEURISTIC_T = GraphSearch::MOZeroHeuristic<2, WideNode, COST_T>>
    class BOPreferencePlannerSearchProblem {
        public:
            using GraphSearch::CostVector;

        public: // Methods & members required by any search problem
            
            // Extension methods
            inline std::vector<typename SYMBOLIC_GRAPH_T::node_t> neighbors(WideNode node) const {
                return m_graph->getChildren(node);
            }

            // Direct conversion 
            inline std::vector<CostVector<2, PreferenceCostSet<COST_T>>> neighborEdges(WideNode node) const {
                std::vector<CostVector<2, PreferenceCostSet<COST_T>>> 
                return m_graph->getOutgoingEdges(node);
            }

            // Termination goal node
            inline bool goal(const Node& node) const {return m_goal_node_set.contains(node);}

            // Quantative methods
            inline CostVector<2, COST_T> gScore(const CostVector<2, COST_T>& parent_g_score, const CostVector<2, COST_T>& edge) const {return parent_g_score + edge;}
            CostVector<2, COST_T> hScore(const WideNode& node) const {return heuristic.operator()(node);}

            // Member variables
            std::vector<WideNode> initial_node_set;
            HEURISTIC_T heuristic = HEURISTIC_T{}; // assumes default ctor

        public:
            BOPreferencePlanningProblem(const std::shared_ptr<DeterministicTaskPlanner::SymbolicProductGraph>& sym_graph, const DiscreteModel::State& init_state);

            virtual bool goal(const Node& node) const override;
        private:
            EDGE_CONVERTER m_edge_converter;
    };


}
}