#include<iostream>

#include "core/DirectedAcyclicGraph.h"

using namespace TP;

static std::string intToStr(const int& i) {return std::to_string(i);}

int main() {
 

    DirectedAcyclicGraph<int> graph(&intToStr);
    LOG("Connected 0 to 1?: " << ((graph.connect(0, 1, 1)) ? "yes" : "no"));
    LOG("Connected 0 to 2?: " << ((graph.connect(0, 2, 1)) ? "yes" : "no"));
    LOG("Connected 2 to 1?: " << ((graph.connect(2, 1, 1)) ? "yes" : "no"));
    LOG("Connected 1 to 3?: " << ((graph.connect(1, 3, 1)) ? "yes" : "no"));
    LOG("Connected 3 to 4?: " << ((graph.connect(3, 4, 1)) ? "yes" : "no"));
    LOG("Connected 4 to 2?: " << ((graph.connect(4, 2, 1)) ? "yes" : "no"));

    graph.print();
	return 0;
}
