#pragma once

#include <unordered_set>
#include <set>
#include <map>

#include <yaml-cpp/yaml.h>

#include "core/Graph.h"

namespace TP {
namespace FormalMethods {

    using Observation = std::string;

    using Alphabet = std::unordered_set<std::string>;

    using StateSet = std::set<Node>;

    template<class T>
    class Automaton : public Graph<T> {
        public:
            Automaton(bool reversible = true, Graph<T>::EdgeToStrFunction edgeToStr = nullptr) 
                : Graph<T>(true, reversible, edgeToStr) 
            {}


            void setAcceptingStates(const std::vector<uint32_t>& accepting_states) {
                for (auto ind : accepting_states) 
                    m_accepting_states.insert(ind);
            }

            inline bool addAcceptingState(Node accepting_state) {return m_accepting_states.insert(accepting_state).second;}
            inline bool isAccepting(Node ind) const {return m_accepting_states.contains(ind);}
            inline const std::set<Node>& getAcceptingStates() const {return m_accepting_states;}

            inline void setInitStates(const std::vector<Node>& init_states) {for (auto ind : init_states) m_init_states.insert(ind);}
            inline const std::set<Node>& getInitStates() const {return m_init_states;}

            inline void setAlphabet(const Alphabet& alphabet) {m_alphabet = alphabet;}
            inline const Alphabet& getAlphabet() const {return m_alphabet;}
            inline bool inAlphabet(const Observation& obs) const {return m_alphabet.contains(obs);}

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


    class DFA : public Automaton<Observation> {
        public:
            DFA(bool reversible = true) 
                : Automaton<Observation>(reversible) {}

            virtual bool connect(Node src, Node dst, const Observation& edge) override {
                if (src < size()) {
                    const std::vector<Observation>& outgoing_edges = getOutgoingEdges(src);
                    for (const auto& label : outgoing_edges) {
                        // Do not connect if there is already
                        if (label == edge) return false;
                    }
                }
                Graph<Observation>::connect(src, dst, edge);
                return true;
            }

            virtual bool deserialize(const std::string& filepath) {
                YAML::Node data;
                try {
                    data = YAML::LoadFile(filepath);

                    m_atomic_propositions = data["Atomic Propositions"].as<Alphabet>();
                    m_init_states = data["Initial States"].as<StateSet>();
                    m_accepting_states = data["Accepting States"].as<StateSet>();

                    std::map<uint32_t, std::vector<uint32_t>> connections = data["Connections"].as<std::map<uint32_t, std::vector<uint32_t>>>();
                    std::map<uint32_t, std::vector<Observation>> observations = data["Labels"].as<std::map<uint32_t, std::vector<Observation>>>();
                    for (auto[src, destinations] : connections) {
                        for (uint32_t i = 0; i <destinations.size(); ++i) {
                            const Observation& observation = observations[src][i];
                            Node dst = destinations[i];
                            m_alphabet.insert(observation);
                            connect(src, dst, observation);
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

                Node node = 0;
                for (const auto& list : m_graph) {
                    for (uint32_t i=0; i < list.forward.size(); ++i) {
                        if (i == 0) PRINT_NAMED("State " << node, "is connected to:");
                        PRINT_NAMED("    - child state " << list.forward.nodes[i], "with edge: " << list.forward.edges[i]);
                    }
                    node++;
                }
            }
            ~DFA() {}
    };
} // namespace FormalMethods
} // namespace TP


// YAML parsing

namespace YAML {
    template <>
    struct convert<TP::FormalMethods::Alphabet> {
        static Node encode(const TP::FormalMethods::Alphabet& alphabet) {
            Node node;
            for (auto letter : alphabet) node.push_back(letter);
            return node;
        }
        static bool decode(const Node& node, TP::FormalMethods::Alphabet& alphabet) {
            if (!node.IsSequence()) return false;

            std::vector<std::string> alphabet_vector = node.as<std::vector<std::string>>();
            for (auto letter : alphabet_vector) alphabet.insert(letter);
            return true;
        }
    };

    template <>
    struct convert<TP::FormalMethods::StateSet> {
        static Node encode(const TP::FormalMethods::StateSet& state_set) {
            Node node;
            for (auto state : state_set) node.push_back(state);
            return node;
        }
        static bool decode(const Node& node, TP::FormalMethods::StateSet& state_set) {
            if (!node.IsSequence()) return false;

            std::vector<uint32_t> state_set_vector = node.as<std::vector<uint32_t>>();
            for (auto state : state_set_vector) state_set.insert(state);
            return true;
        }
    };
} // namespace YAML

// Alphabet merging operator
static TP::FormalMethods::Alphabet operator+(const TP::FormalMethods::Alphabet& alph_1, const TP::FormalMethods::Alphabet& alph_2) {
    TP::FormalMethods::Alphabet merged = alph_1;
    merged.merge(TP::FormalMethods::Alphabet(alph_2));
    return merged;
}
