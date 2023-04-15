#pragma once

#include <memory>

#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Automaton.h"
#include "core/SymbolicProductAutomaton.h"

#include "graph_search/SymbolicSearchProblem.h"

#include "planners/PlanningProblem.h"

namespace TP {
namespace Planner {

    
    using DiscreteModel::TransitionSystem;

    using SymbolicProductGraph = DiscreteModel::SymbolicProductAutomaton<TransitionSystem, FormalMethods::DFA, DiscreteModel::ModelEdgeInheritor<TransitionSystem, FormalMethods::DFA>>;

    class DeterministicTaskPlannerSearchProblem : public GraphSearch::QuantitativeSymbolicSearchProblem<SymbolicProductGraph, DiscreteModel::TransitionSystemLabel::cost_t, GraphSearch::SearchDirection::Forward> {
        public:
            typedef SymbolicProductGraph graph_t;
            typedef graph_t::node_t node_t;
            typedef DiscreteModel::TransitionSystemLabel edge_t;
            typedef DiscreteModel::TransitionSystemLabel::cost_t cost_t;
            typedef DiscreteModel::TransitionSystemLabel::action_t action_t;
        public:
            DeterministicTaskPlannerSearchProblem(const std::shared_ptr<graph_t>& sym_graph, const DiscreteModel::State& init_state);

            virtual bool goal(const Node& node) const override;
    };

    class DeterministicTaskPlanner {
        public:
            DeterministicTaskPlanner(const std::shared_ptr<TransitionSystem>& ts, const std::vector<std::shared_ptr<FormalMethods::DFA>>& automata);

            Plan<DeterministicTaskPlannerSearchProblem> plan(const DiscreteModel::State& init_state) const;
        private:
            const std::shared_ptr<SymbolicProductGraph> m_sym_graph;
    };


}
}