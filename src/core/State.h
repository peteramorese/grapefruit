#pragma once
#include<vector>
#include<iostream>
#include<unordered_map>

#include "tools/Logging.h"
#include "core/StateSpace.h"


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

	class State {

		public:
			State(const StateSpace* ss);
			State(const StateSpace* ss, const std::vector<std::string>& vars);
			State(const State& other);
			~State();

			const StateSpace* getStateSpace() const {return m_ss;}
			//void generateAllPossibleStates(std::vector<State>& all_states) ;
			void print() const;
			bool exclEquals(const State& other, const std::vector<std::string>& excl_labels) const;

			bool operator== (const State& other) const;
			void operator= (const std::vector<std::string>& vars);
		 	inline const std::string& operator[](const std::string& label) const {
				dimension_t dim = m_ss->getDimension(label);
				return m_ss->interpretIndex(dim, m_state_index_buffer[dim]);
			}
			inline VariableReference operator[](const std::string& label) {
				dimension_t dim = m_ss->m_data.getDimension(label);
				return VariableReference(m_ss, dim, &m_state_index_buffer[dim]);
			}

		protected:
			uint32_t* m_state_index_buffer;
			const StateSpace* m_ss;
	};
}