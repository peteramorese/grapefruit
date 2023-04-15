#include<iostream>

#include "core/Automaton.h"
#include "theory/PartialSatisfactionAutomaton.h"

using namespace TP;
using namespace TP::FormalMethods;

int main() {
    //PartialSatisfactionDFA psdfa_0;
    //PartialSatisfactionDFA psdfa_1;
    
    //psdfa_0.deserialize("test_dfas/dfa_0.yaml", "test_dfas/sub_map_0.yaml");
    //psdfa_0.print();

    //psdfa_1.deserialize("test_dfas/dfa_1.yaml", "test_dfas/sub_map_1.yaml");
    //psdfa_1.print();

    DFA dfa_1;
    dfa_1.generateFromFormula("GF(a & Fb)");
    dfa_1.print();

    DFA dfa_2;
    dfa_2.generateFromFormula("F(a & GFb)");
    dfa_2.print();

    auto dfas = createDFAsFromFile("formulas.yaml");
    for (auto & dfa : dfas) dfa->print();
	return 0;
}
