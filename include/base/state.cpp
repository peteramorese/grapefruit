#include<string>
#include<vector>
#include<iostream>
#include<unordered_map>
#include "state.h"
#include "stateSpace.h"


const std::string State::UNDEF = "UNDEF";


State::State(StateSpace* SS_) : SS(SS_) {
	state_space.resize(SS->getDim());
}

void State::resizeAll(unsigned int size) {
	SS->resizeAll_(size);
}

void State::resizeAll() {
	SS->resizeAll_();
}

void State::generateAllPossibleStates(std::vector<State>& all_states) {
	SS->generateAllPossibleStates_(all_states);
}

int State::getVarOptionsCount(unsigned int dim) {
	int count = SS->getVarOptionsCount_(dim);
	return count;
}

bool State::getDomains(const std::string& var, std::vector<std::string>& in_domains) const {
	return SS->getDomains_(var, in_domains);
}

void State::getGroupDimLabels(const std::string& group_label, std::vector<std::string>& group_dim_labels) const {
	SS->getGroupDimLabels_(group_label, group_dim_labels);
}

bool State::argFindGroup(const std::string& var_find, const std::string& group_label, std::string& arg_dimension_label) const {
	return SS->argFindGroup_(var_find, group_label, arg_dimension_label, state_space);
}

void State::setState(const std::vector<std::string>& set_state) {
	if (state_space.size() == SS->getDim()) {
		SS->setState_(set_state, state_space);
	} else {
		std::cout<<"Error: Dimension mismatch when trying to set state\n";
	}
}

void State::setState(const std::string& set_state_var, unsigned int dim) {
	if (state_space.size() == SS->getDim()) {
		SS->setState_(set_state_var, dim, state_space);
	} else {
		std::cout<<"Error: Dimension mismatch when trying to set state\n";
	}
}

void State::getState(std::vector<std::string>& ret_state) const {
	SS->getState_(ret_state, state_space);
}

std::string State::getVar(const std::string& dimension_label) const {
	return SS->getVar_(dimension_label, state_space);
}

bool State::isDefined() const {
	return SS->isDefined_(state_space);
}

void State::print() const {
	SS->print_(state_space);
}

bool State::exclEquals(const State* state_ptr_, const std::vector<std::string>& excl_dimension_labels) const {
	return SS->exclEquals_(state_ptr_, excl_dimension_labels, state_space);
}

bool State::operator== (const State& state_) const {
	return (state_space == state_.state_space);
}

bool State::operator== (const State* state_ptr_) const {
	return (state_space == state_ptr_->state_space);
}

void State::operator= (const State& state_eq) {
	state_space = state_eq.state_space;
	SS = state_eq.SS;
}

void State::operator= (const State* state_eq_ptr) {
	state_space = state_eq_ptr->state_space;
	SS = state_eq_ptr->SS;
}

/* BlockingState DEFINTION */

BlockingState::BlockingState(BlockingStateSpace* BSS_) : State(BSS_), BSS(BSS_) {
	SS = BSS_;
}

void BlockingState::toggleDebug(bool debug_) {
	BSS->toggleDebug_(debug_);
}

void BlockingState::setBlockingDim(const std::vector<bool>& blocking_dims_) {
	BSS->setBlockingDim_(blocking_dims_);
}

void BlockingState::setBlockingDim(bool blocking, unsigned int dim) {
	BSS->setBlockingDim_(blocking, dim);
}

void BlockingState::generateAllPossibleStates(std::vector<BlockingState>& all_states) {
	BSS->generateAllPossibleStates_(all_states);
}

bool BlockingState::setState(const std::vector<std::string>& set_state) {
	return BSS->setState_(set_state, state_space);
}

bool BlockingState::setState(const std::string& set_state_var, unsigned int dim) {
	return BSS->setState_(set_state_var, dim, state_space);
}

