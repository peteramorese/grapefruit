#pragma once

#include <unordered_set>
#include <set>
#include <map>

#include <yaml-cpp/yaml.h>

#include "core/Graph.h"

namespace FormalMethods {

    typedef std::unordered_set<std::string> Alphabet;

    typedef std::set<uint32_t> StateSet;

    template<class T>
    class Automaton : public Graph<T> {
        public:
        public:
            Automaton(bool reversible = true, Graph<T>::EdgeToStrFunction edgeToStr = nullptr) 
                : Graph<T>(true, reversible, edgeToStr) 
            {}

            void setAcceptingStates(const std::vector<uint32_t>& accepting_states) {
                for (auto ind : accepting_states) 
                    m_accepting_states.insert(ind);
            }

            bool addAcceptingState(uint32_t accepting_state) {return m_accepting_states.insert(accepting_state).second;}

            bool isAccepting(uint32_t ind) const {return m_accepting_states.contains(ind);}

            const std::set<uint32_t>& getAcceptingStates() const {return m_accepting_states;}

            void setInitStates(const std::vector<uint32_t>& init_states) {
                for (auto ind : init_states) 
                    m_init_states.insert(ind);
            }

            const std::set<uint32_t>& getInitStates() const {return m_init_states;}

            void setAlphabet(const Alphabet& alphabet) {m_alphabet = alphabet;}

            const Alphabet& getAlphabet() const {return m_alphabet;}

            bool inAlphabet(const std::string& letter) const {return m_alphabet.contains(letter);}

            void setAtomicPropositions(const std::vector<std::string>& aps) {
                for (auto ap : aps) 
                    m_atomic_propositions.insert(ap);
            }

            const Alphabet& getAtomicPropositions() const {
                return m_atomic_propositions;
            }

        protected:
            Alphabet m_alphabet;
            StateSet m_accepting_states;
            StateSet m_init_states;
            Alphabet m_atomic_propositions;
    };


    class DFA : public Automaton<std::string> {
        public:
            DFA(bool reversible = true, Graph<std::string>::EdgeToStrFunction edgeToStr = nullptr) 
                : Automaton<std::string>(reversible, edgeToStr) {}

            virtual bool connect(uint32_t src, uint32_t dst, const std::string& edge) override {
                if (src < size()) {
                    const std::vector<std::string>& outgoing_edges = getOutgoingEdges(src);
                    for (const auto& label : outgoing_edges) {
                        // Do not connect if there is already
                        if (label == edge) return false;
                    }
                }
                Graph<std::string>::connect(src, dst, edge);
                return true;
            }

            bool deserialize(const std::string& filepath) {
                YAML::Node data;
                try {
                    data = YAML::LoadFile(filepath);

                    m_alphabet = data["Alphabet"].as<Alphabet>();
                    m_init_states = data["Initial States"].as<StateSet>();
                    m_accepting_states = data["Accepting States"].as<StateSet>();

                    std::map<uint32_t, std::vector<uint32_t>> connections = data["Connections"].as<std::map<uint32_t, std::vector<uint32_t>>>();
                    std::map<uint32_t, std::vector<std::string>> labels = data["Labels"].as<std::map<uint32_t, std::vector<std::string>>>();
                    for (auto[src, destinations] : connections) {
                        for (uint32_t i = 0; i <destinations.size(); ++i) {
                            const std::string& label = labels[src][i];
                            uint32_t dst = destinations[i];
                            connect(src, dst, label);
                        }
                    }
                } catch (YAML::ParserException e) {
                    ERROR("Failed to load file" << filepath << " ("<< e.what() <<")");
                }
                return true;
            }

            virtual void print() const override {
                LOG("Printing automaton");
                
                // Print init states:
                std::string init_states_str = std::string();
                for (auto state : m_init_states) init_states_str += (std::to_string(state) + ", ");
                PRINT_NAMED("Initial states:", init_states_str);

                // Print init states:
                std::string accepting_states_str = std::string();
                for (auto state : m_accepting_states) accepting_states_str += (std::to_string(state) + ", ");
                PRINT_NAMED("Accepting states:", accepting_states_str);

                uint32_t node_ind = 0;
                for (const auto& list : m_graph) {
                    PRINT_NAMED("State " << node_ind++, "is connected to:");
                    for (uint32_t i=0; i < list.forward.size(); ++i) {
                        std::string edge_str = (m_edgeToStr) ? m_edgeToStr(list.forward.edges[i]) : list.forward.edges[i];
                        PRINT_NAMED("    - child state " << list.forward.nodes[i], "with edge: " << list.forward.edges[i]);
                    }
                }
            }

            ~DFA() {}
    };
}

// YAML parsing

namespace YAML {
    template <>
    struct convert<FormalMethods::Alphabet> {
        static Node encode(const FormalMethods::Alphabet& alphabet) {
            Node node;
            for (auto letter : alphabet) node.push_back(letter);
            return node;
        }
        static bool decode(const Node& node, FormalMethods::Alphabet& alphabet) {
            if (!node.IsSequence()) return false;

            std::vector<std::string> alphabet_vector = node.as<std::vector<std::string>>();
            for (auto letter : alphabet_vector) alphabet.insert(letter);
            return true;
        }
    };

    template <>
    struct convert<FormalMethods::StateSet> {
        static Node encode(const FormalMethods::StateSet& state_set) {
            Node node;
            for (auto state : state_set) node.push_back(state);
            return node;
        }
        static bool decode(const Node& node, FormalMethods::StateSet& state_set) {
            if (!node.IsSequence()) return false;

            std::vector<uint32_t> state_set_vector = node.as<std::vector<uint32_t>>();
            for (auto state : state_set_vector) state_set.insert(state);
            return true;
        }
    };
}