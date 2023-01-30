#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>

#include <yaml-cpp/yaml.h>

#include "tools/Logging.h"
#include "core/Automaton.h"

std::string toString(const std::string& str) {return str;};

int main() {

    //std::string filepath = "./spot_automaton_file_dump/dfas/dfa_0.yaml";
    std::string filepath = "./spot_automaton_file_dump/dfas/test.yaml";

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
    std::ofstream fout(filepath);
    fout << out.c_str();

    //FormalMethods::DFA dfa(true, &toString);
    //dfa.deserialize(filepath);
    //dfa.print();
	return 0;
}
