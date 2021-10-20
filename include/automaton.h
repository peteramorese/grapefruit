#pragma once
#include<array>
#include "graph.h"

template<class T>
class Automaton : public Graph<T> {
	private:
		unsigned int max_accepting_state_index;
		unsigned int max_init_state_index;
	protected:
		std::vector<T> node_data_list;
		std::vector<unsigned int> accepting_states;
		std::vector<unsigned int> init_states;
		std::vector<std::string> alphabet;
		void addWord(const std::string&);
		void addAcceptingState(unsigned int accepting_state);
		bool isAutomatonValid();
		bool inAlphabet(std::string);
		bool checkAlphabet(const Automaton* arg_dfa);
		//template<typename pLAM> void print(pLAM printLambda);
	public:
		Automaton();
		void setAcceptingStates(const std::vector<unsigned int>& accepting_states_);
		void setInitStates(const std::vector<unsigned int>& init_states_);
		void setAlphabet(const std::vector<std::string>& alphabet_);
};

class NFA : public Automaton<std::string> {
	private:
	public:
};

class DFA : public Automaton<std::string> {
	private:
		bool check_det;
	public:
		DFA();
		void toggleCheckDeterminism(bool check_det_);
		int getInitState();
		bool connectDFA(unsigned int ind_from, unsigned int ind_to, const std::string& label_);
		bool readFileSingle(const std::string& filename);
		void print();
		//void operator*(const DFA& arg_dfa);
		//static bool syncProduct( const DFA* arg_dfa, const DFA* arg_dfa2, DFA* product);
		//static bool readFileMultiple(const std::string& filename, std::array<DFA, int>& dfa_arr);
};
