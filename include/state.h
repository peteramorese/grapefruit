#ifndef STATE_H
#define STATE_H
#include<vector>
#include<iostream>
#include<unordered_map>
class State {
	protected:
		static std::vector<std::vector<std::string>> state_space_named;
		static std::unordered_map<std::string, unsigned int> index_labels;
		static std::vector<int> num_vars;
		std::vector<int> state_space;
		static unsigned int state_space_dim;
		static bool is_dimensions_defined;
		struct domain {
			std::string label;
			std::vector<std::string> vars;
		};
		static std::vector<domain> domains;
		static std::vector<domain> groups;
	public:
		State();
		static void resizeAll(unsigned int size);
		static void resizeAll();
		void initNewSS();
		static const std::string UNDEF;
		static void setStateDimension(const std::vector<std::string>& var_labels, unsigned int dim);
		static void generateAllPossibleStates(std::vector<State>& all_states) ;
		static int getVarOptionsCount(unsigned int dim);
		static void setStateDimensionLabel(unsigned int dim, const std::string& dimension_label);
		static void setDomain(const std::string& domain_label, const std::vector<std::string>& vars);
		static void setDomain(const std::string& domain_label, const std::vector<std::string>& vars, unsigned int index);
		static bool getDomains(const std::string& var, std::vector<std::string>& in_domains);
		static void setLabelGroup(const std::string& group_label, const std::vector<std::string>& dimension_labels);
		static void setLabelGroup(const std::string& group_label, const std::vector<std::string>& dimension_labels, unsigned int index);
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
};

#endif

#ifndef BLOCKINGSTATE_H
#define BLOCKINGSTATE_H

class BlockingState : public State {
	private:
		static std::vector<bool> blocking_dims;
		static bool debug;
	public:
		static void toggleDebug(bool debug_);
		static void setBlockingDim(const std::vector<bool>& blocking_dims_);
		static void setBlockingDim(bool blocking, unsigned int dim);
		static void generateAllPossibleStates(std::vector<BlockingState>& all_states) ;
		bool setState(const std::vector<std::string>& set_state);
		bool setState(const std::string& set_state_var, unsigned int dim);
};

#endif

#ifndef AUGMENTEDSTATE_H
#define AUGMENTEDSTATE_H

class AugState : public State {
	private:
		static std::vector<bool> blocking_dims;
		static bool debug;
	public:
		static void toggleDebug(bool debug_);
		static void setBlockingDim(const std::vector<bool>& blocking_dims_);
		static void setBlockingDim(bool blocking, unsigned int dim);
		static void generateAllPossibleStates(std::vector<BlockingState>& all_states) ;
		bool setState(const std::vector<std::string>& set_state);
		bool setState(const std::string& set_state_var, unsigned int dim);
};

#endif


