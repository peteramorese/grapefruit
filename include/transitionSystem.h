#pragma once
#include<string>
#include<vector>
#include<iostream>
#include<unordered_map>
#include "graph.h"
#include "state.h"
#include "condition.h"
#include "astar.h"

template <class T>
class TransitionSystem {
	private:
		std::vector<Condition*> conditions;
		bool has_conditions;
		bool is_blocking;
		bool has_init_state;
		T* init_state;
		std::vector<T> all_states;
		std::vector<bool> state_added;
		//unsigned int q_i;
		void safeAddState(int q_i, T* add_state, int add_state_ind, Condition* cond);
	protected:
		std::unordered_map<std::string, SimpleCondition*> propositions;
		Graph<WL>* graph_TS;
		std::vector<T*> state_map;
		std::vector<WL*> node_container;
		bool generated;
		bool parseLabelAndEval(const std::string* label, const T* state);
	public:
		TransitionSystem(Graph<WL>* graph_TS_);
		void addCondition(Condition* condition_);
		void setConditions(const std::vector<Condition*>& conditions_);
		void addProposition(SimpleCondition* proposition_);
		void setPropositions(const std::vector<SimpleCondition*>& propositions_);
		void setInitState(T* init_state_);
		T* getState(int node_index);
		void generate();
		//T compose(const T* mult_TS) const;
		void clearTS();
		void printTS() const;
		~TransitionSystem();
};

template <class T>
class TS_EVAL {
	private:
		const TransitionSystem<T>* tsptr;
		int curr_node;
		friend class TransitionSystem<T>;
		std::unordered_map<int, std::vector<std::string>> state_to_label_map;
	public:
		TS_EVAL(const TransitionSystem<T>* tsptr_, int init_node);
		void mapStatesToLabels(const std::vector<const std::vector<std::string>*>& alphabet);
		bool eval(const std::string& action);
		int getCurrNode() const;
		void getConnectedDataEVAL(std::vector<WL*>& con_data);
		const std::vector<std::string>* returnStateLabels(int state_ind);
		void set(int set_node);
		void reset(int init_node);
		T* getCurrState() const;
};

template <class T>
class ProductSystem : public TransitionSystem<T> {
	private:
		//std::vector<SimpleCondition*> propositions;
		bool automaton_init, plan_found;
		DFA* graph_DFA;
		Graph<WL>* graph_product;
		std::vector<T*> prod_state_map;
		std::vector<WL*> prod_node_container;
		std::vector<int> prod_TS_index_map;
		std::vector<int> prod_DFA_index_map;
		const std::vector<unsigned int>* accepting_DFA_states;
		int init_state_DFA_ind;
		//unsigned int p_i;
		unsigned int TS_f, DFA_f;
		std::vector<bool> prod_state_added;
		std::vector<bool> is_DFA_accepting;
		std::vector<bool> is_accepting;
		std::vector<int> stored_plan;
		void safeAddProdState(int p_i, T* add_state, int add_state_ind, float weight, const std::string& action);
	public:
		ProductSystem(Graph<WL>* graph_TS_, DFA* graph_DFA_, Graph<WL>* graph_product_);
		void setAutomatonInitStateIndex(int init_state_DFA_ind_);
		void setAutomatonAcceptingStateIndices(const std::vector<int>& accepting_DFA_states_);
		void addAutomatonAcceptingStateIndex(int accepting_DFA_state_);
		void compose();
		bool plan(std::vector<int>& plan);
		bool plan();
		void getPlan(std::vector<T*>& state_sequence, std::vector<std::string>& action_sequence);
		float getEdgeWeight(unsigned int action_ind) const;
		void updateEdgeWeight(unsigned int action_ind, float weight);
		void clearPS();
		void printPS() const;
		~ProductSystem();
};

