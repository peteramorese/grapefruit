#pragma once
#include<iostream>
#include "edge.h"
class Astar {
	private:
		Edge* e;
		unsigned int Nv; //number of nodes (length of heads vector)
		int vinit; //initial node
		std::vector<int> vgoal_set;
		//int vgoal; //goal node
		bool initialized[3];
		struct listO;
	public:
		Astar();
		void setGraph(Edge* e_);
		void setVInit(unsigned int vinit_);
		void setVGoalSet(const std::vector<int>& vgoal_set_);
		bool searchDijkstra(std::vector<int>& path, float& pathlength);

};
