#pragma once
#include<queue>
#include "graph.h"
#include "lexSet.h"
#include "transitionSystem.h"

class SymbSearch {
	private:
		//std::unordered_map<std::string, SimpleCondition*> propositions;
		const std::vector<DFA_EVAL*>* dfa_list_ordered;
		int node_size;
		TS_EVAL<State>* TS;
		float mu;
		std::vector<IVFlexLexS*> node_list;
		IVFlexLexS* newNode();
	public:
		SymbSearch();
		void setAutomataPrefs(const std::vector<DFA_EVAL*>* dfa_list_ordered_);
		void setTransitionSystem(TS_EVAL<State>* TS_);
		void setFlexibilityParam(float mu_);
		bool search();
		~SymbSearch();
};
