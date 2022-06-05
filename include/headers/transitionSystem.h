#pragma once
#include<string>
#include<vector>
#include<iostream>
#include<unordered_map>
#include<memory>
#include "graph.h"
#include "state.h"
#include "condition.h"

template <class T>
class TransitionSystem : public Graph<WL> {
	private:
		std::vector<Condition*> conditions;
		bool has_conditions;
		bool is_blocking;
		bool has_init_state;
		const bool UNIQUE_ACTION;
		const bool manual;
		T* init_state;
		std::vector<T> all_states;
		std::vector<bool> state_added;
		//unsigned int q_i;
		void safeAddState(int q_i, T* add_state, int add_state_ind, Condition* cond);
	protected:
		std::unordered_map<std::string, SimpleCondition*> propositions;
		std::vector<T*> state_map;
		std::vector<std::shared_ptr<WL>> node_container;
		bool generated;
	public:
	 	//TODO Move this to protected:
		bool parseLabelAndEval(const std::string& label, const T* state);
		TransitionSystem(bool UNIQUE_ACTION_ = true, bool manual_ = false);
		//unsigned int size() const;
		bool connect(T* src, T* dst, float weight, const std::string& action);
		void finishConnecting();
		void addCondition(Condition* condition_);
		void setConditions(const std::vector<Condition*>& conditions_);
		void addProposition(SimpleCondition* proposition_);
		void setPropositions(const std::vector<SimpleCondition*>& propositions_);
		void setInitState(T* init_state_);
		const T* getState(int node_index) const;
		void generate();
		//T compose(const T* mult_TS) const;
		void clear();
		void printTS();
		//~TransitionSystem();
};

template <class T>
class TS_EVAL : public TransitionSystem<T> {
	private:
	 	bool mapped;
		int curr_node, init_node;
		std::unordered_map<int, std::vector<std::string>> state_to_label_map;
	public:
		//TS_EVAL(const TransitionSystem<T>* tsptr_, int init_node);
		TS_EVAL(int init_node);
		TS_EVAL(bool UNIQUE_ACTION_, bool manual_, int init_node);
		void mapStatesToLabels(const std::vector<const DFA::alphabet_t*>& alphabet);
		bool eval(const std::string& action, bool evolve);
		bool evalReverse(const std::string& action, bool evolve);
		//bool isReversible() const;
		int getCurrNode() const;
		void getConnectedDataEVAL(std::vector<WL*>& con_data);
		void getConnectedNodesEVAL(std::vector<int>& con_nodes);
		void getParentDataEVAL(std::vector<WL*>& con_data);
		void getParentNodesEVAL(std::vector<int>& con_nodes);
		const std::vector<std::string>* returnStateLabels(int state_ind) const;
		void set(int set_node);
		void reset();
		const T* getCurrState() const;
};
