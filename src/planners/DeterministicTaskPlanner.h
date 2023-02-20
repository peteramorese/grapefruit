#pragma once

#include <memory>

#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Automaton.h"
#include "core/SymbolicProductAutomaton.h"

#include "planners/PlanningProblem.h"

namespace TP {
namespace Planner {

    class DeterministicTaskPlanner {
        public:
            DeterministicTaskPlanner(const std::shared_ptr<DiscreteModel::TransitionSystem>& ts, const std::vector<std::shared_ptr<FormalMethods::DFA>>& automata)
                : m_ts(ts)
                , m_automata(automata)
                {}

            Plan plan(const DiscreteModel::State& init_state) const;
        private:
            const std::shared_ptr<DiscreteModel::TransitionSystem> m_ts;
            const std::vector<std::shared_ptr<FormalMethods::DFA>> m_automata;
    };
}
}