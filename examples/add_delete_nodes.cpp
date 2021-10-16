#include<iostream>
#include "edge.h"

int main() {
	//std::vector<int*> hello(0, nullptr);
	//int* boop = new int;
	//hello.push_back(boop);
	//hello.push_back(boop);
	//hello.resize(5);
	//for (int i=0; i<hello.size(); ++i) {
	//	std::cout<<hello[i]<<std::endl;
	//}
	//
	//delete boop;

	Edge graph(true);
	graph.connect(0,0,1,"hello_0_0");
	graph.connect(0,1,1,"hello_0_1");
	graph.connect(1,2,1,"hello_1_2");
	graph.connect(2,4,1,"hello_2_4");
	graph.connect(3,1,1,"hello_3_1");
	graph.connect(2,3,1,"hello_2_3");

	std::cout<<"Initial graph:"<<std::endl;
	graph.print();

	std::cout<<"\n Removing node '2'... \n"<<std::endl;
	graph.remove(2);
	std::cout<<"Removed node '2':"<<std::endl;
	graph.print();

	std::cout<<"\n Removing node '3'... \n"<<std::endl;
	graph.remove(3);
	std::cout<<"Removed node '3':"<<std::endl;
	graph.print();
	return 0;
}
