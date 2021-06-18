#pragma once
#include<iostream>
#include "edge.h"
class Astar {
	private:
		Edge* e;
		unsigned int Nv; //number of nodes (length of heads vector)
		int vinit; //initial node
		int vgoal; //goal node
		bool initialized[3];
		struct listO;
	public:
		Astar();
		void setGraph(Edge* e_);
		void setVInit(unsigned int vinit_);
		void setVGoal(unsigned int vgoal_);
		bool searchDijkstra(std::vector<int>& path, float& pathlength);

};
