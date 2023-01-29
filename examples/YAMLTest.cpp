#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

#include <yaml-cpp/yaml.h>

#include "tools/Logging.h"
#include "core/Automaton.h"

std::string toString(const std::string& str) {return str;};

int main() {
    //YAML::Emitter out;
    //out << YAML::BeginMap;
    //out << YAML::Key << "key" << YAML::Value <<"value";
    //out << YAML::EndMap;
    //std::ofstream fout("test.yaml");
    //fout << out.c_str();
    std::string filepath = "./spot_automaton_file_dump/dfas/dfa_0.yaml";
    FormalMethods::DFA dfa(true, &toString);
    dfa.readFromFile(filepath);
    dfa.print();
	return 0;
}
