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
    using FormalMethods::DFA;

    class DeterministicTaskPlanner {
        public:
            using SymbolicProductGraph = DiscreteModel::SymbolicProductAutomaton<TransitionSystem, DFA, DiscreteModel::ModelEdgeInheritor<TransitionSystem, DFA>>;
        public:
            DeterministicTaskPlanner(const std::shared_ptr<TransitionSystem>& ts, const std::vector<std::shared_ptr<DFA>>& automata);

            Plan<SymbolicProductGraph, DiscreteModel::TransitionSystemLabel::cost_t> plan(const DiscreteModel::State& init_state) const;
        private:
            const std::shared_ptr<SymbolicProductGraph> m_sym_graph;
    };

    class DeterministicTaskPlannerSearchProblem : public GraphSearch::QuantitativeSymbolicSearchProblem<DeterministicTaskPlanner::SymbolicProductGraph, DiscreteModel::TransitionSystemLabel::cost_t, GraphSearch::SearchDirection::Forward> {
        public:
            DeterministicTaskPlannerSearchProblem(const std::shared_ptr<DeterministicTaskPlanner::SymbolicProductGraph>& sym_graph, const DiscreteModel::State& init_state);

            virtual bool goal(const Node& node) const override;
    };


}
}