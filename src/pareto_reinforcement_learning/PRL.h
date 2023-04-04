#pragma once

#include "PRLSearchProblem.h"

namespace PRL {

template <class BEHAVIOR_HANDLER_T>
class ParetoReinforcementLearning {
    public:
        using SymbolicProductGraph = TP::DiscreteModel::SymbolicProductAutomaton<
            TP::DiscreteModel::TransitionSystem, 
            TP::FormalMethods::DFA, 
            TP::DiscreteModel::ModelEdgeInheritor<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA>>;

        using Plan = PathSolution<typename PRLSearchProblem::node_t, typename PRLSearchProblem::edge_t, typename PRLSearchProblem::cost_t>;

        using ParetoFrontResult = TP::GraphSearch::MultiObjectiveSearchResult<GraphNode, EDGE_STORAGE_T, COST_VECTOR_T>;

    public:
        ParetoReinforcementLearning(const std::shared_ptr<TP::DiscreteModel::TransitionSystem>& ts, const std::vector<std::shared_ptr<TP::FormalMethods::DFA>>& automata)
            : m_product(std::make_shared<SymbolicProductGraph>(ts, automata))
        {}

        ParetoFrontResult computePlan(uint32_t step_horizon) {
            PRLSearchProblem problem(m_product, step_horizon, m_behavior_handler);
            return NAMOAStar<PRLSearchProblem::CostVector, decltype(problem)>::search(problem);
        }

        std::list<Plan>::const_iterator select(const ParetoFrontResult& search_result) {
            // TODO
            return search_result.solution_set.begin();
        }

        void execute(const Plan& plan) {
            // TODO
        }

        void run() {
            // while (true)
            ParetoFrontResult pf = computePlan(10);
            auto plan = select(pf);
            execute(plan);
        }

    private:
        std::shared_ptr<SymbolicProductGraph> m_product;
        std::shared_ptr<BEHAVIOR_HANDLER_T> m_behavior_handler;


};
}