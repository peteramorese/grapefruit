#include "Automaton.h"


namespace TP {
namespace FormalMethods {

std::vector<DFAptr> createDFAsFromFile(Deserializer& dszr) {
    YAML::Node& data = dszr.get();

    std::vector<DFAptr> dfas;
    std::vector<std::string> formulae = data["DFAs"].as<std::vector<std::string>>();
    for (const auto& formula : formulae) {
        DFAptr dfa = std::make_shared<DFA>();
        dfa->generateFromFormula(formula);
        dfas.push_back(std::move(dfa));
    }
    return dfas;
}


}
}
