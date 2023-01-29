#pragma once
#include<queue>
#include<memory>
#include "graph.h"
#include "lexSet.h"
#include "transitionSystem.h"
#include "benchmark.h"

class SymbSearch {
	public:
		struct StrategyResult {
			bool success;
			std::vector<bool> reachability;
			std::vector<std::string> action_map;
			StrategyResult(int graph_size);
		};
		struct PlanResult {
			DetourLex pathcost;
			bool success;
			PlanResult(int num_dfas, float mu);
		};
		template<class LS> using nodeContainer_t = std::vector<std::shared_ptr<IVFlexLex<LS>>>;
		template<class LS> using setContainer_t = std::vector<std::shared_ptr<LS>>;
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
			minWeight(int size) { //TODO move this out of header
				is_inf.resize(size, true);
				min_weight.resize(size, 0.0f);
			}
			void reset() { //TODO move this out of header
				for (int i=0; i<is_inf.size(); ++i) {
					is_inf[i] = true;
					min_weight[i] = 0.0f;
				}
			}
		};
		struct minWeightLS {
			std::vector<bool> is_inf;
			std::vector<LexSet> min_weight;
			minWeightLS(int set_size, int size) { //TODO move this out of header
				is_inf.resize(size, true);
				min_weight.resize(size, LexSet(set_size));
			}
			void reset() { //TODO move this out of header
				for (int i=0; i<is_inf.size(); ++i) {
					is_inf[i] = true;
					min_weight[i].fill(0.0f);
				}
			}
		};
		struct minLS {
			std::vector<bool> is_inf;
			std::unordered_map<int, int> prod2node_list;
			minLS(int size) { //TODO move this out of header
				is_inf.resize(size, true);
				prod2node_list.clear();
			}
			void reset() { //TODO move this out of header
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
		//std::vector<IVFlexLex<DetourLex>*> node_list;
		//std::vector<IVLex> node_list_ls;
		//std::vector<DetourLex*> set_list;
		//std::vector<LexSet*> set_list_ls;
		std::vector<std::string> TS_action_sequence;
		std::vector<int> TS_state_sequence;
		//const std::string* bench_mark_session;
		bool verbose, dfas_set, TS_set, mu_set, plan_found, use_benchmark;
		std::vector<spaceWeight> heuristic;
		Benchmark benchmark;

		// DetourLexODO remove LS methods
		template<typename LS> std::weak_ptr<IVFlexLex<LS>> newNode(unsigned num_dfas, float mu, nodeContainer_t<LS>& node_container);
		//IVLex* newNodeLS(unsigned node_size, unsigned set_size);
		//DetourLex* newSet();
		//LexSet* newSetLS(unsigned set_size);
		template<typename LS> std::weak_ptr<LS> newSet(unsigned set_size, setContainer_t<LS>& set_container);
		template<typename Q> void printQueue(Q queue);
		template<typename Q_f> void printQueueFloat(Q_f queue);
		void extractPath(const std::vector<int>& parents, int accepting_state, const std::vector<int>& graph_sizes, const nodeContainer_t<DetourLex>& node_container);
		bool spaceSearch(TS_EVAL<State>* TS_sps, std::vector<DFA_EVAL*>* dfa_sps, spaceWeight& spw, std::function<float(float, unsigned int)> spwFunc, int max_depth = -1);
		bool generateRisk(TS_EVAL<State>* TS_sps, DFA_EVAL* cosafe_dfa, spaceWeight& spw);
		bool generateHeuristic();
		float pullStateWeight(unsigned ts_ind, unsigned dfa_ind, unsigned dfa_list_ind, bool& reachable) const;
		//void clearNodes();
		//void clearSetsLS();
		//void clearNodesLS();
		PlanResult BFS(std::function<bool(const std::pair<int, DetourLex*>&, const std::pair<int, DetourLex*>&)> compare, std::function<bool(const DetourLex&, const DetourLex&)> acceptanceCompare, std::function<bool(const DetourLex&)> pruneCriterion, bool prune, bool extract_path, bool use_heuristic = false);
		//void clearNodesAndSets();
		//void clearNodesAndSetsLS();
		void resetSearchParameters();
	public:
		SymbSearch();
		SymbSearch(const std::string* bench_mark_session_, bool verbose_);
		void setAutomataPrefs(const std::vector<DFA_EVAL*>* dfa_list_ordered_);
		void setTransitionSystem(TS_EVAL<State>* TS_);
		void setFlexibilityParam(float mu_);
		std::pair<bool, float> search(bool use_heuristic = false);
		StrategyResult synthesizeRiskStrategy(TS_EVAL<State>* TS_sps, DFA_EVAL* cosafe_dfa, DFA_EVAL* live_dfa);
		void writePlanToFile(std::string filename, const std::vector<std::string>& xtra_info);
		//~SymbSearch();
};
