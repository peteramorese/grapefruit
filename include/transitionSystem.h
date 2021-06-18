#pragma once
#include<string>
#include<vector>
#include<iostream>
#include<unordered_map>
#include "edge.h"
#include "state.h"
#include "condition.h"

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
		unsigned int q_i;
		void safeAddState(T* add_state, int add_state_ind, Condition* cond);
	protected:
		Edge* graph_TS;
		std::vector<T*> state_map;
		bool generated;
	public:
		TransitionSystem(Edge* graph_TS_);
		void addCondition(Condition* condition_);
		void setConditions(const std::vector<Condition*>& conditions_);
		void setInitState(T* init_state_);
		T* getState(int node_index);
		void generate();
		T compose(const T* mult_TS) const;
		virtual void print() const;
};

template <class T>
class ProductSystem : public TransitionSystem<T> {
	private:
		//std::vector<SimpleCondition*> propositions;
		std::unordered_map<std::string, SimpleCondition*> propositions;
		bool automaton_init, plan_found;
		Edge* graph_DA;
		Edge* graph_product;
		std::vector<T*> prod_state_map;
		std::vector<int> prod_TS_index_map;
		std::vector<int> prod_DA_index_map;
		std::vector<int> accepting_DA_states;
		int init_state_DA_ind;
		unsigned int p_i;
		unsigned int TS_f, DA_f;
		std::vector<bool> prod_state_added;
		std::vector<bool> is_DA_accepting;
		std::vector<bool> is_accepting;
		std::vector<int> stored_plan;
		void safeAddProdState(T* add_state, int add_state_ind, float weight, const std::string& action);
	public:
		ProductSystem(Edge* graph_TS_, Edge* graph_DA_, Edge* graph_product_);
		void addProposition(SimpleCondition* proposition_);
		void setPropositions(const std::vector<SimpleCondition*>& propositions_);
		void setAutomatonInitStateIndex(int init_state_DA_ind_);
		void setAutomatonAcceptingStateIndices(const std::vector<int>& accepting_DA_states_);
		void addAutomatonAcceptingStateIndex(int accepting_DA_state_);
		bool parseLabelAndEval(const std::string& label, const T* state);
		void compose();
		bool plan(std::vector<int>& plan);
		bool plan();
		void getPlan(std::vector<T*>& state_sequence, std::vector<std::string>& action_sequence);
		void print() const;
};

