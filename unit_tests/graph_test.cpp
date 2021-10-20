#include<string>
#include "edge.h"

void dispF(const std::string& method) {
	std::cout<<"\n!!!   FAILURE   !!!  method: "<<method<<"\n";
}

void dispS(const std::string& method) {
	std::cout<<"\nSuccess!  method: "<<method<<"\n";
}

void disp(const std::string& method, bool pass) {
	if (pass) {
		dispS(method)
	}
}

int main() {
	std::cout<<"Unit method test: EDGE\n";
	Edge graph(true);
	graph.connect(0, 0, 0, "");
	graph.connect(1, 0, 0, "");
	graph.connect(1, 1, 5, "1_to_1");
	graph.connect(1, 1, 5, "1_to_1");
	std::string method
	
	// size
	bool pass = true;
	method = "size";
	if (graph.size() != 2) {
		pass = false;	
	}
	graph.connect(2, 5, 1, "2_to_5");
	if (graph.size() != 6) {
		pass = false;
	}


	

	return 0;
}
