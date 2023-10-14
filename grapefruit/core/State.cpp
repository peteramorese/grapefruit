#include "State.h"

#include<string>
#include<vector>
#include<iostream>
#include<unordered_map>
#include<bitset>

#include "tools/Logging.h"
#include "core/StateSpace.h"


namespace GF {
namespace DiscreteModel {

	// StateAccessCapture

	const std::string& StateAccessCapture::operator[](const std::string& label) {
		dimension_t dim = m_state->getStateSpace()->getDimension(label);
		m_accessed_bit_field |= (1 << dim);
		return (*m_state)[dim];
	}

	void StateAccessCapture::removeAccess(const std::vector<std::string>& lbls) {
		for (const auto& label : lbls) {
			dimension_t dim = m_state->getStateSpace()->getDimension(label);
			m_accessed_bit_field &= ~(1 << dim);
		}
	}

 	//	State

	State::State(const StateSpace* ss) : m_ss(ss) {
		m_state_index_buffer = new uint32_t[m_ss->rank()];
	}

	State::State(const StateSpace* ss, const std::vector<std::string>& vars) : m_ss(ss) {
		m_state_index_buffer = new uint32_t[m_ss->rank()];
		operator=(vars);
	}

	State::State(const StateSpace* ss, const Containers::SizedArray<uint32_t>& var_indices) : m_ss(ss) {
		m_state_index_buffer = new uint32_t[m_ss->rank()];
		operator=(var_indices);
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
		PRINT_NAMED("State", to_str());
	}

	std::string State::to_str() const {
		const Containers::SizedArray<const std::string*> vars = m_ss->interpret(m_state_index_buffer);
		std::string vars_str = *(vars[0]);
		for (dimension_t i=1; i < m_ss->rank(); ++i) {
			vars_str += ", " + *(vars[i]);
		}
		return vars_str;
	}

	bool State::exclEquals(const State& other, const StateAccessCapture& sac) const {
		for (dimension_t i=0; i<m_ss->rank(); ++i) {
			if (!sac.accessed(i) && m_state_index_buffer[i] != other.m_state_index_buffer[i]) return false;
		}
		return true;
	}

	std::pair<bool, const std::string&> State::argFindGroup(const std::string& var_find, const std::string& group_label) const {
		const auto& group = m_ss->getGroup(group_label);
		for (const auto& lbl : group) {
			dimension_t lbl_dim = m_ss->getDimension(lbl);
			auto[var_ind, found] = m_ss->variableIndex(lbl_dim, var_find);
			if (found && var_ind == m_state_index_buffer[lbl_dim]) return {true, lbl};
		}
		return {false, ""};
	}

	bool State::operator== (const State& other) const {
		for (dimension_t i=0; i<m_ss->rank(); ++i) {
			if (m_state_index_buffer[i] != other.m_state_index_buffer[i]) return false;
		}
		return true;
	}

	bool State::operator== (const std::vector<std::string>& vars) const {
		for (dimension_t dim=0; dim < m_ss->rank(); ++dim) {
			auto[var_ind, found] = m_ss->variableIndex(dim, vars[dim]);
			ASSERT(found, "Variable '" << vars[dim] << "' was not found along dimension " << (uint32_t)dim);
			if (m_state_index_buffer[dim] != var_ind) return false;
		}
		return true;
	}

	void State::operator= (const State& other) {
		m_ss = other.m_ss;
		for (dimension_t dim=0; dim < m_ss->rank(); ++dim) {
			m_state_index_buffer[dim] = other.m_state_index_buffer[dim];
		}
	}

	void State::operator= (const std::vector<std::string>& vars) {
		ASSERT(vars.size() == m_ss->rank(), "Attempting to set state of size: " << vars.size() << " when state space is of rank: " << m_ss->rank());
		for (dimension_t dim=0; dim < m_ss->rank(); ++dim) {
			auto[var_ind, found] = m_ss->variableIndex(dim, vars[dim]);
			ASSERT(found, "Variable '" << vars[dim] << "' was not found along dimension " << (uint32_t)dim);
			m_state_index_buffer[dim] = var_ind;
		}
	}

	void State::operator= (const Containers::SizedArray<uint32_t>& var_indices) {
		ASSERT(var_indices.size() == m_ss->rank(), "Variable index array size does not match state space rank");
		for (dimension_t dim = 0; dim < m_ss->rank(); ++dim) {
			ASSERT(var_indices[dim] < m_ss->m_data.getVariables(dim).size(), "Variable index exceeds the number of variables in dimension " << (uint32_t)dim);
			m_state_index_buffer[dim] = var_indices[dim];
		}
	}

} // namespace DiscreteModel
} // namespace GF