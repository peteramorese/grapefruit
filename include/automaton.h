#pragma once
#include "edge.h"

class Automaton : public Edge {
	private:
		std::vector<unsigned int> accepting_states;
		std::vector<unsigned int> init_states;
		std::vector<std::string> alphabet;
		unsigned int max_accepting_state_index;
		unsigned int max_init_state_index;
	protected:
		void addWord(const std::string&);
		void addAcceptingState(unsigned int accepting_state);
		bool isAutomatonValid();
		bool inAlphabet(std::string);
	public:
		Automaton();
		void setAcceptingStates(const std::vector<unsigned int>& accepting_states_);
		void setInitStates(const std::vector<unsigned int>& init_states_);
		void setAlphabet(const std::vector<std::string>& alphabet_);
		void print();
};

class NFA : public Automaton {
	private:
	public:
};

class DFA : public Automaton {
	private:
		bool check_det;
	public:
		DFA();
		void toggleCheckDeterminism(bool check_det_);
		bool connect(unsigned int ind_from, unsigned int ind_to, float weight_, std::string label_);
		bool readFromFile(std::string);
			
};
