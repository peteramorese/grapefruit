#include "DeterministicTaskPlanner.h"

#include "graph_search/AStar.h"

namespace GF {
namespace Planner {

    DeterministicTaskPlannerSearchProblem::DeterministicTaskPlannerSearchProblem(const std::shared_ptr<SymbolicProductGraph>& sym_graph, const DiscreteModel::State& init_state) 
        : GraphSearch::QuantitativeSymbolicSearchProblem<SymbolicProductGraph, DiscreteModel::TransitionSystemLabel::cost_t, GraphSearch::SearchDirection::Forward>(
            sym_graph,
            {},
            {} // Empty goal node set
        )
    {
        Containers::SizedArray<Node> init_aut_nodes(sym_graph->rank() - 1);
        for (uint32_t i=0; i<init_aut_nodes.size(); ++i) init_aut_nodes[i] = *(sym_graph->getAutomaton(i).getInitStates().begin());
        initial_node_set = {sym_graph->getWrappedNode(sym_graph->getModel().getGenericNodeContainer()[init_state], init_aut_nodes)};

    }

    bool DeterministicTaskPlannerSearchProblem::goal(const Node& node) const {
        auto unwrapped_node = m_graph->getUnwrappedNode(node);
        const auto& automata = m_graph->extractAutomata();
        for (uint32_t i=0; i<unwrapped_node.automata_nodes.size(); ++i) {
            Node automaton_node = unwrapped_node.automata_nodes[i];
            
            //Conjunctive acceptance
            if (!automata[i]->isAccepting(automaton_node)) return false;
        }
        return true;
    }

    DeterministicTaskPlanner::DeterministicTaskPlanner(const std::shared_ptr<TransitionSystem>& ts, const std::vector<std::shared_ptr<FormalMethods::DFA>>& automata) 
        : m_sym_graph(std::make_shared<SymbolicProductGraph>(ts, automata))
    {}

    Plan<DeterministicTaskPlannerSearchProblem> DeterministicTaskPlanner::plan(const DiscreteModel::State& init_state) const {
        DeterministicTaskPlannerSearchProblem problem(m_sym_graph, init_state);

        auto result = GraphSearch::AStar<SymbolicProductGraph::node_t, SymbolicProductGraph::edge_t, DiscreteModel::TransitionSystemLabel::cost_t, decltype(problem)>::search(problem);
        return Plan<DeterministicTaskPlannerSearchProblem>(result.solution, result.cost, m_sym_graph, result.success);
    }

}
}