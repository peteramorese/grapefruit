#pragma once

#include "graph_search/SearchProblem.h"
#include "graph_search/SymbolicSearchProblem.h"

namespace TP {
namespace Planner {

template <class MODEL_T, class AUTOMATON_T, class EDGE_INHERITOR>
class MinCostToGo {

    using SymbolicProductGraph = DiscreteModel::SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>;
    using ProdNode = SymbolicProductGraph::node_t;
    using CostType = MODEL_T::edge_t::cost_t;

    static MinCostMap<ProdNode, CostType> search(const std::shared_ptr<MODEL_T>& model, const std::shared_ptr<AUTOMATON_T>& automata) {

    }
};

template <class SYMBOLIC_GRAPH_T, class COST_T, GraphSearch::SearchDirection SEARCH_DIRECTION>
class DecentralizedSearchProblem {

}

}
}