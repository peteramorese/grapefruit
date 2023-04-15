#pragma once

#include <memory>

#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Automaton.h"
#include "core/SymbolicProductAutomaton.h"

#include "graph_search/MultiObjectiveSearchProblem.h"

#include "planners/PlanningProblem.h"

#include "planners/PreferenceCostObjective.h"

namespace TP {
namespace Planner {

    template <class SYMBOLIC_GRAPH_T, class INHERITED_COST_T, class OBJ_1_T, class OBJ_2_T, class HEURISTIC_T = GraphSearch::MOZeroHeuristic<WideNode, Containers::TypeGenericArray<OBJ_1_T, OBJ_2_T>>>
    class BOPreferencePlannerSearchProblem {
        public:
            using CostVector = Containers::TypeGenericArray<OBJ_1_T, OBJ_2_T>;
            using ProdNode = SYMBOLIC_GRAPH_T::node_t;

            typedef SYMBOLIC_GRAPH_T graph_t;
            typedef ProdNode node_t;
            typedef graph_t::edge_t::action_t action_t;
            typedef CostVector cost_t;

            static constexpr uint32_t numObjectives() {return 2;}

            struct CostVectorActionEdge {
                CostVectorActionEdge() = delete;
                CostVectorActionEdge(OBJ_1_T&& obj_1, OBJ_2_T&& obj_2, graph_t::edge_t::action_t&& action_) : cv(std::move(obj_1), std::move(obj_2)), action(action_) {}
                bool operator==(const CostVectorActionEdge& other) const {return cv == other.cv && action == other.action;}
                CostVector cv;
                action_t action;

                // Search problem action_t must be convertable to the base action_t
                operator action_t&() {return action;}
                operator const action_t&() const {return action;}
                operator action_t&&() const {return std::move(action);}
            };
            typedef CostVectorActionEdge edge_t;

        public: // Methods & members required by any search problem
            
            // Extension methods
            inline std::vector<ProdNode> neighbors(ProdNode node) const {
                return m_graph->getChildren(node);
            }

            // Direct conversion 
            inline std::vector<edge_t> neighborEdges(ProdNode node) const {
                std::vector<typename SYMBOLIC_GRAPH_T::edge_t> inherited_edges = m_graph->getOutgoingEdges(node);
                std::vector<edge_t> converted_edges;

                converted_edges.reserve(inherited_edges.size());
                for (auto& inherited_edge : inherited_edges) {
                    converted_edges.emplace_back(OBJ_1_T(*m_graph, node, std::move(inherited_edge)), OBJ_2_T(*m_graph, node, std::move(inherited_edge)), static_cast<action_t&&>(inherited_edge));
                }
                std::vector<edge_t> test = converted_edges;
                return converted_edges;
            }

            // Termination goal node
            inline bool goal(const ProdNode& node) const {
                auto unwrapped_node = m_graph->getUnwrappedNode(node);
                const auto& automata = m_graph->extractAutomata();
                for (uint32_t i=0; i<unwrapped_node.automata_nodes.size(); ++i) {
                    Node automaton_node = unwrapped_node.automata_nodes[i];
                    
                    //Conjunctive acceptance
                    if (!automata[i]->isAccepting(automaton_node)) return false;
                }
                return true;
            }

            // Quantative methods
            inline CostVector gScore(const ProdNode& src_node, const ProdNode& dst_node, CostVector parent_g_score, const edge_t& edge) const {
                // Element-wise add each of the objectives (edge_t is alias for CostVector)
                parent_g_score += edge.cv;

                return parent_g_score;
            }

            CostVector hScore(const ProdNode& node) const {return heuristic.operator()(node);}

            // Member variables
            std::set<WideNode> initial_node_set;
            HEURISTIC_T heuristic = HEURISTIC_T{}; // assumes default ctor

        public:
            BOPreferencePlannerSearchProblem(const std::shared_ptr<SYMBOLIC_GRAPH_T>& sym_graph) : m_graph(sym_graph) {}

        private:
            std::shared_ptr<SYMBOLIC_GRAPH_T> m_graph;
    };


}
}