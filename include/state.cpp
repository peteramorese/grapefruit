#include<string>
#include<vector>
#include<iostream>
#include<unordered_map>
#include "state.h"


/* State DEFINITION */
std::vector<std::vector<std::string>> State::state_space_named;
std::unordered_map<std::string, unsigned int> State::index_labels;
std::vector<int> State::num_vars;
unsigned int State::state_space_dim;
std::vector<State::domain> State::domains;
std::vector<State::domain> State::groups;
const std::string State::UNDEF = "UNDEF";
bool State::is_dimensions_defined = false;

State::State() {
	// Set Static Variables (setStateDimension) before instantiating a State
	initNewSS();
	if (!is_dimensions_defined) {
		std::cout<<"Error: Must define State Dimensions before instantiating a State\n";	
	}
}

void State::resizeAll(unsigned int size) {
	state_space_named.resize(size);
	num_vars.resize(size);
}

void State::resizeAll() {
	state_space_named.resize(state_space_dim);
	num_vars.resize(state_space_dim);
}

void State::initNewSS() {
	state_space.resize(state_space_dim);
	for (int i=0; i<state_space_dim; ++i) {
		state_space[i] = num_vars[i]-1;
	}
}


void State::setStateDimension(const std::vector<std::string>& var_labels, unsigned int dim) {
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

void State::generateAllPossibleStates(std::vector<State>& all_states) {
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

int State::getVarOptionsCount(unsigned int dim) {
	if (dim < state_space_dim){
		return state_space_named[dim].size();
	} else {
		std::cout<<"Error: Index out of bounds\n";
	}
}

void State::setStateDimensionLabel(unsigned int dim, const std::string& dimension_label){
	if (dim < state_space_dim) {
		index_labels[dimension_label] = dim;
	} else {
		std::cout<<"Error: Index out of bounds\n";
	}
}

void State::setDomain(const std::string& domain_label, const std::vector<std::string>& vars){
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

void State::setDomain(const std::string& domain_label, const std::vector<std::string>& vars, unsigned int index){
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

bool State::getDomains(const std::string& var, std::vector<std::string>& in_domains) {
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

void State::setLabelGroup(const std::string& group_label, const std::vector<std::string>& dimension_labels) {
	domain add_group;
	add_group.label = group_label;
	add_group.vars = dimension_labels;
	groups.push_back(add_group);

}

void State::setLabelGroup(const std::string& group_label, const std::vector<std::string>& dimension_labels, unsigned int index) {
	if (index+1 > groups.size()) {
		groups.resize(index+1);
	}
	domain add_group;
	add_group.label = group_label;
	add_group.vars = dimension_labels;
	groups[index] = add_group;
}

void State::getGroupDimLabels(const std::string& group_label, std::vector<std::string>& group_dim_labels) const {
	for (int i=0; i<groups.size(); ++i) {
		if (groups[i].label == group_label) {
			group_dim_labels = groups[i].vars;
			break;
		}
	}
}

bool State::argFindGroup(const std::string& var_find, const std::string& group_label, std::string& arg_dimension_label) const {
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

void State::setState(const std::vector<std::string>& set_state) {
	if (set_state.size() == state_space_dim){
		bool names_found = true;
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
		if (!names_found) {
			std::cout<<"Error: Unrecognized label in set state\n";
		}
	} else {
		std::cout<<"Error: Set state must have same dimension as state space\n";
	}
}

void State::setState(const std::string& set_state_var, unsigned int dim) {
	bool name_found = false;
	if (dim+1 > state_space_dim) {
		std::cout<<"Error: Dimension out of bounds\n";
	} else {
		for (int i=0; i<num_vars[i]; i++){
			if (state_space_named[dim][i] == set_state_var) {
				name_found = true;
				state_space[dim] = i;
			}
		}
	}
}

void State::getState(std::vector<std::string>& ret_state) const {
	ret_state.clear();
	ret_state.resize(state_space_dim);
	for (int i=0; i<state_space_dim; i++){
		ret_state[i] = state_space_named[i][state_space[i]];
	}
}

std::string State::getVar(const std::string& dimension_label) const {
	unsigned int ind = index_labels[dimension_label];
	int named_ind = state_space[ind];
       	return state_space_named[ind][named_ind];
}

bool State::isDefined() const {
	bool ret_bool = true;
	for (int i=0; i<state_space_dim; i++){
		// If the state space index is the last value for the dimension, it was
		// automatically set to be undefined
		if (state_space[i] == num_vars[i]-1) {
			ret_bool = false;
		}	
	}
	return ret_bool;
}

void State::print() const {
	for (int i=0; i<state_space_dim; i++){
		std::cout<<"State variable "<< i << " = " << state_space_named[i][state_space[i]]<< "\n";
	}
}

bool State::exclEquals(const State* state_ptr_, const std::vector<std::string>& excl_dimension_labels) const {
	bool ret_bool = true;
	std::vector<bool> check(state_space_dim);
	for (int i=0; i<state_space_dim; i++) {
		check[i] = true;
	}
	for (int i=0; i<excl_dimension_labels.size(); i++){
		int ind = index_labels[excl_dimension_labels[i]];
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

bool State::operator== (const State& state_) const {
	return (state_space == state_.state_space);
}

bool State::operator== (const State* state_ptr_) const {
	return (state_space == state_ptr_->state_space);
}

void State::operator= (const State& state_eq) {
	state_space = state_eq.state_space;
}

void State::operator= (const State* state_eq_ptr) {
	state_space = state_eq_ptr->state_space;
}

/* BlockingState DEFINTION */

std::vector<bool> BlockingState::blocking_dims;
bool BlockingState::debug = false;

void BlockingState::toggleDebug(bool debug_) {
	debug = debug_;
}

void BlockingState::setBlockingDim(const std::vector<bool>& blocking_dims_) {
	blocking_dims = blocking_dims_;
}

void BlockingState::setBlockingDim(bool blocking, unsigned int dim) {
	if (dim < state_space_dim){
		blocking_dims[dim] = blocking;
	} else {
		std::cout<<"Error: Dimension index out of bounds\n";
	}
}

bool BlockingState::setState(const std::vector<std::string>& set_state) {
	if (set_state.size() == state_space_dim){
		bool conflict = false;
		bool names_found = true;
		if (set_state.size() > 1){
			for (int i=0; i<set_state.size()-1; i++) {
				if (blocking_dims[i]) {
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

bool BlockingState::setState(const std::string& set_state_var, unsigned int dim) {
	bool name_found = false;
	if (dim+1 > state_space_dim) {
		std::cout<<"Error: Dimension out of bounds\n";
	} else {
		bool conflict = false;
		if (blocking_dims[dim]) {
			for (int i=0; i<state_space_dim; i++) {
				if (blocking_dims[i] && i!=dim) {
					//std::cout<<"statevar: "<<state_space_named[i][state_space[i]]<<std::endl;
					if (state_space_named[i][state_space[i]] == set_state_var) {
						if (debug) {
							std::cout<<"Warning: Cannot set Blocking State, duplication location: "<<set_state_var<<"\n";
						}
						conflict = true;
						goto blocking;
					}
				}
			}
		}
		for (int i=0; i<num_vars[dim]; i++){
			if (state_space_named[dim][i] == set_state_var) {
				name_found = true;
				state_space[dim] = i;
				break;
			}
		}
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

void BlockingState::generateAllPossibleStates(std::vector<BlockingState>& all_states) {
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

