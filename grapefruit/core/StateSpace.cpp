#include "StateSpace.h"

#include <string>
#include <fstream>

#include <yaml-cpp/yaml.h>

#include "tools/Logging.h"
#include "tools/Containers.h"
#include "tools/Algorithms.h"
#include "core/State.h"

// YAML OVERLOADS
YAML::Emitter& operator << (YAML::Emitter& out, const std::unordered_set<std::string>& str_set) {
	out << YAML::Flow;
	out << YAML::BeginSeq;
	for (const auto& str : str_set) out << str;
	out << YAML::EndSeq;
	return out;
}

namespace GF {
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
		// Count the number of possible states using a permutation.
		uint32_t counter = 1;
		Containers::SizedArray<uint32_t> n_options(rank());
		Containers::SizedArray<uint32_t> digits(rank());
		for (dimension_t dim=0; dim<rank(); ++dim) {
			uint32_t num_variables = m_data.getVariables(dim).size();
			counter *= num_variables;
			n_options[dim] = num_variables;
			digits[dim] = 0;
		}
		all_states.resize(counter, State(this));

		uint32_t i = 0;
		auto onPermutation = [&] (const Containers::SizedArray<uint32_t>& option_indices) {
			all_states[i++] = option_indices;
		};

		Algorithms::Combinatorics::permutations(n_options, onPermutation);

	}

	void StateSpace::serialize(Serializer& szr) const {
		YAML::Emitter& out = szr.get();

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

		std::vector<std::string> labels(m_data.rank());
		for (dimension_t i=0; i < m_data.rank(); ++i) {
			labels[i] = m_data.getLabel(i);
			if (begin) {
				out << YAML::Key << "State Space" << YAML::Value << YAML::BeginMap;
				begin = false;
			}
			out << YAML::Key << m_data.getLabel(i);
			out << YAML::Value << m_data.getVariables(i);
		}
		out << YAML::EndMap;

		out << YAML::Key << "Dimension Labels" << YAML::Value << labels;
	}

	void StateSpace::deserialize(const Deserializer& dszr) {
		const YAML::Node& data = dszr.get();
		if (data["Rank"].as<uint32_t>() == 0 || !data["State Space"]) {
			WARN("Attempted to deserialize empty state space");
		}

		m_data.reset(data["Rank"].as<uint32_t>());

		std::vector<std::string> dimension_labels = data["Dimension Labels"].as<std::vector<std::string>>();
		ASSERT(dimension_labels.size() == m_data.rank(), "Number of dimension labels does not match rank");

		typedef std::map<std::string, std::vector<std::string>> StrToStrArr;

		StrToStrArr state_space = data["State Space"].as<StrToStrArr>();
		for (dimension_t dim = 0; dim < dimension_labels.size(); ++dim) {
		 	const std::string& dim_label = dimension_labels[dim];
			m_data.setDimension(dim, dim_label, state_space[dim_label]);
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

	std::pair<uint32_t, bool> StateSpace::variableIndex(dimension_t index, const std::string& variable) const {
		uint32_t i=0;
		for (const auto& var : m_data.getVariables(index)) {
			if (var == variable) return {i, true};
			i++;
		}
		return {0, false};
	}
}
} // namespace GF

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
