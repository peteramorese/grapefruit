#include<string>
#include<vector>
#include<iostream>
#include<unordered_map>
#include "state.h"
#include "stateSpace.h"


/* State DEFINITION */
/*
std::vector<std::vector<std::string>> State<SS_TYPE>::state_space_named;
std::unordered_map<std::string, unsigned int> State<SS_TYPE>::index_labels;
std::vector<int> State<SS_TYPE>::num_vars;
unsigned int State<SS_TYPE>::state_space_dim;
std::vector<State<SS_TYPE>::domain> State<SS_TYPE>::domains;
std::vector<State<SS_TYPE>::domain> State<SS_TYPE>::groups;
const std::string State<SS_TYPE>::UNDEF = "UNDEF";
bool State<SS_TYPE>::is_dimensions_defined = false;
*/

template<class SS_TYPE>
State<SS_TYPE>::State(SS_TYPE* SS_) : SS(SS_) {
	initNewState();
	if (!is_dimensions_defined) {
		std::cout<<"Error: Must define State Dimensions before instantiating a State\n";	
	}
}

/*
void State<SS_TYPE>::resizeAll(unsigned int size) {
	state_space_named.resize(size);
	num_vars.resize(size);
}

void State<SS_TYPE>::resizeAll() {
	state_space_named.resize(state_space_dim);
	num_vars.resize(state_space_dim);
}
*/

template<class SS_TYPE>
void State<SS_TYPE>::initNewState() {
	state_space.resize(SS->getStateSpaceDim);
	for (unsigned int i=0; i<SS->getStateSpaceDim; ++i) {
		// Init to UNDEF
		state_space[i] = SS->getVarOptionsCount(i) - 1;
	}
}


