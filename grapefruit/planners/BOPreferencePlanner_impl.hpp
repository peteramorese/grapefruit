#pragma once

#include <memory>

#include "BOPreferencePlanner.h"

#include "tools/Containers.h"

#include "graph_search/BOAStar.h"

namespace GF {
namespace Planner {

    template <class EDGE_INHERITOR, class AUTOMATON_T, class OBJ_1_T, class OBJ_2_T>
    BOPreferencePlanner<EDGE_INHERITOR, AUTOMATON_T, OBJ_1_T, OBJ_2_T>::BOPreferencePlanner(const std::shared_ptr<TransitionSystem>& ts, const std::vector<std::shared_ptr<AUTOMATON_T>>& automata) 
        : m_sym_graph(std::make_shared<SymbolicProductGraph>(ts, automata))
    {}


    template <class EDGE_INHERITOR, class AUTOMATON_T, class OBJ_1_T, class OBJ_2_T>
    PlanSet<typename BOPreferencePlanner<EDGE_INHERITOR, AUTOMATON_T, OBJ_1_T, OBJ_2_T>::Problem> BOPreferencePlanner<EDGE_INHERITOR, AUTOMATON_T, OBJ_1_T, OBJ_2_T>::plan(const DiscreteModel::State& init_state) const {
        Problem problem(m_sym_graph);

        // Find and add the init node
        Containers::SizedArray<Node> init_aut_nodes(m_sym_graph->rank() - 1);
        for (uint32_t i=0; i<init_aut_nodes.size(); ++i) init_aut_nodes[i] = *(m_sym_graph->getAutomaton(i).getInitStates().begin());
        problem.initial_node_set = {m_sym_graph->getWrappedNode(m_sym_graph->getModel().getGenericNodeContainer()[init_state], init_aut_nodes)};

        using CostVector = Containers::TypeGenericArray<OBJ_1_T, OBJ_2_T>;
        auto mo_result = GraphSearch::BOAStar<CostVector, decltype(problem)>::search(problem);
        
        // Convert path solutions to plans
        PlanSet<Problem> plan_set;
        plan_set.reserve(mo_result.solution_set.size());

        auto pf_it = mo_result.pf.begin();
        for (auto& sol : mo_result.solution_set) {
            plan_set.emplace_back(sol, *pf_it++, m_sym_graph, mo_result.success);
        }

        return plan_set;
    }

}
}