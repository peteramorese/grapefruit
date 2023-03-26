#pragma once

#include "graph_search/SearchProblem.h"
#include "graph_search/SymbolicSearchProblem.h"

namespace TP {
namespace Planner {

template <class SYMBOLIC_GRAPH_T, class COST_T>
class DecentralizedHeuristicSearchProblem : public QuantitativeSymbolicSearchProblem<SYMBOLIC_GRAPH_T, COST_T, GraphSearch::SearchDirection::Backward> {

    public:
        // Do not terminate, search the entire graph
        virtual bool goal(const Node& node) const override {return false;}

        DecentralizedHeuristicSearchProblem(const std::shared_ptr<SYMBOLIC_GRAPH_T>& graph, const std::vector<typename SYMBOLIC_GRAPH_T::node_t>& initial_node_set) 
            : QuantitativeSymbolicSearchProblem<SYMBOLIC_GRAPH_T, COST_T, GraphSearch::SearchDirection::Backward>(graph, initial_node_set, {})
        {}
};

template <class MODEL_T, class AUTOMATON_T, class EDGE_INHERITOR>
class MinCostToGo {

    using SymbolicProductGraph = DiscreteModel::SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>;
    using ProdNode = SymbolicProductGraph::node_t;
    using CostType = MODEL_T::edge_t::cost_t;

    static MinCostMap<ProdNode, CostType> search(const std::shared_ptr<MODEL_T>& model, const std::shared_ptr<AUTOMATON_T>& automaton) {

        // Construct the single automaton product
        SymbolicProductGraph graph(model, {automaton});

        DecentralizedHeuristicSearchProblem<SymbolicProductGraph, CostType> problem(graph, )

    }
};

}
}