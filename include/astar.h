#pragma once
#include<iostream>
#include "graph.h"
template<class T>
class Astar {
	private:
		Graph<T>* e;
		unsigned int Nv; //number of nodes (length of heads vector)
		int vinit; //initial node
		std::vector<int> vgoal_set;
		//int vgoal; //goal node
		bool initialized[3];
		struct listO;
	public:
		Astar();
		void setGraph(Graph<T>* e_);
		void setVInit(unsigned int vinit_);
		void setVGoalSet(const std::vector<int>& vgoal_set_);
		bool searchDijkstra(std::vector<int>& path, float& pathlength);

};
