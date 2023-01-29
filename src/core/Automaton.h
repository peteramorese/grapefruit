#pragma once

#include <unordered_set>
#include <set>
#include <map>

#include "core/Graph.h"

template<class T>
class Automaton : public Graph<T> {
	public:
		typedef std::unordered_set<std::string> Alphabet;
	public:
		Automaton(bool reversible = true) 
            : Graph<T>(true, reversible) 
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

		const std::vector<uint32_t>& getInitStates() const {return m_init_states;}

		void setAlphabet(const Alphabet& alphabet) {m_alphabet = alphabet;}

		const Alphabet& getAlphabet() const {return m_alphabet}

        bool inAlphabet(const std::string& letter) const {return m_alphabet.contains(letter);}

		void setAtomicPropositions(const std::vector<std::string>& aps) {
            for (auto ap : aps) 
                m_atomic_propositions.insert(ap);
        }

		const std::vector<std::string>& getAtomicPropositions() const {
            return m_atomic_propositions;
        }

    protected:
		Alphabet m_alphabet;
		std::set<uint32_t> m_accepting_states;
		std::set<uint32_t> m_init_states;
		std::unordered_set<std::string> m_atomic_propositions;
};


class DFA : public Automaton<std::string> {
	public:
		DFA(bool reversible = true) 
            : Automaton<std::string>(reversible) {}

		virtual bool connect(uint32_t src, uint32_t dst, const std::string& edge) override {
            const std::vector<std::string>& outgoing_edges = getOutgoingEdges(src);
            for (const auto& label : outgoing_edges) {
                // Do not connect if there is already
                if (label == edge) return false;
            }
            Graph<std::string>::connect(src, dst, edge);
            return true;
        }

		bool readFileSingle(const std::string& filename) {
            
        }

		virtual void print() const override {
			LOG("Printing automaton");
            
            // Print init states:
            std::string init_states_str = std::string();
            for (auto state : m_init_states) init_states_str += (std::to_string(state) + ", ";
            PRINT_NAMED("Initial states:", init_states_str);

            // Print init states:
            std::string accepting_states_str = std::string();
            for (auto state : m_accepting_states) accepting_states_str += (std::to_string(state) + ", ";
            PRINT_NAMED("Accepting states:", m_accepting_states);

			uint32_t node_ind = 0;
			for (const auto& list : m_graph) {
				PRINT_NAMED("State " << node_ind++, "is connected to:");
				for (uint32_t i=0; i < list.forward.size(); ++i) {
					std::string edge_str = (m_edgeToStr) ? m_edgeToStr(list.forward.edges[i]) : list.forward.edges[i];
					PRINT_NAMED("    - child child " << list.forward.nodes[i], "with edge: " << list.forward.edges[i]);
				}
			}
        }

		~DFA() {}
};