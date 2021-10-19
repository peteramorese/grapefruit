#include<iostream>
#include<vector>
#include "astar.h"
#include "edge.h"

template<class T>
Astar<T>::Astar() {
	for (int i=0;i<3;i++){
		initialized[i] = false; 
	}
}

template<class T>
void Astar<T>::setGraph(Graph<T>* e_) {
	e = e_;
	Nv = e->size();
	initialized[0] = true;
}

template<class T>
void Astar<T>::setVInit(unsigned int vinit_){
	if (vinit_>Nv-1){
		std::cout<<"Error: vinit index is larger than any node index"<<std::endl;
	} else {
		vinit = vinit_;	
		initialized[1] = true;
	}
}

template<class T>
void Astar<T>::setVGoalSet(const std::vector<int>& vgoal_set_){
	bool in_bounds = true;
	for (int i=0; i<vgoal_set_.size(); i++) {
		if (vgoal_set_[i]>Nv-1){
			in_bounds = false;
			break;
		} 
	}	
	if (in_bounds) {
		vgoal_set = vgoal_set_;	
		initialized[2] = true;
	} else {
		std::cout<<"Error: vgoal index is larger than any node index"<<std::endl;
	}
}

template<class T>
struct Astar<T>::listO{
	unsigned int node;
	float g;
	int parent;
};

template<class T>
bool Astar<T>::searchDijkstra(std::vector<int>& path, float& pathlength){
	bool init = true;
	bool path_found = false;
	int vgoal;
	std::vector<int> reverse_path;
	reverse_path.clear();
	if (!reverse_path.empty()){
		init = false;
	} else if (!(initialized[0] && initialized[1] && initialized[2])){
		init = false;	
	}

	if (init) {
		pathlength = 0;
		std::vector<listO> O;
		std::vector<listO> C;
		O.resize(1);
		C.resize(0);
		O[0].node = vinit;
		O[0].g = 0;
		O[0].parent = -1;

		while (O.size()!=0){
			float minf = 0;
			float ming;
			bool begin = true;
			int nbest, minI;
			for (int i = 0; i<O.size(); i++){
				float gi = 0;
				gi = O[i].g;	
				if ((gi<minf)||begin){
					minf = gi;
					ming = gi;
					minI = i;
					nbest = O[i].node;
					begin = false;
				}
			}
			C.push_back(O[minI]);

			// Search through the goal set to check if nbest is included
			bool goal_found = false;
			for (int i=0; i<vgoal_set.size(); i++){
				if (nbest == vgoal_set[i]) {
					vgoal = vgoal_set[i];
					goal_found = true;
					break;
				}
			}
			if (goal_found) {
				break;
			}
			O[minI] = O[O.size()-1];
			O.pop_back();

			std::vector<int> star_set;
			std::vector<float> star_weight_set;  
			auto heads = e->getHeads();
			auto currptr = heads->operator[](nbest)->adjptr;
			while (currptr!=nullptr) {
				auto nextptr = currptr->adjptr;
				bool in_C = false;
				for (int i = 0; i<C.size(); i++){
					if (C[i].node==currptr->nodeind){
						in_C = true;
					}
				}
				if (!in_C){
					star_set.push_back(currptr->nodeind);
					star_weight_set.push_back(currptr->weight);
				}
				//std::cout<<"pushing back"<<currptr->nodeind<<std::endl;
				currptr = nextptr;
			}
			for (int i=0; i<star_set.size(); i++){
				int inO_i = -1;
				for (int ii=0; ii<O.size(); ii++){
					if (O[ii].node==star_set[i]){
						inO_i = ii;	
					}
				}
				if (inO_i==-1){
					listO O_add;
					O_add.node = star_set[i];
					O_add.g = star_weight_set[i]+ming;
					O_add.parent = nbest;
					O.push_back(O_add);
				} else if ((ming + star_weight_set[i])<O[inO_i].g){
					O[inO_i].g = ming + star_weight_set[i];
					O[inO_i].parent = nbest;
				}
			}
		}
		bool reached_goal = false;
		for (int i=0; i<C.size(); i++){
			if (C[i].node==vgoal){
				reached_goal = true;
			}
		}
		if (reached_goal){
			int currnode = vgoal;
			reverse_path.push_back(vgoal);
			while (currnode!=vinit){
				for (int i=0; i<C.size(); i++){
					if (C[i].node==currnode){
						if (currnode==vgoal){
							pathlength = C[i].g;
						}
						currnode = C[i].parent;
						reverse_path.push_back(currnode);
					}	
				}
			}	
			path.resize(reverse_path.size());
			std::cout<<"REVERSE PATH SIZE: "<<reverse_path.size()<<std::endl;
			for (int i=0; i<reverse_path.size(); ++i) {
				path[i] = reverse_path[reverse_path.size()-1-i];
			}
			std::cout<<"PATH SIZE: "<<path.size()<<std::endl;
			
			path_found = true;
		} else {
			std::cout<<"Could not find path"<<std::endl;
		}
	} else {
		std::cout<<"Cannot call 'search'"<<std::endl;
	}
	return path_found;
}




