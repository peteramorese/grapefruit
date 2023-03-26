#pragma once
#include<vector>
#include<iostream>
#include<unordered_map>
#include<memory>
#include "state.h"

class State;
class BlockingState;

class StateSpace {
	protected:
		std::vector<std::vector<std::string>> state_space_named;
		std::unordered_map<std::string, unsigned int> index_labels;
		std::vector<std::string> index_labels_rev;
		std::vector<int> num_vars;
		//std::vector<int> state_space;
		unsigned int state_space_dim;
		bool is_dimensions_defined;
		struct domain {
			std::string label;
			std::vector<std::string> vars;
		};
		std::vector<domain> domains;
		std::vector<domain> groups;
		friend class State;
	public:
		StateSpace();
		void resizeAll_(unsigned int size);
		void resizeAll_();
		//void initNewSS();
		static const std::string UNDEF;
		void setStateDimension(const std::vector<std::string>& var_labels, unsigned int dim);
		void generateAllPossibleStates_(std::vector<State>& all_states) ;
		unsigned int getDim() const;
		int getVarOptionsCount_(unsigned int dim);
		void setStateDimensionLabel(unsigned int dim, const std::string& dimension_label);
		void setDomain(const std::string& domain_label, const std::vector<std::string>& vars);
		void setDomain(const std::string& domain_label, const std::vector<std::string>& vars, unsigned int index);
		bool getDomains_(const std::string& var, std::vector<std::string>& in_domains) const;
		void setLabelGroup(const std::string& group_label, const std::vector<std::string>& dimension_labels);
		void setLabelGroup(const std::string& group_label, const std::vector<std::string>& dimension_labels, unsigned int index);
		void getGroupDimLabels_(const std::string& group_label, std::vector<std::string>& group_dim_labels) const;
		bool argFindGroup_(const std::string& var_find, const std::string& group_label, std::string& arg_dimension_label, const std::vector<int>& state_space); 
		void setState_(const std::vector<std::string>& set_state, std::vector<int>& state_space);
		void setState_(const std::string& set_state_var, unsigned int dim, std::vector<int>& state_space);
		void getState_(std::vector<std::string>& ret_state, const std::vector<int>& state_space) const;
		std::string getVar_(const std::string& dimension_label, const std::vector<int>& state_space);
		bool isDefined_(const std::vector<int>& state_space) const;
		void print_(const std::vector<int>& state_space) const;
		bool exclEquals_(const State* state_ptr_, const std::vector<std::string>& excl_dimension_labels, const std::vector<int>& state_space);
		void writeToFile(const std::string& filename) const;
		// string methods that don't exist in CXX 20:
		static bool starts_with(const std::string& str, const std::string& prefix); // in case CXX 20 cant be used
		static std::string::iterator str_find(std::string* str, char stop_char);

		static std::shared_ptr<StateSpace> readFromFile(const std::string& filename);
		/*
		bool operator== (const State& state_) const;
		bool operator== (const State* state_ptr_) const;
		void operator= (const State& state_eq);
		void operator= (const State* state_eq_ptr);
		*/
};


class BlockingStateSpace : public StateSpace {
	private:
		std::vector<bool> blocking_dims;
		bool debug;
	public:
		void toggleDebug_(bool debug_);
		void setBlockingDim_(const std::vector<bool>& blocking_dims_);
		void setBlockingDim_(bool blocking, unsigned int dim);
		void generateAllPossibleStates_(std::vector<BlockingState>& all_states) ;
		bool setState_(const std::vector<std::string>& set_state, std::vector<int>& state_space);
		bool setState_(const std::string& set_state_var, unsigned int dim, std::vector<int>& state_space);
};


