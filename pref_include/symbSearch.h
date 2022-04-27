#pragma once
#include<queue>
#include "graph.h"
#include "lexSet.h"
#include "transitionSystem.h"
#include "benchmark.h"

template<class T>
class SymbSearch {
	public:
		struct StrategyResult {
			bool success;
			std::vector<bool> reachability;
			std::vector<std::string> action_map;
			StrategyResult(int graph_size);
		};
		struct PlanResult {
			T pathcost;
			bool success;
			PlanResult(float mu, int num_dfas);
		};
	private:
		struct spaceWeight {
			int dfa_ind;
			std::vector<bool> reachability;
			std::vector<float> state_weights;
			std::vector<bool> is_inf;
		};	
		struct minWeight {
			std::vector<bool> is_inf;
			std::vector<float> min_weight;
			minWeight(int size) {
				is_inf.resize(size, true);
				min_weight.resize(size, 0.0f);
			}
			void reset() {
				for (int i=0; i<is_inf.size(); ++i) {
					is_inf[i] = true;
					min_weight[i] = 0.0f;
				}
			}
		};
		struct minLS {
			std::vector<bool> is_inf;
			std::unordered_map<int, int> prod2node_list;
			minLS(int size) {
				is_inf.resize(size, true);
				prod2node_list.clear();
			}
			void reset() {
				for (int i=0; i<is_inf.size(); ++i) {
					is_inf[i] = true;
				}
				prod2node_list.clear();
			}
		};
		const std::vector<DFA_EVAL*>* dfa_list_ordered;
		int num_dfas;
		TS_EVAL<State>* TS;
		float mu, pathlength;
		//std::vector<IVFlexLex<T>*> node_list;
		std::vector<T*> set_list;
		std::vector<std::string> TS_action_sequence;
		std::vector<int> TS_state_sequence;
		const std::string bench_mark_session;
		bool verbose, dfas_set, TS_set, mu_set, plan_found, use_benchmark;
		std::vector<spaceWeight> heuristic;
		Benchmark benchmark;

		IVFlexLex<T>* newNode();
		T* newSet();
		template<typename Q> void printQueue(Q queue);
		template<typename Q_f> void printQueueFloat(Q_f queue);
		void extractPath(const std::vector<IVFlexLex<T>*> node_list, const std::vector<int>& parents, int accepting_state, const std::vector<int>& graph_sizes);
		bool spaceSearch(TS_EVAL<State>* TS_sps, std::vector<DFA_EVAL*>* dfa_sps, spaceWeight& spw, std::function<float(float, unsigned int)> spwFunc, int max_depth = -1);
		bool riskSearch(TS_EVAL<State>* TS_sps, DFA_EVAL* dfa_sps, spaceWeight& spw, std::function<float(unsigned int)> cFunc);
		bool generateRisk(DFA_EVAL* cosafe_dfa, spaceWeight& spw);
		bool generateHeuristic();
		float pullStateWeight(unsigned ts_ind, unsigned dfa_ind, unsigned dfa_list_ind, bool& reachable) const;
		void clearNodes();
		PlanResult BFS(std::function<bool(const std::pair<int, T*>&, const std::pair<int, T*>&)> compare, std::function<bool(const T&, const T&)> acceptanceCompare, std::function<bool(const T&)> pruneCriterion, bool prune, bool extract_path, bool use_heuristic = false);
		void clearNodesAndSets();
		void resetSearchParameters();
	public:
		SymbSearch();
		SymbSearch(const std::string& bench_mark_session_, bool verbose_);
		void setAutomataPrefs(const std::vector<DFA_EVAL*>* dfa_list_ordered_);
		void setTransitionSystem(TS_EVAL<State>* TS_);
		void setFlexibilityParam(float mu_);
		std::pair<bool, float> search(bool use_heuristic = false);
		StrategyResult synthesizeRiskStrategy(DFA_EVAL* cosafe_dfa, DFA_EVAL* live_dfa);
		void writePlanToFile(std::string filename, const std::vector<std::string>& xtra_info);
		~SymbSearch();
};
