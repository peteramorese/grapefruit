#pragma once
#include<queue>
#include "Graph.h"
#include "transitionSystem.h"

class SymbSearch : public TransitionSystem<State> {
	private:
		std::unordered_map<std::string, SimpleCondition*> propositions;
		const std::vector<DFA*>* dfa_list_ordered;
		const TransitionSystem<State>* TS;
		float mu;
	public:
		SymbSearch();
		void setAutomataPrefs(const std::vector<DFA*>* dfa_list_ordered_);
		void setTransitionSystem(const TransitionSystem<State>* TS_);
		void setFlexibilityParam(float mu_);
		bool search();
};
