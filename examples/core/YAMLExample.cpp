#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>

// Debug
#include <typeinfo>

#include <yaml-cpp/yaml.h>

#include "tools/Logging.h"
#include "core/Automaton.h"

std::string toString(const std::string& str) {return str;};

int main() {

    //std::string filepath = "./spot_automaton_file_dump/dfas/dfa_0.yaml";
    std::string filepath = "./spot_automaton_file_dump/dfas/test.yaml";

    YAML::Emitter hello_there;
    LOG("type: " << typeid(hello_there).name());
    hello_there << YAML::BeginMap;
    //out << YAML::Key << "Groups";
    //out << YAML::Value << YAML::BeginMap;
    //out << YAML::Key << 0;
    //out << YAML::Value << YAML::BeginSeq;
    //out << 0;
    //out << 1;
    //out << 2;
    //out << 3;
    //out << YAML::EndSeq << YAML::EndMap;
    //out << YAML::EndMap;
    //std::ofstream fout(filepath);
    //fout << out.c_str();

    //FormalMethods::DFA dfa(true, &toString);
    //dfa.deserialize(filepath);
    //dfa.print();
	return 0;
}