/*
void State<SS_TYPE>::setStateDimension(const std::vector<std::string>& var_labels, unsigned int dim) {
	is_dimensions_defined = true;
	if (dim+1 > state_space_dim) {
		state_space_dim = dim + 1;
		resizeAll();
	}	
	// The last label for each dimension is automatically set to be undefined
	std::vector<std::string> set_labels = var_labels;
	set_labels.push_back(UNDEF);
	state_space_named[dim] = set_labels;
	num_vars[dim] = set_labels.size();
}

void State<SS_TYPE>::generateAllPossibleStates(std::vector<State>& all_states) {
	int counter = 1;
	// Count the number of possible states using a permutation. Omit the last element in each
	// state_space_named array because it represents UNDEF
	std::vector<int> column_wrapper(state_space_dim);
	std::vector<int> digits(state_space_dim);
	for (int i=0; i<state_space_dim; i++){
		int inds = state_space_named[i].size() - 1;
		counter *= inds;
		column_wrapper[i] = inds;
		digits[i] = 0;
	}
	all_states.resize(counter);
	int a = 0;
	int b = 0;
	for (int i=0; i<counter; i++) {
		for (int ii=0; ii<state_space_dim; ii++){
			all_states[i].setState(state_space_named[ii][digits[ii]],ii);
		}
		digits[0]++;
		for (int ii=0; ii<state_space_dim; ii++){
			if (digits[ii] > column_wrapper[ii]-1) {
				digits[ii] = 0;
				digits[ii+1]++;
			}
		}
	}
}

int State<SS_TYPE>::getVarOptionsCount(unsigned int dim) {
	if (dim < state_space_dim){
		return state_space_named[dim].size();
	} else {
		std::cout<<"Error: Index out of bounds\n";
	}
}

void State<SS_TYPE>::setStateDimensionLabel(unsigned int dim, const std::string& dimension_label){
	if (dim < state_space_dim) {
		index_labels[dimension_label] = dim;
	} else {
		std::cout<<"Error: Index out of bounds\n";
	}
}

void State<SS_TYPE>::setDomain(const std::string& domain_label, const std::vector<std::string>& vars){
	bool names_found = true;
	for (int i=0; i<vars.size(); i++){
		bool names_found_i = false;
		for (int ii=0; ii<state_space_dim; ii++){
			for (int iii=0; iii<state_space_named[ii].size(); iii++){
				if (vars[i] == state_space_named[ii][iii]) {
					names_found_i = true;
					break;
				}
			}
			if (names_found_i) {
				break;
			}
		}
		if (!names_found_i){
			names_found = false;
			break;
		}
	}
	if (names_found) {
		domain add_domain;
		add_domain.label = domain_label;
		add_domain.vars = vars;
		domains.push_back(add_domain);
	} else {
		std::cout<<"Error: At least one variable was not recognized in input vector. Make sure to call setStateDimension before setting domains\n";
	}
}

void State<SS_TYPE>::setDomain(const std::string& domain_label, const std::vector<std::string>& vars, unsigned int index){
	bool names_found = true;
	for (int i=0; i<vars.size(); i++){
		bool names_found_i = false;
		for (int ii=0; ii<state_space_dim; ii++){
			for (int iii=0; iii<state_space_named[ii].size(); iii++){
				if (vars[i] == state_space_named[ii][iii]) {
					names_found_i = true;
					break;
				}
			}
			if (names_found_i) {
				break;
			}
		}
		if (!names_found_i){
			names_found = false;
			break;
		}
	}
	if (names_found) {
		if (index+1 > domains.size()) {
			domains.resize(index+1);
		}
		domain add_domain;
		add_domain.label = domain_label;
		add_domain.vars = vars;
		domains[index] = add_domain;
	} else {
		std::cout<<"Error: At least one variable was not recognized in input vector. Make sure to call setStateDimension before setting domains\n";
	}
}

bool State<SS_TYPE>::getDomains(const std::string& var, std::vector<std::string>& in_domains) {
	in_domains.clear();
	bool found = false;
	for (int i=0; i<domains.size(); i++){
		for (int ii=0; ii<domains[i].vars.size(); ii++) {
			if (var == domains[i].vars[ii]){
				in_domains.push_back(domains[i].label);
				found = true;
			}
		}
	}
	return found;
}

void State<SS_TYPE>::setLabelGroup(const std::string& group_label, const std::vector<std::string>& dimension_labels) {
	domain add_group;
	add_group.label = group_label;
	add_group.vars = dimension_labels;
	groups.push_back(add_group);

}

void State<SS_TYPE>::setLabelGroup(const std::string& group_label, const std::vector<std::string>& dimension_labels, unsigned int index) {
	if (index+1 > groups.size()) {
		groups.resize(index+1);
	}
	domain add_group;
	add_group.label = group_label;
	add_group.vars = dimension_labels;
	groups[index] = add_group;
}

void State<SS_TYPE>::getGroupDimLabels(const std::string& group_label, std::vector<std::string>& group_dim_labels) const {
	for (int i=0; i<groups.size(); ++i) {
		if (groups[i].label == group_label) {
			group_dim_labels = groups[i].vars;
			break;
		}
	}
}

bool State<SS_TYPE>::argFindGroup(const std::string& var_find, const std::string& group_label, std::string& arg_dimension_label) const {
	bool is_found = false;
	arg_dimension_label = "NOT FOUND";
	for (int i=0; i<groups.size(); i++) {
		if (groups[i].label == group_label) {
			for (int ii=0; ii<groups[i].vars.size(); ii++){
				std::string dim_label = groups[i].vars[ii];
				int ind;
				ind = index_labels[dim_label];
				if (state_space_named[ind][state_space[ind]] == var_find) {
					is_found = true;
					arg_dimension_label = dim_label;
					break;
				}
			}
			if (is_found) {
				break;
			}
		}
	}
	return is_found;
}
*/

template<class SS_TYPE>
bool State<SS_TYPE>::argFindGroup(const std::string& var_find, const std::string& group_label, std::string& arg_dimension_label) const {
	bool found = SS->argFindGroupSS(var_find, group_label, arg_dimension_label, state_space);
	return found;
}

template<class SS_TYPE>
void State<SS_TYPE>::setState(const std::vector<std::string>& set_state) {
	bool found = SS->findVar(set_state, state_space);
	if (!found) {
		std::cout<<"Error: Cannot set state\n";
	}
	
}

