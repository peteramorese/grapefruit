#include<string>
#include<vector>
#include<iostream>
#include<iostream>
#include "automaton.h"
#include "edge.h"

Automaton::Automaton(Edge* g_, std::vector<std::string> alphabet_) : g(g_), alphabet(alphabet_), max_accepting_state_index(0), max_init_state_index(0)
{
	
}

bool Automaton::isAutomatonValid() {
	bool valid = true;
	int graph_size = g.size();
	if (max_accpeting_state_index > graph_size || max_init_state_index > graph_size) {
		valid = false;	
	}
	return valid;
}

bool Automaton::inAlphabet(std::string s) {
	for (int i=0; i<alphabet.size(); ++i) {
		if (s == alphabet[i]) {
			return true;
		}
	}	
	return false;
}

void Automaton::setAcceptingStates(const std::vector<unsigned int>& accepting_states_){
	// Determine the max accepting state index to easily verify that all
	// accepting states exist in the graph
	accepting_states = accepting_states_;
	for (int i=0; i<accepting_states.size(); ++i) {
		if (accepting_states[i] > max_accepting_state_index) {
			max_accepting_state_index = accepting_states[i];	
		}
	}	
}

void Automaton::setInitStates(const std::vector<unsigned int>& init_states_){
	// Determine the max init state index to easily verify that all
	// init states exist in the graph
	init_states = init_states_;
	for (int i=0; i<init_states.size(); ++i) {
		if (initn_states[i] > max_init_state_index) {
			max_init_state_index = init_states[i];	
		}
	}	
}




DFA::DFA() : check_det(true) {}

void DFA::toggleCheckDeterminism(bool check_det_) {
	check_det = check_det_;
}

bool DFA::connect(unsigned int state_ind_from, unsigned int state_ind_to, std::string letter) {
	if (inAlphabet(letter)) {
		if (check_det){
			bool new_state_from, new_state_to;
			new_state_from = (state_ind_from > g.size) ? true : false;
			//if (state_ind_from > g.size()) {
			//	new_state_from = true;
			//} else {
			//	new_state_from = false;
			//}
			if (new_state_from) {
				std::vector<std::string> label_list;
				g.returnListLabels(state_ind_from, label_list);
				for (int i=0; i<label_list.size(); ++i) {
					if (letter = label_list[i]) {
						std::cout<<"Error: Determinism check failed when connecting state "<<state_ind_from<<" to "<<state_ind_to<<" with letter: "<<letter<<"\n";
						return false;
					}
				}
			}
		}
		// If the letter is not seen among all other outgoing edges
		// from the 'from state', the connection is deterministic and
		// the states can be connected. DFA is unweighted by default.
		g.connect(state_ind_from, state_ind_to, 0, letter);
		return true;
	}
}

