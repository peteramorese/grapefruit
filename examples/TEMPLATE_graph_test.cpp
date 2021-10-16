#include<iostream>
#include "TEMPLATE_graph.h"

int main() {
	Graph<int> g(true);
	std::vector<int> data(3);
	data[0] = 33;
	data[1] = 223;
	data[2] = 39;
	std::vector<std::pair<unsigned int, int*>> nodes(3);
	std::cout<<"hello"<<std::endl;
	for (int i=0; i<3; ++i) {
		nodes[i].first = i;
		nodes[i].second = &data[i];
	}
	g.connect(nodes[0], nodes[1]);
	g.connect(nodes[1], nodes[0]);
	g.connect(nodes[1], nodes[2]);
	g.print();

	return 0;
}
