#include<iostream>
#include "automaton.h"

int main() {
	DFA A;
	std::string filename = "../spot_automaton_file_dump/dfas.txt";
	A.readFromFile(filename);
	std::cout<<"hello from the end"<<std::endl;
	A.print();
	return 0;
}
