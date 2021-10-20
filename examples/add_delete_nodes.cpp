#include<iostream>
#include "graph.h"

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

	std::string test = "hello";
	std::string* str_ptr = &test;

	Graph<std::string> graph(true);
	graph.connect(0, {0, str_ptr});
	graph.connect(0, {1, str_ptr});
	graph.connect(1, {2, str_ptr});
	graph.connect(2, {4, str_ptr});
	graph.connect(3, {1, str_ptr});
	graph.connect(2, {3, str_ptr});

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
