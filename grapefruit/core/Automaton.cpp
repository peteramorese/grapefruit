#include "Automaton.h"


namespace GF {
namespace FormalMethods {

std::vector<DFAptr> createDFAsFromFile(const Deserializer& dszr) {
    const YAML::Node& data = dszr.get();

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
