#include "symbSearch.h"

SymbSearch::SymbSearch() {}

void SymbSearch::setAutomataPrefs(const std::vector<DFA*>* dfa_list_ordered_) {
	dfa_list_ordered = dfa_list_ordered_;
}

void SymbSearch::setTransitionSystem(const TransitionSystem<State>* TS_) {
	TS = TS_;	
}

void SymbSearch::setFlexibilityParam(float mu_) {
	mu = mu_;
}

bool SymbSearch::search() {

}
