#include "State.h"

#include<string>
#include<vector>
#include<iostream>
#include<unordered_map>

#include "tools/Logging.h"
#include "core/StateSpace.h"


namespace DiscreteModel {

	State::State(const StateSpace* ss) : m_ss(ss) {
		m_state_index_buffer = new uint32_t[m_ss->rank()];
	}

	State::State(const StateSpace* ss, const std::vector<std::string>& vars) : m_ss(ss) {
		m_state_index_buffer = new uint32_t[m_ss->rank()];
		operator=(vars);
	}

	State::~State() {
		delete[] m_state_index_buffer;
	}

	State::State(const State& other) : m_ss(other.m_ss) {
		m_state_index_buffer = new uint32_t[m_ss->rank()];
		for (dimension_t i=0; i < m_ss->rank(); ++i) {
			m_state_index_buffer[i] = other.m_state_index_buffer[i];
		}
	}

	//void State::generateAllPossibleStates(std::vector<State>& all_states) {
	//	SS->generateAllPossibleStates_(all_states);
	//}

	void State::print() const {
		const Containers::SizedArray<const std::string*> vars = m_ss->interpret(m_state_index_buffer);
		std::string vars_str = *(vars[0]);
		for (dimension_t i=1; i < m_ss->rank(); ++i) {
			vars_str += ", " + *(vars[i]);
		}
		PRINT_NAMED("State", vars_str);
	}

	bool State::exclEquals(const State& other, const std::vector<std::string>& excl_labels) const {
		std::vector check(m_ss->rank(), true);
		for (const auto& label : excl_labels) {
			dimension_t dim = m_ss->getDimension(label);
			check[dim] = false;
		}
		for (dimension_t i=0; i<m_ss->rank(); ++i) {
			if (check[i] && m_state_index_buffer[i] != other.m_state_index_buffer[i]) return false;
		}
		return true;
	}

	bool State::operator== (const State& other) const {
		for (dimension_t i=0; i<m_ss->rank(); ++i) {
			if (m_state_index_buffer[i] != other.m_state_index_buffer[i]) return false;
		}
		return true;
	}

	void State::operator= (const std::vector<std::string>& vars) {
		ASSERT(vars.size() == m_ss->rank(), "Attempting to set state of size: " << vars.size() << " when state space is of rank: " << m_ss->rank());
		for (dimension_t i=0; i < m_ss->rank(); ++i) {
			m_state_index_buffer[i] = m_ss->variableIndex(i, vars[i]);
		}
	}

}