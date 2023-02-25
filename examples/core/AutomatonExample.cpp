#include<iostream>

#include "core/Automaton.h"
#include "theory/PartialSatisfactionAutomaton.h"

using namespace TP;
using namespace TP::FormalMethods;

int main() {
    PartialSatisfactionDFA psdfa_0;
    PartialSatisfactionDFA psdfa_1;
    
    psdfa_0.deserialize("test_dfas/dfa_0.yaml", "test_dfas/sub_map_0.yaml");
    psdfa_0.print();

    psdfa_1.deserialize("test_dfas/dfa_1.yaml", "test_dfas/sub_map_1.yaml");
    psdfa_1.print();
	return 0;
}