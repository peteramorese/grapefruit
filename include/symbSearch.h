#pragma once
#include<queue>
#include "graph.h"
#include "lexSet.h"
#include "transitionSystem.h"

template<class T>
class SymbSearch {
	private:
		//std::unordered_map<std::string, SimpleCondition*> propositions;
		const std::vector<DFA_EVAL*>* dfa_list_ordered;
		int node_size;
		TS_EVAL<State>* TS;
		float mu, pathlength;
		std::vector<IVFlexLex<T>*> node_list;
		IVFlexLex<T>* newNode();
		template<typename Q>
		void printQueue(Q queue);
		bool plan_found;
		std::vector<std::string> TS_action_sequence;
		std::vector<int> TS_state_sequence;
		void extractPath(const std::vector<int>& parents, int accepting_state);
		struct spaceWeight {
			int dfa_ind;
			std::vector<float> state_weights;
		};
		std::vector<spaceWeight> heuristic;
		bool spaceSearch(TS_EVAL<State> * TS_sps, DFA_EVAL* dfa_sps, spaceWeight& spw);
	public:
		SymbSearch();
		void setAutomataPrefs(const std::vector<DFA_EVAL*>* dfa_list_ordered_);
		void setTransitionSystem(TS_EVAL<State>* TS_);
		void setFlexibilityParam(float mu_);
		bool search();
		void writePlanToFile(std::string filename, const std::vector<std::string>& xtra_info);
		~SymbSearch();
};
