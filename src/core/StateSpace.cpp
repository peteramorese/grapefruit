#include "StateSpace.h"

#include <string>
#include <fstream>

#include <yaml-cpp/yaml.h>

#include "tools/Logging.h"
#include "tools/Containers.h"
#include "core/State.h"

// YAML OVERLOADS
YAML::Emitter& operator << (YAML::Emitter& out, const std::unordered_set<std::string>& str_set) {
	out << YAML::Flow;
	out << YAML::BeginSeq;
	for (const auto& str : str_set) out << str;
	out << YAML::EndSeq;
	return out;
}

namespace DiscreteModel {

	void StateSpace::addDomain(const std::string& domain_name, const std::vector<std::string>& vars) {
		for (const auto& variable : vars) {
			ASSERT(m_data.hasVariable(variable), "Input variable ('" << variable << "') was not found in state space");
		}
		m_domains.emplace(std::make_pair(domain_name, LabelBundle(vars)));
	}

	void StateSpace::addGroup(const std::string& group_name, const std::vector<std::string>& labels) {
		for (const auto& label : labels) {
			ASSERT(m_data.hasLabel(label), "Input label ('" << label << "') was not found in state space");
		}
		m_groups.emplace(std::make_pair(group_name, LabelBundle(labels)));
	}

	void StateSpace::generateAllStates(std::vector<State>& all_states) const {
		dimension_t incrementing_dim = 0;
		uint32_t var_index = 0;

		Containers::SizedArray<uint32_t> increment_indices(rank());
		for (dimension_t dim = 0; dim < increment_indices.size(); ++dim) increment_indices[dim] = 0;

		while (incrementing_dim < rank() || var_index < m_data.getVariables(rank() - 1).size()) {
			increment_indices[incrementing_dim] = var_index;
			++var_index;
			if (var_index == m_data.getVariables(incrementing_dim).size()) {
				var_index = 0;
				++incrementing_dim;
			}
		}
		
	}

