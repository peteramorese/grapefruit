#include<string>
#include<vector>
#include<iostream>
#include<unordered_map>
#include<fstream>
#include "stateSpace.h"
#include "state.h"


/* StateSpace DEFINITION */
/*
std::vector<std::vector<std::string>> StateSpace::state_space_named;
std::unordered_map<std::string, unsigned int> StateSpace::index_labels;
std::vector<int> StateSpace::num_vars;
unsigned int StateSpace::state_space_dim;
std::vector<StateSpace::domain> StateSpace::domains;
std::vector<StateSpace::domain> StateSpace::groups;
bool StateSpace::is_dimensions_defined = false;
*/
const std::string StateSpace::UNDEF = "UNDEF";

StateSpace::StateSpace() {
	state_space_dim = 0;
}

void StateSpace::resizeAll_(unsigned int size) {
	state_space_named.resize(size);
	num_vars.resize(size);
}

void StateSpace::resizeAll_() {
	state_space_named.resize(state_space_dim);
	num_vars.resize(state_space_dim);
}

/*
void StateSpace::initNewSS() {
	state_space.resize(state_space_dim);
	for (int i=0; i<state_space_dim; ++i) {
		state_space[i] = num_vars[i]-1;
	}
}
*/


void StateSpace::setStateDimension(const std::vector<std::string>& var_labels, unsigned int dim) {
	is_dimensions_defined = true;
	if (dim+1 > state_space_dim) {
		state_space_dim = dim + 1;
		resizeAll_();
	}	
	// The last label for each dimension is automatically set to be undefined
	std::vector<std::string> set_labels = var_labels;
	set_labels.push_back(UNDEF);
	state_space_named[dim] = set_labels;
	num_vars[dim] = set_labels.size();
}

void StateSpace::generateAllPossibleStates_(std::vector<State>& all_states) {
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
	all_states.resize(counter, State(this));
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
				if (ii != state_space_dim-1) {
					digits[ii+1]++;
				}
			}
		}
	}
}

unsigned int StateSpace::getDim() const {
	return state_space_dim;
}

int StateSpace::getVarOptionsCount_(unsigned int dim) {
	if (dim < state_space_dim){
		return state_space_named[dim].size();
	} else {
		std::cout<<"Error: Index out of bounds\n";
		return -1;
	}
}

void StateSpace::setStateDimensionLabel(unsigned int dim, const std::string& dimension_label){
	if (dim < state_space_dim) {
		index_labels[dimension_label] = dim;
		if (index_labels_rev.size() == 0 || index_labels_rev.size()-1 < dim) index_labels_rev.resize(dim+1);
		index_labels_rev[dim] = dimension_label;
	} else {
		std::cout<<"Error: Index out of bounds\n";
	}
}

