#pragma once
#include<vector>
#include<iostream>
#include<unordered_map>
#include<unordered_set>

#include "tools/Containers.h"
//#include "core/State.h"

namespace DiscreteModel {

	typedef uint8_t dimension_t;

	class State;

	class StateSpace {
		protected:
			class StateSpaceData {
				private:
					struct Dimension {
						Dimension() = default;
						Dimension(const std::vector<std::string>& variables_, const std::string& label_) : variables(variables_), label(label_) {}
						std::vector<std::string> variables;
						std::string label = std::string();
					};
				private:
					std::vector<Dimension> m_state_space;
					std::unordered_map<std::string, uint32_t> m_label_to_dim;
				public:
					StateSpaceData(uint32_t rank) : m_state_space(rank) {}

					inline dimension_t rank() const {return m_state_space.size();}
					bool hasVariable(const std::string& variable) const {
						for (const auto& dim : m_state_space) {
							for (const auto& var : dim.variables) {
								if (var == variable) return true;
							}
						}
						return false;
					}
					std::pair<bool, const std::string&> findVariable(const std::string& variable, dimension_t index) const {
						for (const auto& var : m_state_space[index].variables) {
							if (var == variable) return {true, var};
						}
						return {false, ""};
					}
					inline bool hasLabel(const std::string& variable) const {return m_label_to_dim.find(variable) != m_label_to_dim.end();}

					inline dimension_t getDimension(const std::string& label) const {return m_label_to_dim.at(label);}
					inline const std::string& getLabel(uint32_t index) const {return m_state_space[index].label;}
					inline const std::vector<std::string>& getVariables(dimension_t dim) const {return m_state_space[dim].variables;}
					inline const std::vector<std::string>& getVariables(const std::string& label) const {return m_state_space[m_label_to_dim.at(label)].variables;}

					void setDimension(dimension_t dim, const std::string& label, const std::vector<std::string>& variables) {
						m_state_space[dim].label = label;
						m_state_space[dim].variables = variables;
						m_label_to_dim[label] = dim;
					}

					inline void clear() {m_state_space.clear(); m_label_to_dim.clear();}
					inline void reset(dimension_t rank) {clear(); m_state_space.resize(rank);}
			};

			struct LabelBundle {
				LabelBundle(const std::vector<std::string>& vars_) {
					for (const auto& var : vars_) vars.insert(var);
				}
				std::unordered_set<std::string> vars;
				bool inBundle(const std::string& label) const {return vars.contains(label);};
			};

		protected:
			StateSpaceData m_data;

			std::unordered_map<std::string, LabelBundle> m_domains;
			std::unordered_map<std::string, LabelBundle> m_groups;


		protected:
		 	// Backdoor methods for State
			const Containers::SizedArray<const std::string*> interpret(const uint32_t var_indices[]) const;
			inline const std::string& interpretIndex(dimension_t dim, uint32_t var_index) const {return m_data.getVariables(dim)[var_index];}
			inline dimension_t getDimension(const std::string& label) const {return m_data.getDimension(label);}
			uint32_t variableIndex(dimension_t index, const std::string& variable) const;


			// Read from file
			bool deserialize(const std::string& filepath);

		private:
			friend class State;
			friend class VariableReference;

		public:
			StateSpace(dimension_t rank) : m_data(rank) {}
			StateSpace(const std::string& filepath) : m_data(0) {deserialize(filepath);}

			inline dimension_t rank() const {return m_data.rank();}

			inline void setDimension(uint32_t dim, const std::string& label, const std::vector<std::string>& vars) {m_data.setDimension(dim, label, vars);}
			inline const std::vector<std::string>& getVariables(uint32_t dim) const {return m_data.getVariables(dim);}

			// Add a domain that names and bundles a set of variables
			void addDomain(const std::string& domain_name, const std::vector<std::string>& vars);
			inline const std::unordered_set<std::string>& getDomain(const std::string& domain_label) const {return m_domains.at(domain_label).vars;}
			inline bool inDomain(const std::string& domain_name, const std::string& var) const {return m_domains.at(domain_name).inBundle(var);}

			// Add a group that names and bundles a set of dimension labels
			void addGroup(const std::string& group_name, const std::vector<std::string>& labels);
			inline const std::unordered_set<std::string>& getGroup(const std::string& group_label) const {return m_groups.at(group_label).vars;}
			inline bool inGroup(const std::string& group_name, const std::string& label) const {return m_groups.at(group_name).inBundle(label);}

			// Find an instance of 'var_find' among dimensions that are within group 'group_label'
			std::pair<bool, const std::string&> argFindGroup(const std::string& var_find, const std::string& group_label) const;

			void serialize(const std::string& filepath) const;

			void print() const;
	};
}