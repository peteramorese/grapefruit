#pragma once
#include "edge.h"

class Automaton : Edge {
	private:
		std::vector<unsigned int> accepting_states;
		std::vector<unsigned int> init_states;
		Edge* g;
		std::vector<std::string> alphabet;
		unsigned int max_accepting_state_index;
		unsigned int max_init_state_index;
		bool isAutomatonValid();
		bool inAlphabet(std::string);
	public:
		Automaton(std::vector<std::string> alphabet_);
		void setAcceptingStates(const std::vector<unsigned int>& accepting_states_);
		void setInitStates(const std::vector<unsigned int>& init_states_);
}

class NFA : Automaton {
	private:
	public:
}

class DFA : Automaton {
	private:
		bool check_det;
	public:
		DFA();
		void toggleCheckDeterminism(bool check_det_)
		bool connect(unsigned int ind_from, unsigned int ind_to, float weight, std::string letter);
			
}