void StateSpace::setDomain(const std::string& domain_label, const std::vector<std::string>& vars){
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

void StateSpace::setDomain(const std::string& domain_label, const std::vector<std::string>& vars, unsigned int index){
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

bool StateSpace::getDomains_(const std::string& var, std::vector<std::string>& in_domains) const {
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

void StateSpace::setLabelGroup(const std::string& group_label, const std::vector<std::string>& dimension_labels) {
	domain add_group;
	add_group.label = group_label;
	add_group.vars = dimension_labels;
	groups.push_back(add_group);

}

void StateSpace::setLabelGroup(const std::string& group_label, const std::vector<std::string>& dimension_labels, unsigned int index) {
	if (index+1 > groups.size()) {
		groups.resize(index+1);
	}
	domain add_group;
	add_group.label = group_label;
	add_group.vars = dimension_labels;
	groups[index] = add_group;
}

void StateSpace::getGroupDimLabels_(const std::string& group_label, std::vector<std::string>& group_dim_labels) const {
	for (int i=0; i<groups.size(); ++i) {
		if (groups[i].label == group_label) {
			group_dim_labels = groups[i].vars;
			break;
		}
	}
}

bool StateSpace::argFindGroup_(const std::string& var_find, const std::string& group_label, std::string& arg_dimension_label, const std::vector<int>& state_space) {
	bool is_found = false;
	arg_dimension_label = "NOT FOUND";
	for (int i=0; i<groups.size(); i++) {
		if (groups[i].label == group_label) {
			for (int ii=0; ii<groups[i].vars.size(); ii++){
				std::string dim_label = groups[i].vars[ii];
				int ind;
				ind = index_labels.at(dim_label);
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

void StateSpace::setState_(const std::vector<std::string>& set_state, std::vector<int>& state_space) {
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
			for(auto& state : set_state) {
				std::cout<<state<<std::endl;
			}
		}
	} else {
		std::cout<<"Error: Set state must have same dimension as state space\n";
	}
}

void StateSpace::setState_(const std::string& set_state_var, unsigned int dim, std::vector<int>& state_space) {
	bool name_found = false;
	if (dim+1 > state_space_dim) {
		std::cout<<"Error: Dimension out of bounds\n";
	} else {
		//std::cout<<"   DIM = "<<dim<<" num_vars[dim] = "<<num
		for (int i=0; i<num_vars[dim]; i++){
			//std::cout<<"state_space_named: "<<state_space_named[dim][i]<<" set state var: "<<set_state_var<<std::endl;
			if (state_space_named[dim][i] == set_state_var) {
				name_found = true;
				state_space[dim] = i;
			}
		}
	}
	if (!name_found) {
		std::cout<<"Error: Unrecognized label in set state\n";
	}
}

void StateSpace::getState_(std::vector<std::string>& ret_state, const std::vector<int>& state_space) const {
	ret_state.clear();
	ret_state.resize(state_space_dim);
	for (int i=0; i<state_space_dim; i++){
		ret_state[i] = state_space_named[i][state_space[i]];
	}
}

std::string StateSpace::getVar_(const std::string& dimension_label, const std::vector<int>& state_space) {
	unsigned int ind = index_labels.at(dimension_label);
	int named_ind = state_space[ind];
	return state_space_named[ind][named_ind];
}

bool StateSpace::isDefined_(const std::vector<int>& state_space) const {
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

void StateSpace::print_(const std::vector<int>& state_space) const {
	for (int i=0; i<state_space_dim; i++){
		std::cout<<"State variable "<< i << " = " << state_space_named[i][state_space[i]]<< "\n";
	}
}

bool StateSpace::exclEquals_(const State* state_ptr_, const std::vector<std::string>& excl_dimension_labels, const std::vector<int>& state_space) {
	std::vector<bool> check(state_space_dim, true);
	for (int i=0; i<excl_dimension_labels.size(); i++){
		int ind = index_labels.at(excl_dimension_labels[i]);
		check[ind] = false;
	}
	for (int i=0; i<state_space_dim; i++){
		if (check[i]) {
			if (state_space[i] != state_ptr_->state_space[i]) {
				return false;
			}	
		}
	}
	return true;
}

void StateSpace::writeToFile(const std::string& filename) const {
	std::ofstream file;
	file.open(filename);
	file<<"<SS>NDIMS: "<<state_space_named.size()<<"\n";
	for (int i=0; i<state_space_named.size(); ++i) {
		std::cout<<"b4"<<std::endl;
		file<<"<SS>DIM"<<i<<" "<<index_labels_rev[i]<<": ";
		std::cout<<"af"<<std::endl;
		for (int ii=0; ii<state_space_named[i].size(); ++ii) {
			file<<state_space_named[i][ii];
			if (ii != state_space_named[i].size() - 1) {
				file<<", ";
			} else {
				file<<";";
			}
		}
		file<<"\n";
	}
	file<<"<SS>NDOMAINS: "<<domains.size()<<"\n";
	for (int i=0; i<domains.size(); ++i) {
		file<<"<SS>DOM"<<i<<" "<<domains[i].label<<": ";
		for (int ii=0; ii<domains[i].vars.size(); ++ii) {
			file<<domains[i].vars[ii];
			if (ii != domains[i].vars.size() - 1) {
				file<<", ";
			} else {
				file<<";";
			}
		}
		file<<"\n";
	}
	file<<"<SS>NGROUPS: "<<groups.size()<<"\n";
	for (int i=0; i<groups.size(); ++i) {
		file<<"<SS>GRP"<<i<<" "<<groups[i].label<<": ";
		for (int ii=0; ii<groups[i].vars.size(); ++ii) {
			file<<groups[i].vars[ii];
			if (ii != groups[i].vars.size() - 1) {
				file<<", ";
			} else {
				file<<";";
			}
		}
		file<<"\n";
	}
	file.close();
}

bool StateSpace::starts_with(const std::string& str, const std::string& prefix) {
	for (int i=0; i<prefix.size(); ++i) {
		if (str[i] != prefix[i]) {
			return false;
		}
	}
	return true;
}

std::string::iterator StateSpace::str_find(std::string* str, char stop_char) {
	for (auto c=str->begin(); c!=str->end(); c++) {
		std::cout<<"*c: "<<*c<<" stop_char: "<<stop_char<<";\n";
		if (*c == stop_char) {
			return c;
		}
	}
	return str->end();
}

std::shared_ptr<StateSpace> StateSpace::readFromFile(const std::string& filename) {
	std::ifstream model_file(filename);
	std::shared_ptr<StateSpace> SS = std::make_shared<StateSpace>();
	if (model_file.is_open()) {
		std::string line;
		while (std::getline(model_file, line)) {
			if (starts_with(line,"<SS>")) {
				line.erase(0,4);
				int type = -1;
				if (starts_with(line, "NDIMS: ")) {
					type = 0;
				} else if (starts_with(line, "NDOMAINS: ")) {
					type = 1;
				} else if (starts_with(line, "NGROUPS: ")) {
					type = 1;
				} else {
					std::cout<<"Unrecognized state space line"<<std::endl;
					return nullptr;
				}
				line.erase(line.begin(), std::find(line.begin(), line.end(), ' '));
				int n_dims = std::stoi(line);
				for (int i=0; i<n_dims; ++i) {
					if (!std::getline(model_file, line)) std::cout<<"Error expected more dimensions";
					std::vector<std::string> vars;
					//line.erase(line.begin(), std::find(line.begin(), line.end(), ' ') + 1);
					line.erase(line.begin(), str_find(&line, ' ') + 1);
					//auto colon_itr = std::find(line.begin(), line.end(), ':');
					auto colon_itr = str_find(&line, ':');
					std::string lbl(line.begin(), colon_itr);
					line.erase(line.begin(), colon_itr + 2);
					std::cout<<"FOUND LABEL: "<<lbl<<std::endl;
					std::string buffer;
					for (int j=0; j<line.size(); ++j) {
						if (line[j] == ',' || line[j] == ';') {
							vars.push_back(buffer);
							std::cout<<"ADDING VAR: "<<buffer<<std::endl;
							buffer.clear();
						} else if (line[j] != ' ') {
							buffer.push_back(line[j]);
						}
					}
					if (type == 0) {
						std::cout<<"Setting state dimension "<<i<<" vars: "<<std::endl;
						for (auto var : vars) std::cout<<"	-var: "<<var<<std::endl;
						SS->setStateDimension(vars, i);
						std::cout<<"Setting state dimension label "<<i<<" lbl: "<<lbl<<std::endl;
						SS->setStateDimensionLabel(i, lbl);
					} else if (type == 1) {
						std::cout<<"SETTING GROUP: "<<lbl<<std::endl;
						SS->setLabelGroup(lbl, vars);
					} else if (type == 2) {
						std::cout<<"SETTING DOMAIN: "<<lbl<<std::endl;
						SS->setDomain(lbl, vars);
					}
				}
			} else {
				break;
			}
		}
	}
	model_file.close();
	return SS;
}

/*
bool StateSpace::operator== (const State& state_) const {
	return (state_space == state_.state_space);
}

bool StateSpace::operator== (const State* state_ptr_) const {
	return (state_space == state_ptr_->state_space);
}

void StateSpace::operator= (const State& state_eq) {
	state_space = state_eq.state_space;
}

void StateSpace::operator= (const State* state_eq_ptr) {
	state_space = state_eq_ptr->state_space;
}
*/

/* BlockingState DEFINTION */

//std::vector<bool> BlockingStateSpace::blocking_dims;
//bool BlockingStateSpace::debug = false;

void BlockingStateSpace::toggleDebug_(bool debug_) {
	debug = debug_;
}

void BlockingStateSpace::setBlockingDim_(const std::vector<bool>& blocking_dims_) {
	blocking_dims = blocking_dims_;
}

void BlockingStateSpace::setBlockingDim_(bool blocking, unsigned int dim) {
	if (dim < state_space_dim){
		blocking_dims[dim] = blocking;
	} else {
		std::cout<<"Error: Dimension index out of bounds\n";
	}
}

void BlockingStateSpace::generateAllPossibleStates_(std::vector<BlockingState>& all_states) {
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
	all_states.resize(counter, BlockingState(this));
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
	all_states.resize(j, BlockingState(this));
	std::cout<<"Info: Generated "<<j<<" blocking states out of "<<counter<<" possible states\n";
}

bool BlockingStateSpace::setState_(const std::vector<std::string>& set_state, std::vector<int>& state_space) {
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
		return false;
	}
	return false;
}

bool BlockingStateSpace::setState_(const std::string& set_state_var, unsigned int dim, std::vector<int>& state_space) {
	bool name_found = false;
	if (dim+1 > state_space_dim) {
		std::cout<<"Error: Dimension out of bounds\n";
	} else {
		bool conflict = false;
		if (blocking_dims[dim]) {
			for (int i=0; i<state_space_dim; i++) {
				if (blocking_dims[i] && i!=dim) {
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
	return false;
}

