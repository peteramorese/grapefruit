#pragma once

#include <unordered_set>
#include <set>
#include <map>

#include <yaml-cpp/yaml.h>

#include <spot/tl/parse.hh>
#include <spot/tl/print.hh>
#include <spot/twaalgos/translate.hh>
#include <spot/twa/formula2bdd.hh>
#include <spot/twa/bddprint.hh>

#include "core/Graph.h"
#include "tools/Serializer.h"

namespace TP {
namespace FormalMethods {

    using Observation = std::string;

    using Alphabet = std::unordered_set<std::string>;

    template <class EDGE_T, typename NATIVE_NODE_T = Node, bool REVERSIBLE = true>
    class Automaton : public Graph<EDGE_T, NATIVE_NODE_T, REVERSIBLE> {
        public:
            Automaton() {}

            void setAcceptingStates(const std::vector<NATIVE_NODE_T>& accepting_states) {
                for (auto ind : accepting_states) 
                    m_accepting_states.insert(ind);
            }

            inline bool addAcceptingState(NATIVE_NODE_T accepting_state) {return m_accepting_states.insert(accepting_state).second;}
            inline bool isAccepting(NATIVE_NODE_T ind) const {return m_accepting_states.contains(ind);}
            inline const std::set<NATIVE_NODE_T>& getAcceptingStates() const {return m_accepting_states;}

            inline void setInitStates(const std::vector<NATIVE_NODE_T>& init_states) {m_init_states = init_states;}
            inline const std::vector<NATIVE_NODE_T>& getInitStates() const {return m_init_states;}

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
            std::set<NATIVE_NODE_T> m_accepting_states;
            std::vector<Node> m_init_states;
            Alphabet m_atomic_propositions;
    };


    template <typename NATIVE_NODE_T = Node, bool REVERSIBLE = true>
    class GenericDFA : public Automaton<Observation, NATIVE_NODE_T, REVERSIBLE> {
        public:
            GenericDFA() {}

            virtual bool connect(NATIVE_NODE_T src, NATIVE_NODE_T dst, const Observation& edge) override {
                if (src < this->size()) {
                    const std::vector<Observation>& outgoing_edges = this->getOutgoingEdges(src);
                    for (const auto& label : outgoing_edges) {
                        // Do not connect if there is already
                        if (label == edge) return false;
                    }
                }
                Graph<Observation>::connect(src, dst, edge);
                return true;
            }

            bool generateFromFormula(const std::string& formula, bool complete = false) {
                spot::parsed_formula pf = spot::parse_infix_psl(formula);
                if (pf.format_errors(std::cerr)) return false;

                spot::translator translator;
                translator.set_type(spot::postprocessor::Buchi); // (state-based) Buchi

                spot::postprocessor::output_pref pref = spot::postprocessor::SBAcc; // State-based acceptance
                pref |= spot::postprocessor::Small; // Deterministic
                if (complete)
                    pref |= spot::postprocessor::Complete;

                translator.set_pref(pref);

                spot::twa_graph_ptr aut = translator.run(pf.f);
                this->setInitStates({aut->get_init_state_number()});

                for (uint32_t s = 0; s < aut->num_states(); ++s) {
                    if (aut->state_is_accepting(s)) this->m_accepting_states.insert(s);
                    for (auto& t : aut->out(s)) {
                        std::string label = spot::bdd_format_formula(aut->get_dict(), t.cond);
                        Graph<Observation>::connect(t.src, t.dst, label);
                        this->m_alphabet.insert(std::move(label));
                    }
                }
                return true;
            }

            bool deserialize(Deserializer& dszr) {
                YAML::Node& data = dszr.get();

                this->m_atomic_propositions = data["Atomic Propositions"].as<Alphabet>();
                this->m_init_states = data["Initial States"].as<std::vector<NATIVE_NODE_T>>();
                this->m_accepting_states = data["Accepting States"].as<std::set<NATIVE_NODE_T>>();

                std::map<NATIVE_NODE_T, std::vector<NATIVE_NODE_T>> connections = data["Connections"].as<std::map<NATIVE_NODE_T, std::vector<NATIVE_NODE_T>>>();
                std::map<NATIVE_NODE_T, std::vector<Observation>> observations = data["Labels"].as<std::map<NATIVE_NODE_T, std::vector<Observation>>>();
                for (auto[src, destinations] : connections) {
                    for (uint32_t i = 0; i <destinations.size(); ++i) {
                        const Observation& observation = observations[src][i];
                        NATIVE_NODE_T dst = destinations[i];
                        this->m_alphabet.insert(observation);
                        connect(src, dst, observation);
                    }
                }
                return true;
            }

            void print() const {
                LOG("Printing automaton");
                
                // Print init states:
                std::string init_states_str = std::string();
                for (auto state : this->m_init_states) init_states_str += (std::to_string(state) + ", ");
                PRINT_NAMED("Initial states", init_states_str);

                // Print init states:
                std::string accepting_states_str = std::string();
                for (auto state : this->m_accepting_states) accepting_states_str += (std::to_string(state) + ", ");
                PRINT_NAMED("Accepting states", accepting_states_str);

                NATIVE_NODE_T node = 0;
                for (const auto& list : this->m_graph) {
                    for (uint32_t i=0; i < list.forward.size(); ++i) {
                        if (i == 0) PRINT_NAMED("State " << node, "is connected to:");
                        PRINT_NAMED("    - child state " << list.forward.nodes[i], "with edge: " << list.forward.edges[i]);
                    }
                    node++;
                }
            }

            ~GenericDFA() {}
    };

    using DFA = GenericDFA<>;
    using DFAptr = std::shared_ptr<DFA>;

    std::vector<DFAptr> createDFAsFromFile(Deserializer& dszr);

    
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
    struct convert<std::set<TP::Node>> {
        static Node encode(const std::set<TP::Node>& state_set) {
            Node node;
            for (auto state : state_set) node.push_back(state);
            return node;
        }
        static bool decode(const Node& node, std::set<TP::Node>& state_set) {
            if (!node.IsSequence()) return false;

            std::vector<uint32_t> state_set_vector = node.as<std::vector<uint32_t>>();
            for (auto state : state_set_vector) state_set.insert(state);
            return true;
        }
    };
} // namespace YAML

// Alphabet merging operator
static TP::FormalMethods::Alphabet operator+(TP::FormalMethods::Alphabet alph_1, TP::FormalMethods::Alphabet alph_2) {
    alph_1.merge(alph_2);
    return alph_1;
}
