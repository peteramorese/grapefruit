#include "StateSpace.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <fstream>

#include <yaml-cpp/yaml.h>

#include "tools/Logging.h"

namespace DiscreteModel {

	void StateSpace::setDimension(uint32_t dim, const std::string& label, const std::vector<std::string>& vars) {
		m_data.setDimension(dim, label, vars);
	}

	void StateSpace::addDomain(const std::string& domain_name, const std::vector<std::string>& vars) {
		for (const auto& value : vars) {
			ASSERT(m_data.hasValue(value), "Input variable: " << value << " was not found in state space");
		}
		//m_domains[domain_name] = LabelBundle(vars);
		m_domains.emplace(std::make_pair(domain_name, LabelBundle(vars)));
	}

	void StateSpace::addGroup(const std::string& group_name, const std::vector<std::string>& labels) {
		for (const auto& label : labels) {
			ASSERT(m_data.hasLabel(label), "Input label: " << label << " was not found in state space");
		}
		m_groups.emplace(std::make_pair(group_name, LabelBundle(labels)));
	}

	std::pair<bool, const std::string&> StateSpace::argFindGroup(const std::string& var_find, const std::string& group_label) const {
		const LabelBundle& group = m_groups.at(group_label);
		for (const auto& var : group.vars) {
			for (uint8_t i=0; i<m_data.rank(); ++i) {
				auto result = m_data.findValue(var_find, i);
				if (result.first) return result;
			}
		}
		return {false, ""};
	}

	void StateSpace::serialize(const std::string& filepath) const {
    	std::string filepath_ = "./spot_automaton_file_dump/dfas/test.yaml";

    	std::map<std::string, std::vector<uint32_t>> map;
    	map["hello"].push_back(0);
    	map["hello"].push_back(1);
    	map["hello"].push_back(2);
    	map["bye"].push_back(22);
    	YAML::Emitter out;
    	out << YAML::BeginMap;
    	out << YAML::Key << "Groups";
    	out << YAML::Value << YAML::BeginMap;
    	out << YAML::Key << 0;
    	out << YAML::Value << YAML::BeginSeq;
    	out << 0;
    	out << 1;
    	out << 2;
    	out << 3;
    	out << YAML::EndSeq << YAML::EndMap;
    	out << YAML::EndMap;
    	std::ofstream fout(filepath_);
    	fout << out.c_str();
		//LOG("b4 test key");
		//out << YAML::Key << "Groups";
		//LOG("af test key");
		//bool begin = true;
		//for (const auto&[name, bundle] : m_domains) {
		//	LOG("not where I should be");
		//	if (begin) {
		//		out << YAML::Key << "Domains" << YAML::Value << YAML::BeginMap;
		//		begin = false;
		//	}
		//	out << YAML::Key << name;
		//	out << YAML::Value << bundle.vars;
		//}
		//if (!begin) {
		//	LOG("not where I should be");
		//	out << YAML::EndMap;
		//	begin = true;
		//}
		//LOG("af domains");

		//for (const auto&[name, bundle] : m_groups) {
		//	if (begin) {
		//		LOG("b4 begin");
		//		out << YAML::Key << "Groups";
		//		LOG("af key");
		//		out << YAML::Value << YAML::BeginMap;
		//		begin = false;
		//	}
		//	out << YAML::Key << name;
		//	LOG("b4 bundle");
		//	out << YAML::Value << bundle.vars;
		//	LOG("af bundle");
		//}
		//if (!begin) {
		//	out << YAML::EndMap;
		//	begin = true;
		//}
		//LOG("af groups");

		//for (uint8_t i=0; i < m_data.rank(); ++i) {
		//	if (begin) {
		//		out << YAML::Key << "State Space" << YAML::Value << YAML::BeginMap;
		//		begin = false;
		//	}
		//	out << YAML::Key << m_data.getLabel(i);
		//	out << YAML::Value << m_data.getValues(i);
		//}
		//out << YAML::EndMap;

		//out << YAML::EndMap;
		//std::ofstream fout(filepath);
		//fout << out.c_str();
	}

	//void StateSpace::deserialize(const std::string& filepath) {
	//	YAML::Node data;
	//	try {
	//		data = YAML::LoadFile(filepath);

	//		m_alphabet = data["Alphabet"].as<Alphabet>();
	//		m_init_states = data["Initial States"].as<StateSet>();
	//		m_accepting_states = data["Accepting States"].as<StateSet>();

	//		std::map<uint32_t, std::vector<uint32_t>> connections = data["Connections"].as<std::map<uint32_t, std::vector<uint32_t>>>();
	//		std::map<uint32_t, std::vector<std::string>> labels = data["Labels"].as<std::map<uint32_t, std::vector<std::string>>>();
	//		for (auto[src, destinations] : connections) {
	//			for (uint32_t i = 0; i <destinations.size(); ++i) {
	//				const std::string& label = labels[src][i];
	//				uint32_t dst = destinations[i];
	//				connect(src, dst, label);
	//			}
	//		}
	//	} catch (YAML::ParserException e) {
	//		ERROR("Failed to load file" << filepath << " ("<< e.what() <<")");
	//	}
	//	return true;
	//}
}




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
