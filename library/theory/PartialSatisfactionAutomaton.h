#pragma once

#include <string>
#include <map>

#include <yaml-cpp/yaml.h>

#include "core/Automaton.h"

namespace TP {
namespace FormalMethods {

    using SubstitutionCost = uint32_t;

    struct PartialSatisfactionEdge {
        PartialSatisfactionEdge() = default;
        PartialSatisfactionEdge(const std::string& observation_, SubstitutionCost substitution_cost_ = SubstitutionCost{}) : observation(observation_), substitution_cost(substitution_cost_) {}
        Observation observation = Observation{};
        SubstitutionCost substitution_cost = SubstitutionCost{};

        operator Observation&() {return observation;}
        operator const Observation&() const {return observation;}
        operator Observation&&() {return std::move(observation);}
    };

    using LetterSubstitutionMap = std::map<std::string, std::pair<std::string, SubstitutionCost>>;

    class PartialSatisfactionDFA : public Automaton<PartialSatisfactionEdge> {
        public:
            PartialSatisfactionDFA(bool reversible = true) 
                : Automaton<PartialSatisfactionEdge>(reversible) {}

            virtual bool connect(Node src, Node dst, const PartialSatisfactionEdge& edge) override {
                if (src < size()) {
                    const std::vector<PartialSatisfactionEdge>& outgoing_edges = getOutgoingEdges(src);
                    for (const auto& curr_edge : outgoing_edges) {
                        // Do not connect if there is already
                        if (curr_edge.observation == edge.observation) return false;
                    }
                }
                Graph<PartialSatisfactionEdge>::connect(src, dst, edge);
                return true;
            }

            bool deserialize(const std::string& dfa_filepath, const std::string& sub_map_filepath = std::string()) {
                YAML::Node data;
                try {
                    data = YAML::LoadFile(dfa_filepath);

                    m_atomic_propositions = data["Alphabet"].as<Alphabet>();
                    m_init_states = data["Initial States"].as<StateSet>();
                    m_accepting_states = data["Accepting States"].as<StateSet>();

                    std::map<uint32_t, std::vector<uint32_t>> connections = data["Connections"].as<std::map<uint32_t, std::vector<uint32_t>>>();
                    std::map<uint32_t, std::vector<std::string>> labels = data["Labels"].as<std::map<uint32_t, std::vector<std::string>>>();
                    for (auto[src, destinations] : connections) {
                        for (uint32_t i = 0; i <destinations.size(); ++i) {
                            const std::string& label = labels[src][i];
                            Node dst = destinations[i];
                            m_alphabet.insert(label);
                            connect(src, dst, PartialSatisfactionEdge(label));
                        }
                    }
                } catch (YAML::ParserException e) {
                    ERROR("Failed to load file" << dfa_filepath << " ("<< e.what() <<")");
                }

                if (sub_map_filepath.empty()) return true;

                try {
                    data = YAML::LoadFile(sub_map_filepath);

                    std::vector<std::string> from_observations = data["From Observations"].as<std::vector<std::string>>();
                    std::vector<std::string> to_observations = data["To Observations"].as<std::vector<std::string>>();
                    std::vector<SubstitutionCost> sub_costs = data["Costs"].as<std::vector<SubstitutionCost>>();

                    ASSERT(from_observations.size() == to_observations.size() && to_observations.size() == sub_costs.size(), "Number of 'From Observations' and 'To Observations' and 'Costs' must match")

                    LetterSubstitutionMap sub_map;
                    for (uint32_t i=0; i<from_observations.size(); ++i) sub_map.try_emplace(from_observations[i], to_observations[i], sub_costs[i]);

                    for (Node src_node = 0; src_node < size(); ++src_node) {
                        // Copy so that recursive substitutions do not happen
                        std::vector<Node> children = getChildren(src_node);
                        std::vector<PartialSatisfactionEdge> edges = getOutgoingEdges(src_node);
                        for (uint32_t i=0; i<children.size(); ++i) {
                            const auto it = sub_map.find(edges[i].observation);
                            if (it != sub_map.end()) {
                                m_alphabet.insert(it->second.first);
                                connect(src_node, children[i], PartialSatisfactionEdge(it->second.first, it->second.second));
                            }
                        }
                    }

                } catch (YAML::ParserException e) {
                    ERROR("Failed to load file" << sub_map_filepath << " ("<< e.what() <<")");
                }

                return true;
            }

            virtual void print() const override {
                LOG("Printing partial-satisfaction automaton");
                
                // Print init states:
                std::string init_states_str = std::string();
                for (auto state : m_init_states) init_states_str += (std::to_string(state) + ", ");
                PRINT_NAMED("Initial states:", init_states_str);

                // Print init states:
                std::string accepting_states_str = std::string();
                for (auto state : m_accepting_states) accepting_states_str += (std::to_string(state) + ", ");
                PRINT_NAMED("Accepting states:", accepting_states_str);

                Node node = 0;
                for (const auto& list : m_graph) {
                    for (uint32_t i=0; i < list.forward.size(); ++i) {
                        if (i == 0) PRINT_NAMED("State " << node, "is connected to:");
                        PRINT_NAMED("    - child state " << list.forward.nodes[i], "with observation: " << list.forward.edges[i].observation << " and cost: " << list.forward.edges[i].substitution_cost);
                    }
                    node++;
                }
            }

    };
}
}