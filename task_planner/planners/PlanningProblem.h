#pragma once

#include <fstream>


#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Graph.h"
#include "tools/Serializer.h"

#include "graph_search/SearchProblem.h"

#include <yaml-cpp/yaml.h>

namespace TP {
namespace Planner {

    template <class SEARCH_PROBLEM_T>
    struct Plan {
        public:
            struct ConstStateIterator {
                typename std::vector<typename SEARCH_PROBLEM_T::node_t>::const_iterator p_node_it;
                typename std::vector<Node>::const_iterator ts_node_it;
                typename std::vector<typename SEARCH_PROBLEM_T::graph_t::model_t::node_t>::const_iterator state_it;
                ConstStateIterator operator++() {
                    ConstStateIterator it = *this;
                    ++p_node_it;
                    ++ts_node_it;
                    ++state_it;
                    return it;
                }
                ConstStateIterator operator++(int) {
                    ++p_node_it;
                    ++ts_node_it; 
                    ++state_it;
                    return *this;
                }
                inline const typename SEARCH_PROBLEM_T::node_t& productNode() const {return *p_node_it;}
                inline Node tsNode() const {return *ts_node_it;}
                inline const typename SEARCH_PROBLEM_T::graph_t::model_t::node_t& state() const {return *state_it;}
            };
            struct StateIterator {
                typename std::vector<typename SEARCH_PROBLEM_T::node_t>::iterator p_node_it;
                typename std::vector<Node>::iterator ts_node_it; 
                typename std::vector<typename SEARCH_PROBLEM_T::graph_t::model_t::node_t>::iterator state_it;
                StateIterator operator++() {
                    StateIterator it = *this;
                    ++p_node_it;
                    ++ts_node_it;
                    ++state_it;
                    return it;
                }
                StateIterator operator++(int) {
                    ++p_node_it;
                    ++ts_node_it; 
                    ++state_it;
                    return *this;
                }
                inline const typename SEARCH_PROBLEM_T::node_t& productNode() const {return *p_node_it;}
                inline Node tsNode() const {return *ts_node_it;}
                inline const typename SEARCH_PROBLEM_T::graph_t::model_t::node_t& state() const {return *state_it;}

                operator ConstStateIterator() const {
                    ConstStateIterator it;
                    it.p_node_it = p_node_it;
                    it.ts_node_it = ts_node_it;
                    it.state_it = state_it;
                    return it;
                }
            };

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

            void serialize(Serializer& szr, const std::string& title = std::string()) const {
                YAML::Emitter& out = szr.get();

                if (!title.empty()) 
                    out << YAML::Key << "Title" << YAML::Value << title;

                out << YAML::Key << "Action Sequence" << YAML::Value << action_sequence;
                out << YAML::Key << "State Sequence" << YAML::Value << YAML::BeginSeq;

                for (const auto& s : state_sequence) {
                    out << s.to_str();
                }
                YAML::EndSeq;
            }

            StateIterator begin() {
                StateIterator it;
                it.p_node_it = product_node_sequence.begin();
                it.ts_node_it = ts_node_sequence.begin();
                it.state_it = state_sequence.begin();
                return it;
            } 

            StateIterator end() {
                StateIterator it;
                it.p_node_it = product_node_sequence.end();
                it.ts_node_it = ts_node_sequence.end();
                it.state_it = state_sequence.end();
                return it;
            } 

            ConstStateIterator begin() const {
                ConstStateIterator it;
                it.p_node_it = product_node_sequence.begin();
                it.ts_node_it = ts_node_sequence.begin();
                it.state_it = state_sequence.begin();
                return it;
            } 

            ConstStateIterator end() const {
                ConstStateIterator it;
                it.p_node_it = product_node_sequence.end();
                it.ts_node_it = ts_node_sequence.end();
                it.state_it = state_sequence.end();
                return it;
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

    class ParetoFrontSerializer {
        public:
            
            template <class SEARCH_PROBLEM_T, typename LAM>
            static void serializeParetoFront(Serializer& szr, const PlanSet<SEARCH_PROBLEM_T>& plan_set, const std::array<std::string, SEARCH_PROBLEM_T::numObjectives()>& axis_labels, LAM costToFloatArray, const std::string& color = "firebrick") {
                constexpr uint32_t n_obj = SEARCH_PROBLEM_T::numObjectives();

                YAML::Emitter& out = szr.get();

                for (uint32_t obj_i = 0; obj_i < n_obj; ++obj_i) {
                    out << YAML::Key << "Objective " + std::to_string(obj_i) << YAML::Value << YAML::BeginSeq;
                    for (const auto& plan : plan_set) {
                        Containers::FixedArray<n_obj, float> float_arr = costToFloatArray(plan.cost);
                        out << float_arr[obj_i];
                    }
                    out << YAML::EndSeq;
                }

                out << YAML::Key << "Axis Labels" << YAML::Value << YAML::BeginSeq;
                for (uint32_t obj_i = 0; obj_i < n_obj; ++obj_i) {
                    out << axis_labels[obj_i];
                }
                out << YAML::EndSeq;

                out << YAML::Key << "Color" << YAML::Value << color;

            }
        
            static void serialize2DParetoFront(Serializer& szr, const std::vector<std::pair<float, float>>& pareto_points, const std::array<std::string, 2>& axis_labels, const std::string& color = "firebrick") {
                YAML::Emitter& out = szr.get();


                out << YAML::Key << "Objective 0" << YAML::Value << YAML::BeginSeq;
                for (const auto[x, y] : pareto_points) {
                    out << x;
                }
                out << YAML::EndSeq;

                out << YAML::Key << "Objective 1" << YAML::Value << YAML::BeginSeq;
                for (const auto[x, y] : pareto_points) {
                    out << y;
                }
                out << YAML::EndSeq;

                out << YAML::Key << "Axis Labels" << YAML::Value << YAML::BeginSeq;
                out << axis_labels[0];
                out << axis_labels[1];
                out << YAML::EndSeq;

                out << YAML::Key << "Color" << YAML::Value << color;
            }
    };

}
}