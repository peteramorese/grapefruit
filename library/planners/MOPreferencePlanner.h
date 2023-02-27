#pragma once

#include "core/Automaton.h"
#include "core/SymbolicProductAutomaton.h"

#include "graph_search/MultiObjectiveSearchProblem.h"

#include "planners/PlanningProblem.h"
#include "planners/PreferenceCostObjective.h"
#include "planners/MOPreferencePlannerSearchProblem.h"

namespace TP {
namespace Planner {

    using DiscreteModel::TransitionSystem;


    template <class EDGE_INHERITOR, class AUTOMATON_T, class...OBJ_ARGS_T>
    class MOPreferencePlanner {
        public:
            using SymbolicProductGraph = DiscreteModel::SymbolicProductAutomaton<TransitionSystem, AUTOMATON_T, EDGE_INHERITOR>;

            using Problem = MOPreferencePlannerSearchProblem<SymbolicProductGraph, typename DiscreteModel::TransitionSystemLabel::cost_t, GraphSearch::MOZeroHeuristic<typename SymbolicProductGraph::node_t, Containers::TypeGenericArray<OBJ_ARGS_T...>>, OBJ_ARGS_T...>;
        public:
            MOPreferencePlanner(const std::shared_ptr<TransitionSystem>& ts, const std::vector<std::shared_ptr<AUTOMATON_T>>& automata);

            PlanSet<Problem> plan(const DiscreteModel::State& init_state) const;
        private:
            const std::shared_ptr<SymbolicProductGraph> m_sym_graph;
    };


}
}

#include "MOPreferencePlanner_impl.hpp"