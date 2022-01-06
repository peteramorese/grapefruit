#include<iostream>
#include "graph.h"

int main() {
	Graph<int> g(true, true);
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
	g.getConnectedNodes(1, node_list);
	g.getConnectedData(1, data_list);
	for (int i=0; i<node_list.size(); ++i) {
		std::cout<<"connected node: "<<node_list[i]<<std::endl;	
		std::cout<<"connected data: "<<*data_list[i]<<std::endl;	
	}
	std::cout<<"Printing parent nodes and data of node 2"<<std::endl;
	g.getParentNodes(2, node_list);
	g.getParentData(2, data_list);
	for (int i=0; i<node_list.size(); ++i) {
		std::cout<<"connected node: "<<node_list[i]<<std::endl;	
		std::cout<<"connected data: "<<*data_list[i]<<std::endl;	
	}
	//std::cout<<"\n\n printing again:"<<std::endl;
	//g.print();

	std::vector<int> inds = {4, 2, 3};
	std::vector<int> graph_sizes = {5, 4, 4};

	int ret_ind = Graph<int>::augmentedStateImage(inds, graph_sizes);
	std::cout<<"Product ind: "<<ret_ind<<std::endl;

	std::vector<int> ret_inds;
	Graph<int>::augmentedStatePreImage(graph_sizes, ret_ind, ret_inds);
	for (int i=0; i<ret_inds.size(); ++i) {
		std::cout<<"PreImage ind: "<<ret_inds[i]<<std::endl;
	}

	return 0;
}
