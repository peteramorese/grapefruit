#include<iostream>
#include "graph.h"

int main() {
	Graph<int> g(true);
	//Graph<std::string> g_s(true);
	std::vector<int> data(3);
	data[0] = 33;
	data[1] = 223;
	data[2] = 39;
	std::vector<std::pair<unsigned int, int*>> nodes(3);
	for (int i=0; i<3; ++i) {
		nodes[i].first = i;
		nodes[i].second = &data[i];
	}
	g.connect(nodes[0], nodes[2]);
	g.connect(nodes[0], nodes[1]);
	g.connect(nodes[1], nodes[0]);
	g.connect(nodes[1], nodes[2]);
	int* p = &data[2];
	g.connect({3, p}, {5, p});
	//g.connect({3, p}, {5, p});
	//g.connect({3, p}, {5, p});
	g.print();

	//std::cout<<"Removing node 2"<<std::endl;
	//g.remove(2);
	
	std::vector<int> node_list;
	std::vector<int*> data_list;
	g.getConnectedNodes(0, node_list);
	g.getConnectedData(0, data_list);
	for (int i=0; i<node_list.size(); ++i) {
		std::cout<<"connected node: "<<node_list[i]<<std::endl;	
		std::cout<<"connected data: "<<*data_list[i]<<std::endl;	
	}
	//std::cout<<"\n\n printing again:"<<std::endl;
	//g.print();

	std::cout<<"made it out phew"<<std::endl;

	return 0;
}
