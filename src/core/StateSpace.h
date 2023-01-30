#pragma once
#include<vector>
#include<iostream>
#include<unordered_map>
//#include "state.h"

namespace DiscreteModel {
	class StateSpace {
		protected:
			class StateSpaceData {
				private:
					struct Dimension {
						Dimension() = default;
						Dimension(const std::vector<std::string>& values_, const std::string& label_) : values(values_), label(label_) {}
						std::vector<std::string> values;
						std::string label = std::string();
					};
				private:
					std::vector<Dimension> m_state_space;
					std::unordered_map<std::string, uint32_t> m_label_to_index;
				public:
					StateSpaceData(uint32_t rank) : m_state_space(rank) {}

					uint8_t rank() const {return m_state_space.size();}
					bool hasValue(const std::string& value) const {
						for (const auto& dim : m_state_space) {
							for (const auto& val : dim.values) {
								if (val == value) return true;
							}
						}
						return false;
					}
					std::pair<bool, const std::string&> findValue(const std::string& value, uint32_t index) const {
						for (const auto& val : m_state_space[index].values) {
							if (val == value) return {true, val};
						}
						return {false, ""};
					}
					bool hasLabel(const std::string& value) const {return m_label_to_index.find(value) != m_label_to_index.end();}

					uint32_t getIndex(const std::string& label) const {return m_label_to_index.at(label);}
					const std::string& getLabel(uint32_t index) const {return m_state_space[index].label;}
					const std::vector<std::string>& getValues(uint32_t index) const {return m_state_space[index].values;}

					void setDimension(uint32_t dim, const std::string& label, const std::vector<std::string>& values) {
						m_state_space[dim].label = label;
						m_state_space[dim].values = values;
						m_label_to_index[label] = dim;
					}
			};

			struct LabelBundle {
				LabelBundle(const std::vector<std::string>& vars_) : vars(vars_) {}
				std::vector<std::string> vars;
			};

		protected:
			StateSpaceData m_data;

			std::unordered_map<std::string, LabelBundle> m_domains;
			std::unordered_map<std::string, LabelBundle> m_groups;

			friend class State;
		public:
			StateSpace(uint8_t rank) : m_data(rank) {}

			uint8_t rank() const {return m_data.rank();}

			void setDimension(uint32_t dim, const std::string& label, const std::vector<std::string>& vars);
			const std::vector<std::string>& getValues(uint32_t dim) const {return m_data.getValues(dim);}

			// Add a domain that names and bundles a set of variables
			void addDomain(const std::string& domain_name, const std::vector<std::string>& vars);

			// Add a group that names and bundles a set of dimension labels
			void addGroup(const std::string& group_name, const std::vector<std::string>& labels);

			const std::vector<std::string>& getGroup(const std::string& group_label) const {return m_groups.at(group_label).vars;}

			// Find an instance of 'var_find' among dimensions that are within group 'group_label'
			std::pair<bool, const std::string&> argFindGroup(const std::string& var_find, const std::string& group_label) const;

			void serialize(const std::string& filepath) const;
			void deserialize(const std::string& filepath);
	};
}