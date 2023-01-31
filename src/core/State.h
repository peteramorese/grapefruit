#pragma once
#include<vector>
#include<iostream>
#include<unordered_map>

//#include "core/StateSpace.h"

namespace DiscreteModel {

	class StateSpace;

	class State {

		public:
			State(const StateSpace* ss);
			State(const State& other);
			~State();
			const StateSpace* getStateSpace() const {return m_ss;}
			//void generateAllPossibleStates(std::vector<State>& all_states) ;
			//int getVarOptionsCount(unsigned int dim);
			//bool getDomains(const std::string& var, std::vector<std::string>& in_domains) const;
			//void getGroupDimLabels(const std::string& group_label, std::vector<std::string>& group_dim_labels) const;
			//bool argFindGroup(const std::string& var_find, const std::string& group_label, std::string& arg_dimension_label) const; 
			//void setState(const std::vector<std::string>& set_state);
			//void setState(const std::string& set_state_var, unsigned int dim);
			//void getState(std::vector<std::string>& ret_state) const;
			const std::string& getVar(const std::string& label) const;
			bool isDefined() const;
			void print() const;
			bool exclEquals(const State* state_ptr_, const std::vector<std::string>& excl_dimension_labels) const;
			bool operator== (const State& other) const;
			void operator= (const std::vector<std::string>& vars);
			 
		 	inline uint32_t operator[](uint8_t i) const {return m_state_index_buffer[i];}

		protected:
			uint32_t* m_state_index_buffer;
			const StateSpace* m_ss;
	};
}