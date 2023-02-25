#pragma once

#include <fstream>

#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Graph.h"

#include "graph_search/SearchProblem.h"

#include <yaml-cpp/yaml.h>

namespace TP {
namespace Planner {

    template <class SEARCH_PROBLEM_T>
    struct Plan {
            typedef SEARCH_PROBLEM_T::graph_t::model_t::edge_t model_edge_t;
        public:
            Plan(const GraphSearch::PathSolution<typename SEARCH_PROBLEM_T::node_t, typename SEARCH_PROBLEM_T::edge_t, typename SEARCH_PROBLEM_T::cost_t>& path, const std::shared_ptr<typename SEARCH_PROBLEM_T::graph_t> sym_graph, bool success) 
                : product_node_sequence(path.node_path)
            {
                if (success) {
                    ts_node_sequence.reserve(product_node_sequence.size());
                    state_sequence.reserve(product_node_sequence.size());
                    for (auto prod_node : product_node_sequence) {
                        Node ts_node = sym_graph->getUnwrappedNode(prod_node).ts_node;
                        ts_node_sequence.push_back(ts_node);
                        state_sequence.push_back(sym_graph->getModel().getGenericNodeContainer()[ts_node]);
                    }

                    action_sequence.reserve(path.edge_path.size());
                    for (auto edge : path.edge_path) action_sequence.push_back(static_cast<const SEARCH_PROBLEM_T::action_t&>(edge));

                    cost = path.path_cost;
                } else {
                    product_node_sequence.clear();
                }
            }

            inline bool success() const {return product_node_sequence.size();}

            void print() const {
                LOG("Priting plan");
                
                // Align text:
                uint32_t max_action_str_len = 0;
                for (const auto& a : action_sequence) {
                    if (a.size() > max_action_str_len) max_action_str_len = a.size();
                }
                max_action_str_len += 2;

                for (uint32_t i=0; i<ts_node_sequence.size(); ++i) {
                    std::string s = "Node: " +std::to_string(ts_node_sequence[i]) + " [" + state_sequence[i].to_str() + "]";
                    if (i != 0) {
                        std::string a = action_sequence[i-1] + " -> ";
                        std::string a_adj(max_action_str_len + 4 - a.size(), ' ');
                        PRINT_NAMED(a_adj + a, s);
                    } else {
                        PRINT_NAMED(std::string(max_action_str_len + 4, ' '), s);
                    }

                }
            }

            void serialize(const std::string& filepath) const {
                YAML::Emitter out;

                out << YAML::BeginMap;

                out << YAML::Key << "Action Sequence" << YAML::Value << action_sequence;
                out << YAML::Key << "State Sequence" << YAML::Value << YAML::BeginSeq;

                for (const auto& s : state_sequence) {
                    out << s.to_str();
                }
                YAML::EndSeq;
                out << YAML::EndMap;
                std::ofstream fout(filepath);
                fout << out.c_str();
            }

        public:
            std::vector<typename SEARCH_PROBLEM_T::node_t> product_node_sequence;
            std::vector<Node> ts_node_sequence;
            std::vector<typename SEARCH_PROBLEM_T::graph_t::model_t::node_t> state_sequence;
            std::vector<typename SEARCH_PROBLEM_T::action_t> action_sequence;
            typename SEARCH_PROBLEM_T::cost_t cost;
    };

    template <class SEARCH_PROBLEM_T>
    using PlanSet = std::vector<Plan<SEARCH_PROBLEM_T>>;
}
}