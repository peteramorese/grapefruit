#pragma once
#include<vector>
#include<iostream>
#include<unordered_map>
#include "stateSpace.h"

class StateSpace;
class BlockingStateSpace;

class State {
	protected:
		//std::vector<std::vector<std::string>> state_space_named;
		//std::unordered_map<std::string, unsigned int> index_labels;
		//std::vector<int> num_vars;
		std::vector<int> state_space;
		//unsigned int state_space_dim;
		//bool is_dimensions_defined;
		/*
		struct domain {
			std::string label;
			std::vector<std::string> vars;
		};
		*/
		//std::vector<domain> domains;
		//std::vector<domain> groups;
		StateSpace* SS;
		//int ssAccess(int ind) const;
	public:
		State(StateSpace* SS_);
		void resizeAll(unsigned int size);
		void resizeAll();
		//void initNewSS();
		static const std::string UNDEF;
		//void setStateDimension(const std::vector<std::string>& var_labels, unsigned int dim);
		void generateAllPossibleStates(std::vector<State>& all_states) ;
		int getVarOptionsCount(unsigned int dim);
		//void setStateDimensionLabel(unsigned int dim, const std::string& dimension_label);
		//void setDomain(const std::string& domain_label, const std::vector<std::string>& vars);
		//void setDomain(const std::string& domain_label, const std::vector<std::string>& vars, unsigned int index);
		bool getDomains(const std::string& var, std::vector<std::string>& in_domains) const;
		//void setLabelGroup(const std::string& group_label, const std::vector<std::string>& dimension_labels);
		//void setLabelGroup(const std::string& group_label, const std::vector<std::string>& dimension_labels, unsigned int index);
		void getGroupDimLabels(const std::string& group_label, std::vector<std::string>& group_dim_labels) const;
		bool argFindGroup(const std::string& var_find, const std::string& group_label, std::string& arg_dimension_label) const; 
		void setState(const std::vector<std::string>& set_state);
		void setState(const std::string& set_state_var, unsigned int dim);
		void getState(std::vector<std::string>& ret_state) const;
		std::string getVar(const std::string& dimension_label) const;
		bool isDefined() const;
		void print() const;
		bool exclEquals(const State* state_ptr_, const std::vector<std::string>& excl_dimension_labels) const;
		bool operator== (const State& state_) const;
		bool operator== (const State* state_ptr_) const;
		void operator= (const State& state_eq);
		void operator= (const State* state_eq_ptr);
	friend class StateSpace;
};


class BlockingState : public State {
	private:
		//std::vector<bool> blocking_dims;
		//bool debug;
		BlockingStateSpace* BSS;
	public:
		BlockingState(BlockingStateSpace* BSS_);
		void toggleDebug(bool debug_);
		void setBlockingDim(const std::vector<bool>& blocking_dims_);
		void setBlockingDim(bool blocking, unsigned int dim);
		void generateAllPossibleStates(std::vector<BlockingState>& all_states) ;
		bool setState(const std::vector<std::string>& set_state);
		bool setState(const std::string& set_state_var, unsigned int dim);
};


