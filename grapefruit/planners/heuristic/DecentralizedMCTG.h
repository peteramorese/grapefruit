#pragma once

#include "graph_search/SearchProblem.h"
#include "graph_search/SymbolicSearchProblem.h"
#include "graph_search/AStar.h"

namespace GF {
namespace Planner {

template <class SYMBOLIC_GRAPH_T, class COST_T>
class DecentralizedHeuristicSearchProblem : public GraphSearch::QuantitativeSymbolicSearchProblem<SYMBOLIC_GRAPH_T, COST_T, GraphSearch::SearchDirection::Backward> {

    public:
        // Do not terminate, search the entire graph
        virtual bool goal(const Node& node) const override {return false;}

        DecentralizedHeuristicSearchProblem(const std::shared_ptr<SYMBOLIC_GRAPH_T>& graph, const std::vector<typename SYMBOLIC_GRAPH_T::node_t>& initial_node_set) 
            : GraphSearch::QuantitativeSymbolicSearchProblem<SYMBOLIC_GRAPH_T, COST_T, GraphSearch::SearchDirection::Backward>(graph, initial_node_set, {})
        {}
};

template <class MODEL_T, class AUTOMATON_T, class EDGE_INHERITOR>
class MinCostToGo {
    public:
        using SymbolicProductGraph = DiscreteModel::SymbolicProductAutomaton<MODEL_T, AUTOMATON_T, EDGE_INHERITOR>;
        using ProdNode = SymbolicProductGraph::node_t;
        using CostType = MODEL_T::edge_t::cost_t;

        static std::shared_ptr<GraphSearch::MinCostMap<ProdNode, CostType>> search(const std::shared_ptr<MODEL_T>& model, const std::shared_ptr<AUTOMATON_T>& automaton) {

            // Construct the single automaton product
            std::vector<std::shared_ptr<AUTOMATON_T>> automata = {automaton};
            std::shared_ptr<SymbolicProductGraph> graph = std::make_shared<SymbolicProductGraph>(model, automata);

            // Convert accepting node set to init node array
            std::set<ProdNode> accepting_nodes = graph->getAcceptingNodes();
            std::vector<ProdNode> init_nodes;
            init_nodes.reserve(accepting_nodes.size());
            for (auto n : accepting_nodes) init_nodes.push_back(n);

            DecentralizedHeuristicSearchProblem<SymbolicProductGraph, CostType> problem(graph, init_nodes);

            auto result = GraphSearch::AStar<ProdNode, typename SymbolicProductGraph::edge_t, CostType, decltype(problem)>::search(problem);
            return result.min_cost_map;
        }
};

}
}