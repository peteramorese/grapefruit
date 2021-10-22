#include<iostream>
#include "graph.h"

int main() {
	DFA A;
	int N_DFAs;
	std::cout<<"Enter number of formulas: "<<std::endl;
	std::cin >> N_DFAs;
	std::vector<std::string> filenames(N_DFAs);
	for (int i=0; i<N_DFAs; ++i) {
		filenames[i] = "../spot_automaton_file_dump/dfas/dfa_" + std::to_string(i) +".txt";
	}
	//std::string filename = "../spot_automaton_file_dump/dfas/dfa_0.txt";
	std::cout<<"b4 read"<<std::endl;
	A.readFileSingle(filenames[0]);
	std::cout<<"\n\nPrinting the first read-in DFA..."<<std::endl;
	A.print();

	DFA_EVAL A_EVAL(&A);
	std::cout<<"Curr node: "<<A_EVAL.getCurrNode()<<", is accepting: "<<A_EVAL.isCurrAccepting()<<std::endl;
	A_EVAL.eval("a&!b");
	std::cout<<"Curr node: "<<A_EVAL.getCurrNode()<<", is accepting: "<<A_EVAL.isCurrAccepting()<<std::endl;
	A_EVAL.eval("b");
	std::cout<<"Curr node: "<<A_EVAL.getCurrNode()<<", is accepting: "<<A_EVAL.isCurrAccepting()<<std::endl;
	A_EVAL.eval("!b");
	std::cout<<"Curr node: "<<A_EVAL.getCurrNode()<<", is accepting: "<<A_EVAL.isCurrAccepting()<<std::endl;


	//std::cout<<"\n-------------------------------\n"<<std::endl;
	//std::vector<DFA> dfa_arr(N_DFAs);
	////DFA::readFileMultiple(filename, dfa_arr);
	//for (int i=0; i<N_DFAs; ++i) {
	//	dfa_arr[i].readFileSingle(filenames[i]);
	//}
	//std::cout<<"\n\nPrinting all DFA's in file (read into an array)..."<<std::endl;
	//for (int i=0; i<N_DFAs; ++i) {
	//	dfa_arr[i].print();
	//	std::cout<<"\n"<<std::endl;
	//}
	return 0;
}
