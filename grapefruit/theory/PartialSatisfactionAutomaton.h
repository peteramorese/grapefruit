#pragma once

#include <string>
#include <map>

#include <yaml-cpp/yaml.h>

#include "core/Automaton.h"

namespace GF {
namespace FormalMethods {

    using SubstitutionCost = uint32_t;

    struct PartialSatisfactionEdge {
        PartialSatisfactionEdge() = default;
        PartialSatisfactionEdge(const std::string& observation_, SubstitutionCost substitution_cost_ = SubstitutionCost{}) : observation(observation_), substitution_cost(substitution_cost_) {}
        bool operator==(const PartialSatisfactionEdge& other) const {return observation == other.observation && substitution_cost == other.substitution_cost;}
        Observation observation = Observation{};
        SubstitutionCost substitution_cost = SubstitutionCost{};

        operator Observation&() {return observation;}
        operator const Observation&() const {return observation;}
        operator Observation&&() {return std::move(observation);}
    };

    using LetterSubstitutionMap = std::map<std::string, std::pair<std::string, SubstitutionCost>>;

    template <typename NATIVE_NODE_T = Node, bool REVERSIBLE = true>
    class GenericPartialSatisfactionDFA : public Automaton<PartialSatisfactionEdge, NATIVE_NODE_T, REVERSIBLE> {
        public:
            GenericPartialSatisfactionDFA() {}

            virtual bool connect(Node src, Node dst, const PartialSatisfactionEdge& edge) override {
                if (src < this->size()) {
                    const std::vector<PartialSatisfactionEdge>& outgoing_edges = this->outgoingEdges(src);
                    for (const auto& curr_edge : outgoing_edges) {
                        // Do not connect if there is already
                        if (curr_edge.observation == edge.observation) return false;
                    }
                }
                Graph<PartialSatisfactionEdge>::connect(src, dst, edge);
                return true;
            }

            bool deserialize(const Deserializer& dfa_dszr, const Deserializer& sub_map_dszr = Deserializer()) {
                {
                    const YAML::Node& data = dfa_dszr.get();

                    this->m_atomic_propositions = data["Atomic Propositions"].as<Alphabet>();
                    this->m_init_states = data["Initial States"].as<std::vector<NATIVE_NODE_T>>();
                    this->m_accepting_states = data["Accepting States"].as<std::set<NATIVE_NODE_T>>();

                    std::map<NATIVE_NODE_T, std::vector<NATIVE_NODE_T>> connections = data["Connections"].as<std::map<NATIVE_NODE_T, std::vector<NATIVE_NODE_T>>>();
                    std::map<NATIVE_NODE_T, std::vector<std::string>> labels = data["Labels"].as<std::map<NATIVE_NODE_T, std::vector<std::string>>>();
                    for (auto[src, destinations] : connections) {
                        for (uint32_t i = 0; i <destinations.size(); ++i) {
                            const std::string& label = labels[src][i];
                            Node dst = destinations[i];
                            this->m_alphabet.insert(label);
                            connect(src, dst, PartialSatisfactionEdge(label));
                        }
                    }
                }

                if (!sub_map_dszr) return true;

                {
                    const YAML::Node& data = sub_map_dszr.get();

                    if (!data["From Observations"]) return true;
                    if (!data["To Observations"]) LOG("no to observations");
                    if (!data["From Observations"]) LOG("no from observations");

                    std::vector<std::string> from_observations = data["From Observations"].as<std::vector<std::string>>();
                    std::vector<std::string> to_observations = data["To Observations"].as<std::vector<std::string>>();
                    std::vector<SubstitutionCost> sub_costs = data["Costs"].as<std::vector<SubstitutionCost>>();

                    ASSERT(from_observations.size() == to_observations.size() && to_observations.size() == sub_costs.size(), "Number of 'From Observations' and 'To Observations' and 'Costs' must match")

                    LetterSubstitutionMap sub_map;
                    for (uint32_t i=0; i<from_observations.size(); ++i) sub_map.try_emplace(from_observations[i], to_observations[i], sub_costs[i]);

                    for (NATIVE_NODE_T src_node = 0; src_node < this->size(); ++src_node) {
                        // Copy so that recursive substitutions do not happen
                        std::vector<NATIVE_NODE_T> children = this->children(src_node);
                        std::vector<PartialSatisfactionEdge> edges = this->outgoingEdges(src_node);
                        for (uint32_t i=0; i<children.size(); ++i) {
                            const auto it = sub_map.find(edges[i].observation);
                            if (it != sub_map.end()) {
                                this->m_alphabet.insert(it->second.first);
                                connect(src_node, children[i], PartialSatisfactionEdge(it->second.first, it->second.second));
                            }
                        }
                    }
                }

                return true;
            }

            void print() const {
                LOG("Printing partial-satisfaction automaton");
                
                // Print init states:
                std::string init_states_str = std::string();
                for (auto state : this->m_init_states) init_states_str += (std::to_string(state) + ", ");
                PRINT_NAMED("Initial states:", init_states_str);

                // Print init states:
                std::string accepting_states_str = std::string();
                for (auto state : this->m_accepting_states) accepting_states_str += (std::to_string(state) + ", ");
                PRINT_NAMED("Accepting states:", accepting_states_str);

                NATIVE_NODE_T node = 0;
                for (const auto& list : this->m_graph) {
                    for (uint32_t i=0; i < list.forward.size(); ++i) {
                        if (i == 0) PRINT_NAMED("State " << node, "is connected to:");
                        PRINT_NAMED("    - child state " << list.forward.nodes[i], "with observation: " << list.forward.edges[i].observation << " and cost: " << list.forward.edges[i].substitution_cost);
                    }
                    node++;
                }
            }
    };

    using PartialSatisfactionDFA = GenericPartialSatisfactionDFA<>;
}
}