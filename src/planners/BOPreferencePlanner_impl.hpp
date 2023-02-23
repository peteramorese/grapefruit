#pragma once

#include <memory>

#include "BOPreferencePlanner.h"

#include "tools/Containers.h"

namespace TP {
namespace Planner {

    template <class EDGE_INHERITOR, class OBJ_1_T, class OBJ_2_T>
    BOPreferencePlanner<EDGE_INHERITOR, OBJ_1_T, OBJ_2_T>::BOPreferencePlanner(const std::shared_ptr<TransitionSystem>& ts, const std::vector<std::shared_ptr<DFA>>& automata) 
        : m_sym_graph(std::make_shared<SymbolicProductGraph>(ts, automata))
    {

    }

    template <class EDGE_INHERITOR, class OBJ_1_T, class OBJ_2_T>
    Plan BOPreferencePlanner<EDGE_INHERITOR, OBJ_1_T, OBJ_2_T>::plan(const DiscreteModel::State& init_state) const {
        BOPreferencePlannerSearchProblem<SymbolicProductGraph, DiscreteModel::TransitionSystemLabel::cost, OBJ_1_T, OBJ_2_T> problem(m_sym_graph);

        // Find and add the init node
        Containers::SizedArray<Node> init_aut_nodes(sym_graph->rank() - 1);
        for (uint32_t i=0; i<init_aut_nodes.size(); ++i) init_aut_nodes[i] = *(sym_graph->getAutomaton(i).getInitStates().begin());
        m_problem.initial_node_set = {sym_graph->getWrappedNode(sym_graph->getModel().getGenericNodeContainer()[init_state], init_aut_nodes)};

        using CostVector = Containers::TypeGenericArray<OBJ_1_T, OBJ_2_T>;
        auto result = GraphSearch::BOAStar<SymbolicProductGraph::edge_t, DiscreteModel::TransitionSystemLabel::cost_t, CostVector, decltype(problem)>::search(problem);
        return Plan(result.solution, m_sym_graph, result.success);
    }

}
}