#pragma once

#include "core/Automaton.h"
#include "core/SymbolicProductAutomaton.h"

#include "graph_search/MultiObjectiveSearchProblem.h"

#include "planners/PlanningProblem.h"
#include "planners/PreferenceCostObjective.h"
#include "planners/BOPreferencePlannerSearchProblem.h"

namespace TP {
namespace Planner {

    using DiscreteModel::TransitionSystem;
    using FormalMethods::DFA;


    template <class EDGE_INHERITOR, class OBJ_1_T, class OBJ_2_T>
    class BOPreferencePlanner {
        public:
            using SymbolicProductGraph = DiscreteModel::SymbolicProductAutomaton<TransitionSystem, DFA, EDGE_INHERITOR>;

        public:
            BOPreferencePlanner(const std::shared_ptr<TransitionSystem>& ts, const std::vector<std::shared_ptr<DFA>>& automata);

            Plan plan(const DiscreteModel::State& init_state) const;
        private:
            const std::shared_ptr<SymbolicProductGraph> m_sym_graph;
    };


}
}

#include "BOPreferencePlanner_impl.hpp"