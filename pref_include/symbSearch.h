#pragma once
#include<queue>
#include "Graph.h"
#include "transitionSystem.h"

class SymbSearch {
	private:
	public:
		SymbSearch();
		void setAutomataPrefs(const std::vector<DFA*>& dfa_list_ordered_);
		void setTransitionSystem(const TransitionSystem<State>* TS_);
		void setFlexibilityParam(float mu);
		bool plan();
};
