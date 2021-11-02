#include<iostream>
#include "lexSet.h"

int main() {
	LexSet C1(3);
	std::vector<float> set_set_1 = {40, 15, 95};
	C1 = set_set_1;
	std::cout<<"C1: "<<std::endl;
	C1.print();

	LexSet C2(3);
	std::vector<float> set_set_2 = {40, 15, 65};
	C2 = set_set_2;
	std::cout<<"C2: "<<std::endl;
	C2.print();

	std::string gt = (C2 > C1) ? "is" : "is not";
	std::cout<<"C2 "<<gt<<" greater than C1 ( !(C2 > C1))\n"<<std::endl;

	LexSet InfSet(3);
	InfSet.setInf();
	std::cout<<"InfSet:"<<std::endl;
	InfSet.print();

	gt = (C2 > InfSet) ? "is" : "is not";
	std::cout<<"C2 "<<gt<<" greater than InfSet ( !(C2 > InfSet))\n"<<std::endl;


	FlexLexSetS F1(2.0, 3);
	F1 = set_set_1;
	std::cout<<"F1: "<<std::endl;
	F1.print();

	FlexLexSetS F2(2.0, 3);
	F2 = set_set_2;
	std::cout<<"F2: "<<std::endl;
	F2.print();

	gt = (F2 > F1) ? "is" : "is not";
	std::cout<<"F2 "<<gt<<" greater than F1 ( !(F2 > F1))"<<std::endl;


	REQLex R1(10.0, 3);
	R1 = set_set_1;
	std::cout<<"R1: "<<std::endl;
	R1.print();

	REQLex R2(10.0, 3);
	R2 = set_set_2;
	std::cout<<"R2: "<<std::endl;
	R2.print();

	std::vector<float> set_set_3 = {35, 35, 35};
	REQLex R3(10.0, 3);
	R3 = set_set_3;
	std::cout<<"R3: "<<std::endl;
	R3.print();

	gt = (R3 > R1) ? "is" : "is not";
	std::cout<<"R3 "<<gt<<" greater than R1 ( !(R2 > R1))"<<std::endl;

	gt = (R2 > R3) ? "is" : "is not";
	std::cout<<"R2 "<<gt<<" greater than R3 ( !(R2 > R1))"<<std::endl;

	return 0;
}
