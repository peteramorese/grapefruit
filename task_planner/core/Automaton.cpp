#include "Automaton.h"


namespace TP {
namespace FormalMethods {

std::vector<DFAptr> createDFAsFromFile(const std::string& filepath) {
    YAML::Node data;
    std::vector<DFAptr> dfas;
    try {
        data = YAML::LoadFile(filepath);
        std::vector<std::string> formulae = data["DFAs"].as<std::vector<std::string>>();
        for (const auto& formula : formulae) {
            DFAptr dfa = std::make_shared<DFA>();
            dfa->generateFromFormula(formula);
            dfas.push_back(std::move(dfa));
        }
    } catch (YAML::ParserException e) {
        ERROR("Failed to load file" << filepath << " ("<< e.what() <<")");
    }
    return dfas;
}


}
}