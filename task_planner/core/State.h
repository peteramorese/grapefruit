#pragma once
#include<vector>
#include<iostream>
#include<unordered_map>

#include "tools/Logging.h"
#include "tools/Containers.h"
#include "core/StateSpace.h"


namespace TP {
namespace DiscreteModel {

	//class StateSpace;

	class VariableReference {
		public:
		 	VariableReference(const StateSpace* ss, dimension_t dim, uint32_t* var_ind) : m_ss(ss), m_dim(dim), m_var_ind(var_ind) {}
			inline operator const std::string&() const {return m_ss->interpretIndex(m_dim, *m_var_ind);}
			inline void operator= (const std::string& set_var) {
				uint32_t var_ind_set = 0;
				for (const auto& var : m_ss->getVariables(m_dim)) {
					if (var == set_var) {
						*(m_var_ind) = var_ind_set;
						return;
					}
					var_ind_set++;
				}
				ASSERT(false, "Set variable ('" << set_var << "') was not recognized");
			}
		private:
		 	dimension_t m_dim;
			uint32_t* m_var_ind;
			const StateSpace* m_ss;
	};

#ifdef TP_MAX_RANK_64
	class State;

	class StateAccessCapture {
		public:
			StateAccessCapture(const State* state) : m_state(state) {}

		 	const std::string& operator[](const std::string& label);
			inline void operator|=(const StateAccessCapture& other) {m_accessed_bit_field |= other.m_accessed_bit_field;}
			inline void operator&=(const StateAccessCapture& other) {m_accessed_bit_field &= other.m_accessed_bit_field;}
			void removeAccess(const std::vector<std::string>& lbls);

			bool accessed(dimension_t dim) const {
				return m_accessed_bit_field & (1 << dim);
			}
		private:
			const State* m_state;
			int64_t m_accessed_bit_field = 0;
	};
#else

	// TODO (non bit-field implementation)

#endif


	class State {

		public:
		 	State() = delete;
			State(const StateSpace* ss);
			State(const StateSpace* ss, const std::vector<std::string>& vars);
			State(const StateSpace* ss, const Containers::SizedArray<uint32_t>& var_indices);
			State(const State& other);
			~State();

			inline const StateSpace* getStateSpace() const {return m_ss;}
			StateAccessCapture getStateAccessCapture() const {return StateAccessCapture(this);}
			void print() const;
			bool exclEquals(const State& other, const StateAccessCapture& sac) const;

			// Find an instance of 'var_find' among dimensions that are within group 'group_label'
			std::pair<bool, const std::string&> argFindGroup(const std::string& var_find, const std::string& group_label) const;

			bool operator== (const State& other) const;
			bool operator== (const std::vector<std::string>& vars) const;
			void operator= (const State& other);
			void operator= (const std::vector<std::string>& vars);
			void operator= (const Containers::SizedArray<uint32_t>& var_indices);
		 	inline const std::string& operator[](const std::string& label) const {
				dimension_t dim = m_ss->getDimension(label);
				return m_ss->interpretIndex(dim, m_state_index_buffer[dim]);
			}
			inline VariableReference operator[](const std::string& label) {
				dimension_t dim = m_ss->m_data.getDimension(label);
				return VariableReference(m_ss, dim, &m_state_index_buffer[dim]);
			}
			
			// String conversion;
			std::string to_str() const;
			static inline std::string to_str(const State& state) {return state.to_str();}


		protected:
		 	inline const std::string& operator[](dimension_t dim) const {
				return m_ss->interpretIndex(dim, m_state_index_buffer[dim]);
			}

		protected:
			uint32_t* m_state_index_buffer;
			const StateSpace* m_ss;
			
			friend class StateAccessCapture;
			friend class std::hash<State>;
	};


} // namespace DiscreteModel
} // namespace TP

namespace std {
	template <>
	struct hash<TP::DiscreteModel::State> {
		std::size_t operator()(const TP::DiscreteModel::State& state) const {
			std::size_t hash = state.m_state_index_buffer[0];
			for (TP::DiscreteModel::dimension_t dim = 0; dim < state.m_ss->rank(); ++dim) {
				hash = hash ^ (state.m_state_index_buffer[dim] << dim);
			}
			return hash;
		}
	};
}