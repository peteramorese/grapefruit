#pragma once

#include "planners/heuristic/DecentralizedMCTG.h"

namespace TP {
namespace Planner {

template <class SYMBOLIC_GRAPH_T>
class MaxMCTGHeuristic {
    public:
        using ModelType = SYMBOLIC_GRAPH_T::model_t;
        using AutomatonType = SYMBOLIC_GRAPH_T::automaton_t;
        using CostType = ModelType::edge_t::cost_t;
        using ProdNode = SYMBOLIC_GRAPH_T::node_t; 

    public:
        MaxMCTGHeuristic(const std::shared_ptr<SYMBOLIC_GRAPH_T>& graph) 
            : m_graph(graph)
        {}

        void compute() {
            const std::shared_ptr<ModelType>& model = m_graph->extractModel();
            const std::vector<std::shared_ptr<AutomatonType>>& automata = m_graph->extractAutomata();

            m_decentralized_mctg.reserve(automata.size());
            for (const auto& automaton : automata) {
                m_decentralized_mctg.push_back(MinCostToGo<ModelType, AutomatonType, typename SYMBOLIC_GRAPH_T::edge_inheritor_t>::search(model, automaton));
            }
        }

        inline CostType heuristic(ProdNode node) {

        }

    private:
        std::shared_ptr<SYMBOLIC_GRAPH_T> m_graph;
        std::vector<std::shared_ptr<GraphSearch::MinCostMap<ProdNode, CostType>>> m_decentralized_mctg;
};

}
}