template<class SS_TYPE>
void State<SS_TYPE>::setState(const std::string& set_state_var, unsigned int dim) {
	int var_ind;
	bool found = SS->findVar(set_state_var, dim, var_ind);
	if (found) {
		state_space[dim] = var_ind;
	else {
		std::cout<<"Error: Cannot set state dimension\n";
	}
}

template<class SS_TYPE>
void State<SS_TYPE>::getState(std::vector<std::string>& ret_state) const {
	ret_state.clear();
	ret_state.resize(state_space_dim);
	for (int i=0; i<state_space_dim; i++){
		SS->indexGetVar(i, state_space[i], ret_state[i]);
	}
}

template<class SS_TYPE>
std::string State<SS_TYPE>::getVar(const std::string& dimension_label) const {
	unsigned int ind = SS->getStateDimensionLabel(dimension_label);
	int named_ind = state_space[ind];
	std::string ret_var;
       	//return state_space_named[ind][named_ind];
	return SS->indexGetVar(ind, named_ind, ret_var);
}

template<class SS_TYPE>
bool State<SS_TYPE>::isDefined() const {
	bool ret_bool = true;
	for (int i=0; i<state_space_dim; i++){
		// If the state space index is the last value for the dimension, it was
		// automatically set to be undefined
		if (state_space[i] == SS->getVarOptionsCount(i)-1) {
			ret_bool = false;
		}	
	}
	return ret_bool;
}

template<class SS_TYPE>
void State<SS_TYPE>::print() const {
	std::string print_var;
	for (int i=0; i<state_space_dim; i++){
		SS->indexGetVar(i, state_space[i], print_var);
		std::cout<<"State variable "<< i << " = " << print_var<< "\n";
	}
}

template<class SS_TYPE>
bool State<SS_TYPE>::exclEquals(const State* state_ptr_, const std::vector<std::string>& excl_dimension_labels) const {
	bool ret_bool = true;
	std::vector<bool> check(state_space_dim);
	for (int i=0; i<state_space_dim; i++) {
		check[i] = true;
	}
	for (int i=0; i<excl_dimension_labels.size(); i++){
		int ind = SS->getStateDimensionLabel(excl_dimension_labels[i]);
		check[ind] = false;
	}
	for (int i=0; i<state_space_dim; i++){
		if (check[i]) {
			if (state_space[i] != state_ptr_->state_space[i]) {
				ret_bool = false;
				break; 
			}	
		}
	}
	return ret_bool;
}

template<class SS_TYPE>
bool State<SS_TYPE>::operator== (const State& state_) const {
	return (state_space == state_.state_space);
}

template<class SS_TYPE>
bool State<SS_TYPE>::operator== (const State* state_ptr_) const {
	return (state_space == state_ptr_->state_space);
}

template<class SS_TYPE>
void State<SS_TYPE>::operator= (const State& state_eq) {
	state_space = state_eq.state_space;
}

template<class SS_TYPE>
void State<SS_TYPE>::operator= (const State* state_eq_ptr) {
	state_space = state_eq_ptr->state_space;
}

template class State<StateSpace>;

/* BlockingState DEFINTION */

/*
std::vector<bool> BlockingState<SS_TYPE>::blocking_dims;
bool BlockingState<SS_TYPE>::debug = false;
*/

/*
void BlockingState<SS_TYPE>::toggleDebug(bool debug_) {
	debug = debug_;
}

void BlockingState<SS_TYPE>::setBlockingDim(const std::vector<bool>& blocking_dims_) {
	blocking_dims = blocking_dims_;
}

void BlockingState<SS_TYPE>::setBlockingDim(bool blocking, unsigned int dim) {
	if (dim < state_space_dim){
		blocking_dims[dim] = blocking;
	} else {
		std::cout<<"Error: Dimension index out of bounds\n";
	}
}
*/

template<class SS_TYPE>
bool BlockingState<SS_TYPE>::setState(const std::vector<std::string>& set_state) {
	if (set_state.size() == state_space_dim){
		bool conflict = false;
		bool names_found;
		if (set_state.size() > 1){
			for (int i=0; i<set_state.size()-1; i++) {
				if (SS->isBlocking(i)) {
					for (int ii=i+1; ii<set_state.size(); ii++) {
						if (set_state[i] == set_state[ii]) {
							if (debug) {
								std::cout<<"Warning: Cannot set Blocking State, duplication location: "<<set_state[i]<<"\n";
							}
							conflict = true;
							goto blocking;
						}
					}
				}
			}
		}

		/*
		for (int i=0; i<state_space_dim; i++){
			bool names_found_i = false;
			for (int ii=0; ii<num_vars[i]; ii++){
				if (state_space_named[i][ii] == set_state[i]){
					names_found_i = true;
					state_space[i] = ii;
				}
			}
			if (!names_found_i) {
				names_found = false;
			}

		}	
		*/
		
		names_found = SS->findVar(set_state, state_space);
		if (!names_found) {
			std::cout<<"Error: Unrecognized label in set state\n";
		} else {
			return true;
		}
blocking:
		if (conflict) {
			return false;
			std::cout<<"  Set state will not be set...\n";
		}
	} else {
		std::cout<<"Error: Set state must have same dimension as state space\n";
	}
}

template<class SS_TYPE>
bool BlockingState<SS_TYPE>::setState(const std::string& set_state_var, unsigned int dim) {
	bool name_found;
	std::string temp_str;
	int temp_ind;
	if (dim+1 > state_space_dim) {
		std::cout<<"Error: Dimension out of bounds\n";
	} else {
		bool conflict = false;
		if (SS->isBlocking(dim)) {
			for (int i=0; i<state_space_dim; i++) {
				if (SS->isBlocking(i) && i!=dim) {
					//std::cout<<"statevar: "<<state_space_named[i][state_space[i]]<<std::endl;
					temp_str = SS->indexGetVar(i, state_space[i], temp_str);
					if (temp_str == set_state_var) {
						if (debug) {
							std::cout<<"Warning: Cannot set Blocking State, duplication location: "<<set_state_var<<"\n";
						}
						conflict = true;
						goto blocking;
					}
				}
			}
		}
		/*
		for (int i=0; i<SS->getVarOptionsCount(dim); i++){
			if (state_space_named[dim][i] == set_state_var) {
				name_found = true;
				state_space[dim] = i;
				break;
			}
		}
		*/
		name_found = SS->findVar(var_find, dim, temp_ind);
		state_space[dim] = temp_ind;
		if (!name_found) {
			std::cout<<"Error: Unrecognized label in set state\n";
		} else {
			return true;
		}
blocking:
		if (conflict) {
			return false;
			std::cout<<"  Set state will not be set...\n";
		}

	}
}

template class BlockingState<BlockingStateSpace>;

/*
void BlockingState<SS_TYPE>::generateAllPossibleStates(std::vector<BlockingState>& all_states) {
	int counter = 1;
	// Count the number of possible states using a permutation. Omit the last element in each
	// state_space_named array because it represents UNDEF
	std::vector<int> column_wrapper(state_space_dim);
	std::vector<int> digits(state_space_dim);
	for (int i=0; i<state_space_dim; i++){
		int inds = state_space_named[i].size() - 1;
		counter *= inds;
		column_wrapper[i] = inds;
		digits[i] = 0;
	}
	all_states.resize(counter);
	int i = 0;
	int j = 0;
	while (i<counter) {
		bool non_blocking = true;
		for (int ii=0; ii<state_space_dim; ii++){
			bool non_blocking_i = all_states[j].setState(state_space_named[ii][digits[ii]],ii);
			non_blocking = non_blocking && non_blocking_i;
			if (!non_blocking) {
				break;
			}
		}
		if (non_blocking) {
			i++;
			j++;
		} else {
			i++;
		}
		digits[0]++;
		for (int ii=0; ii<state_space_dim; ii++){
			if (digits[ii] > column_wrapper[ii]-1) {
				digits[ii] = 0;
				if (ii != state_space_dim-1) {
					digits[ii+1]++;
				} else {
					goto loopexit;
				}
			}
		}
	}
loopexit:
	all_states.resize(j);
	std::cout<<"Info: Generated "<<j<<" blocking states out of "<<counter<<" possible states\n";
}
*/


