#pragma once

#include <memory>

#include "BOPreferencePlanner.h"

#include "tools/Containers.h"

#include "graph_search/BOAStar.h"

namespace TP {
namespace Planner {

    template <class EDGE_INHERITOR, class OBJ_1_T, class OBJ_2_T>
    BOPreferencePlanner<EDGE_INHERITOR, OBJ_1_T, OBJ_2_T>::BOPreferencePlanner(const std::shared_ptr<TransitionSystem>& ts, const std::vector<std::shared_ptr<DFA>>& automata) 
        : m_sym_graph(std::make_shared<SymbolicProductGraph>(ts, automata))
    {

    }

    template <class EDGE_INHERITOR, class OBJ_1_T, class OBJ_2_T>
    PlanSet<typename BOPreferencePlanner<EDGE_INHERITOR, OBJ_1_T, OBJ_2_T>::SymbolicProductGraph, DiscreteModel::TransitionSystemLabel::cost_t> BOPreferencePlanner<EDGE_INHERITOR, OBJ_1_T, OBJ_2_T>::plan(const DiscreteModel::State& init_state) const {
        BOPreferencePlannerSearchProblem<SymbolicProductGraph, typename DiscreteModel::TransitionSystemLabel::cost_t, OBJ_1_T, OBJ_2_T> problem(m_sym_graph);

        // Find and add the init node
        Containers::SizedArray<Node> init_aut_nodes(m_sym_graph->rank() - 1);
        for (uint32_t i=0; i<init_aut_nodes.size(); ++i) init_aut_nodes[i] = *(m_sym_graph->getAutomaton(i).getInitStates().begin());
        problem.initial_node_set = {m_sym_graph->getWrappedNode(m_sym_graph->getModel().getGenericNodeContainer()[init_state], init_aut_nodes)};

        using CostVector = Containers::TypeGenericArray<OBJ_1_T, OBJ_2_T>;
        auto mo_result = GraphSearch::BOAStar<typename SymbolicProductGraph::edge_t, typename DiscreteModel::TransitionSystemLabel::cost_t, CostVector, decltype(problem)>::search(problem);
        
        // Convert path solutions to plans
        PlanSet<SymbolicProductGraph, DiscreteModel::TransitionSystemLabel::cost_t> plan_set;
        plan_set.reserve(mo_result.solution_set.size());

        for (const auto& sol : mo_result.solution_set) plan_set.emplace_back(sol, m_sym_graph, mo_result.success);

        return plan_set;
    }

}
}