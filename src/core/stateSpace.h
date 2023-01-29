#pragma once
#include<vector>
#include<iostream>
#include<unordered_map>
//#include "state.h"

class StateSpace {
	protected:
	 	class StateSpaceData {
			private:
				struct Dimension {
					std::vector<std::string> values;
					std::string label;
				};
			private:
				std::vector<Dimension> m_state_space;
				std::unordered_map<std::string, uint32_t> m_label_to_index;
			public:
				StateSpaceData(uint32_t rank) : m_state_space(rank) {}

				std::size_t rank() const {return m_state_space.size();}
				uint32_t getIndex(const std::string& label) const {return label_to_index.at(label);}
				const std::string& getLabel(uint32_t index) const {return return m_state_space[index].label;}
		};

		struct Values {
			 values;
		};

		struct Domain {
			std::string label;
			std::vector<std::string> vars;
		};

	protected:
		StateSpaceData m_data;

		std::vector<domain> domains;
		std::vector<domain> groups;
		friend class State;

	public:
		StateSpace(uint32_t rank) : m_data(rank) {}

		uint32_t rank() const {return m_state_space.size();}

		void setDimension(uint32_t dim, const std::string& label, const std::vector<std::string>& values) 
		const std::vector<std::string>& getValues(uint32_t dim) {return m_state_space[dim].values;}

		void setStateDimensionLabel(uint32_t dim, const std::string& dimension_label);
		void setDomain(const std::string& domain_label, const std::vector<std::string>& vars);
		void setDomain(const std::string& domain_label, const std::vector<std::string>& vars, uint32_t index);
		bool getDomains_(const std::string& var, std::vector<std::string>& in_domains) const;
		void setLabelGroup(const std::string& group_label, const std::vector<std::string>& dimension_labels);
		void setLabelGroup(const std::string& group_label, const std::vector<std::string>& dimension_labels, uint32_t index);
		void getGroupDimLabels_(const std::string& group_label, std::vector<std::string>& group_dim_labels) const;
		bool argFindGroup_(const std::string& var_find, const std::string& group_label, std::string& arg_dimension_label, const std::vector<int>& state_space); 
		void setState_(const std::vector<std::string>& set_state, std::vector<int>& state_space);
		void setState_(const std::string& set_state_var, uint32_t dim, std::vector<int>& state_space);
		void getState_(std::vector<std::string>& ret_state, const std::vector<int>& state_space) const;
		std::string getVar_(const std::string& dimension_label, const std::vector<int>& state_space);
		bool isDefined_(const std::vector<int>& state_space) const;
		void print_(const std::vector<int>& state_space) const;
		bool exclEquals_(const State* state_ptr_, const std::vector<std::string>& excl_dimension_labels, const std::vector<int>& state_space);
		void writeToFile(const std::string& filename) const;
		// string methods that don't exist in CXX 20:
		static bool starts_with(const std::string& str, const std::string& prefix); // in case CXX 20 cant be used
		static std::string::iterator str_find(std::string* str, char stop_char);

		static std::shared_ptr<StateSpace> readFromFile(const std::string& filename);
		/*
		bool operator== (const State& state_) const;
		bool operator== (const State* state_ptr_) const;
		void operator= (const State& state_eq);
		void operator= (const State* state_eq_ptr);
		*/
};