#pragma once
#include<queue>
#include "graph.h"
#include "transitionSystem.h"

class SymbSearch {
	private:
		//std::unordered_map<std::string, SimpleCondition*> propositions;
		const std::vector<DFA_EVAL*>* dfa_list_ordered;
		int node_size;
		const TS_EVAL<State>* TS;
		float mu;
		std::vector<WIV*> node_list;
		std::vector<unsigned int>* newNode();
	public:
		SymbSearch();
		void setAutomataPrefs(const std::vector<DFA_EVAL*>* dfa_list_ordered_);
		void setTransitionSystem(const TS_EVAL<State>* TS_);
		void setFlexibilityParam(float mu_);
		bool search();
		~SymbSearch();
};
