#pragma once
#include<queue>
#include "graph.h"
#include "lexSet.h"
#include "transitionSystem.h"

template<class T>
class SymbSearch {
	private:
		//std::unordered_map<std::string, SimpleCondition*> propositions;
		struct spaceWeight {
			int dfa_ind;
			std::vector<bool> reachability;
			std::vector<float> state_weights;
		};	const std::vector<DFA_EVAL*>* dfa_list_ordered;

		int node_size;
		TS_EVAL<State>* TS;
		float mu, pathlength;
		std::vector<IVFlexLex<T>*> node_list;
		std::vector<T*> set_list;
		std::vector<std::string> TS_action_sequence;
		std::vector<int> TS_state_sequence;
		bool plan_found;
		std::vector<spaceWeight> heuristic;

		IVFlexLex<T>* newNode();
		T* newSet();
		template<typename Q> void printQueue(Q queue);
		void extractPath(const std::vector<int>& parents, int accepting_state);
		bool spaceSearch(TS_EVAL<State>* TS_sps, DFA_EVAL* dfa_sps, spaceWeight& spw);
		bool generateHeuristic();
		float pullStateWeight(unsigned ts_ind, unsigned dfa_ind, unsigned dfa_list_ind, bool& reachable) const;
	public:
		SymbSearch();
		void setAutomataPrefs(const std::vector<DFA_EVAL*>* dfa_list_ordered_);
		void setTransitionSystem(TS_EVAL<State>* TS_);
		void setFlexibilityParam(float mu_);
		bool search(bool use_heuristic = true);
		void writePlanToFile(std::string filename, const std::vector<std::string>& xtra_info);
		~SymbSearch();
};