	void StateSpace::serialize(const std::string& filepath) const {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Rank" << YAML::Value << (uint32_t)rank();

		bool begin = true;
		for (const auto&[name, bundle] : m_domains) {
			if (begin) {
				out << YAML::Key << "Domains" << YAML::Value << YAML::BeginMap;
				begin = false;
			}
			out << YAML::Key << name;
			out << YAML::Value << bundle.vars;
		}
		if (!begin) {
			out << YAML::EndMap;
			begin = true;
		}

		for (const auto&[name, bundle] : m_groups) {
			if (begin) {
				out << YAML::Key << "Groups";
				out << YAML::Value << YAML::BeginMap;
				begin = false;
			}
			out << YAML::Key << name;
			out << YAML::Value << bundle.vars;
		}
		if (!begin) {
			out << YAML::EndMap;
			begin = true;
		}

		for (dimension_t i=0; i < m_data.rank(); ++i) {
			if (begin) {
				out << YAML::Key << "State Space" << YAML::Value << YAML::BeginMap;
				begin = false;
			}
			out << YAML::Key << m_data.getLabel(i);
			out << YAML::Value << m_data.getVariables(i);
		}
		out << YAML::EndMap;

		out << YAML::EndMap;
		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	bool StateSpace::deserialize(const std::string& filepath) {
		YAML::Node data;
		try {
			data = YAML::LoadFile(filepath);

			if (data["Rank"].as<uint32_t>() == 0 || !data["State Space"]) {
				WARN("Attempted to deserialize empty state space (" << filepath <<")");
				return false;
			}

			m_data.reset(data["Rank"].as<uint32_t>());

			typedef std::map<std::string, std::vector<std::string>> StrToStrArr;

			StrToStrArr state_space = data["State Space"].as<StrToStrArr>();
			dimension_t dim = 0;
			for (auto&[name, label_bundle] : state_space) {
				m_data.setDimension(dim++, name, label_bundle);
			}
			
			m_domains.clear();
			if (data["Domains"]) {
				StrToStrArr domain_map = data["Domains"].as<StrToStrArr>();
				for (auto&[name, label_bundle] : domain_map) {
					addDomain(name, label_bundle);
				}
			}
			
			m_groups.clear();
			if (data["Groups"]) {
				StrToStrArr group_map = data["Groups"].as<StrToStrArr>();
				for (auto&[name, label_bundle] : group_map) {
					addGroup(name, label_bundle);
				}

			}


		} catch (YAML::ParserException e) {
			ERROR("Failed to load file" << filepath << " ("<< e.what() <<")");
			return false;
		}
		return true;
	}

	void StateSpace::print() const {
		LOG("Printing StateSpace");
		PRINT_NAMED("Rank", (uint32_t)rank());
		bool begin = true;
		for (const auto&[name, bundle] : m_domains) {
			if (begin) {
				PRINT_NAMED("Domains", "");
				begin = false;
			}
			PRINT("   " << name << ":");
			for (const auto& var : bundle.vars) {
				PRINT("     -" << var);
			}
		}
		begin = true;

		for (const auto&[name, bundle] : m_groups) {
			if (begin) {
				PRINT_NAMED("Domains", "");
				begin = false;
			}
			PRINT("   " << name << ":");
			for (const auto& var : bundle.vars) {
				PRINT("     -" << var);
			}
		}
		begin = true;

		for (dimension_t i=0; i < m_data.rank(); ++i) {
			if (begin) {
				PRINT_NAMED("State Space", "");
				begin = false;
			}
			PRINT_NAMED("   (Dim " << (uint32_t)i << ")", m_data.getLabel(i));
			for (const auto& var : m_data.getVariables(i)) {
				PRINT("     -" << var);
			}
		}

	}

	const Containers::SizedArray<const std::string*> StateSpace::interpret(const uint32_t var_indices[]) const {
		Containers::SizedArray<const std::string*> ret_vars(rank());
		for (dimension_t i=0; i < rank(); ++i) {
			ret_vars[i] = &m_data.getVariables(i)[var_indices[i]];
		}
		return ret_vars;
	}

	uint32_t StateSpace::variableIndex(dimension_t index, const std::string& variable) const {
		uint32_t i=0;
		for (const auto& var : m_data.getVariables(index)) {
			if (var == variable) return i;
			i++;
		}
		ASSERT(false, "Variable '" << variable << "' was not found along dimension: " << (uint32_t)index);
	}
}

namespace YAML {
    template <>
    struct convert<std::map<std::string, std::vector<std::string>>> {
        //static Node encode(const std::map<std::string, std::vector<std::string>>& str_to_str_arr) {
        //    Node node;
        //    for (auto state : state_set) node.push_back(state);
        //    return node;
        //}
        static bool decode(const Node& node, std::map<std::string, std::vector<std::string>>& str_to_str_arr) {
            if (!node.IsMap()) return false;
			str_to_str_arr.clear();

            for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
				const YAML::Node& key = it->first;
				const YAML::Node& variable = it->second;
				str_to_str_arr.emplace(std::make_pair(
					key.as<std::string>(),
					variable.as<std::vector<std::string>>()));
			}
            return true;
        }
    };
}


//namespace YAML {
//    template <>
//    struct convert<std::unordered_set<std::string>> {
//        static Node encode(const std::unordered_set<std::string>& str_set) {
//            Node node;
//            for (auto str : str_set) node.push_back(str);
//            return node;
//        }
//        static bool decode(const Node& node, std::unordered_set<std::string>& str_set) {
//            if (!node.IsSequence()) return false;
//			str_set.clear();
//
//			std::vector<std::string> items = node.as<std::vector<std::string>>();
//			for (const auto& item : items) str_set.insert(item);
//            return true;
//        }
//    };
//}




//void StateSpace::generateAllPossibleStates_(std::vector<State>& all_states) {
//	int counter = 1;
//	// Count the number of possible states using a permutation. Omit the last element in each
//	// state_space_named array because it represents UNDEF
//	std::vector<int> column_wrapper(state_space_dim);
//	std::vector<int> digits(state_space_dim);
//	for (int i=0; i<state_space_dim; i++){
//		int inds = state_space_named[i].size() - 1;
//		counter *= inds;
//		column_wrapper[i] = inds;
//		digits[i] = 0;
//	}
//	all_states.resize(counter, State(this));
//	int a = 0;
//	int b = 0;
//	for (int i=0; i<counter; i++) {
//		for (int ii=0; ii<state_space_dim; ii++){
//			all_states[i].setState(state_space_named[ii][digits[ii]],ii);
//		}
//		digits[0]++;
//		for (int ii=0; ii<state_space_dim; ii++){
//			if (digits[ii] > column_wrapper[ii]-1) {
//				digits[ii] = 0;
//				if (ii != state_space_dim-1) {
//					digits[ii+1]++;
//				}
//			}
//		}
//	}
//}
