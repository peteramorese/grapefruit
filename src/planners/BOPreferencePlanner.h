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
    template <class EDGE_T, class COST_T, class OBJ_1_T, class OBJ_2_T>
    struct BOPCSEdgeConverter {
        BOPCSEdgeConverter() = delete;
        BOPCSEdgeConverter(const OBJ_1_T objective_1_, const OBJ_2_T objective_2_) = delete;
        
        // Conversion from PCS to CV
        CostVector<2, COST_T> operator()(const EDGE_T& edge) {
            return {{objective_1(edge), objective_2(edge)}}
        }

        OBJ_1_T objective_1;
        OBJ_2_T objective_2;
    };

    template <class SYMBOLIC_GRAPH_T, class COST_T, class EDGE_CONVERTER, class HEURISTIC_T = GraphSearch::MOZeroHeuristic<2, Node, COST_T>>
    class BOPreferencePlannerSearchProblem {
        public: // Custom search problem methods

            inline static CostVector<2, COST_T> convert(const SYMBOLIC_GRAPH_T::edge_t& edge) {

            }

        public: // Methods & members required by any search problem
            
            // Extension methods
            inline const std::vector<typename EXPLICIT_GRAPH_T::node_t>& neighbors(Node node) const {
                return m_graph->getChildren(node);
            }

            inline const std::vector<CostVector<2, COST_T>>& neighborEdges(Node node) const {
                // TODO use edge converter here
                return m_graph->getOutgoingEdges(node);
            }

            // Termination goal node
            inline bool goal(const Node& node) const {return m_goal_node_set.contains(node);}

            // Quantative methods
            inline CostVector<2, COST_T> gScore(const CostVector<2, COST_T>& parent_g_score, const EXPLICIT_GRAPH_T::edge_t& edge) const {return parent_g_score + m_edgeToCostVector(edge);}
            CostVector<2, COST_T> hScore(const Node& node) const {return heuristic.operator()(node);}

            // Member variables
            std::vector<Node> initial_node_set;
            HEURISTIC_T heuristic = HEURISTIC_T{}; // assumes default ctor

        public:
            BOPreferencePlanningProblem(const std::shared_ptr<DeterministicTaskPlanner::SymbolicProductGraph>& sym_graph, const DiscreteModel::State& init_state);

            virtual bool goal(const Node& node) const override;
        private:
            EDGE_CONVERTER m_edge_converter;
    };


}
}