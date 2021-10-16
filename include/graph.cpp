#include<string>
#include<vector>
#include<iostream>
#include<unordered_map>
#include "graph.h"

//template<class T>
//const static auto typename printLAM = [](Graph<T>::node* dst){std::cout<<dst->nodeind;};

template<class T>
Graph<T>::Graph(bool ordered_) : ordered(ordered_), heads(0, nullptr), tails(0, nullptr){
	tails.clear();
	heads.clear();
}

template<class T>
bool Graph<T>::isOrdered() const {
	return ordered;
}

template<class T>
bool Graph<T>::isEmpty(Graph<T>::node* head_) const {
	return (head_ == nullptr) ? true : false;
	//if (head_==nullptr) {
	//	return true;
	//} else {
	//	return false;
	//}
}

template<class T>
bool Graph<T>::connect(const std::pair<unsigned int, T*>& src, const std::pair<unsigned int, T*>& dst) {
	unsigned int max_ind = (src.first >= dst.first) ? src.first : dst.first;
	if (max_ind >= heads.size()) {
		heads.resize(max_ind + 1);
		tails.resize(max_ind + 1);
	}

	if (isEmpty(heads[src.first])) {
		Graph<T>::node* new_node_e = new Graph<T>::node;
		//std::cout<<"PTR ADDED: "<<new_node_e<<std::endl;
		new_node_e->nodeind = src.first;
		new_node_e->dataptr = src.second;
		new_node_e->adjptr = nullptr;
		heads[src.first] = new_node_e;
		tails[src.first] = new_node_e;
	} 
	if (isEmpty(heads[dst.first])) {
		Graph<T>::node* new_node_e = new Graph<T>::node;
		//std::cout<<"PTR ADDED: "<<new_node_e<<std::endl;
		new_node_e->nodeind = dst.first;
		new_node_e->dataptr = dst.second;
		new_node_e->adjptr = nullptr;
		heads[dst.first] = new_node_e;
		tails[dst.first] = new_node_e;
	}

	Graph<T>::node* new_node = new Graph<T>::node;
	//std::cout<<"PTR ADDED: "<<new_node<<std::endl;
	new_node->nodeind = dst.first;
	new_node->dataptr = dst.second;
	new_node->adjptr = nullptr;	
	// Update the tail pointer to the appended element:
	tails[src.first]->adjptr = new_node;
	tails[src.first] = new_node;
	return true;
}

template<class T>
bool Graph<T>::connect(unsigned int src_ind, const std::pair<unsigned int, T*>& dst) {
	unsigned int max_ind = (src_ind >= dst.first) ? src_ind : dst.first;
	if (max_ind >= heads.size()) {
		heads.resize(max_ind + 1);
		tails.resize(max_ind + 1);
	}

	if (isEmpty(heads[src_ind])) {
		Graph<T>::node* new_node_e = new Graph<T>::node;
		//std::cout<<"PTR ADDED: "<<new_node_e<<std::endl;
		new_node_e->nodeind = src_ind;
		new_node_e->dataptr = nullptr;
		new_node_e->adjptr = nullptr;
		heads[src_ind] = new_node_e;
		tails[src_ind] = new_node_e;
	} 
	if (isEmpty(heads[dst.first])) {
		Graph<T>::node* new_node_e = new Graph<T>::node;
		//std::cout<<"PTR ADDED: "<<new_node_e<<std::endl;
		new_node_e->nodeind = dst.first;
		new_node_e->dataptr = dst.second;
		new_node_e->adjptr = nullptr;
		heads[dst.first] = new_node_e;
		tails[dst.first] = new_node_e;
	}

	Graph<T>::node* new_node = new Graph<T>::node;
	//std::cout<<"PTR ADDED: "<<new_node<<std::endl;
	new_node->nodeind = dst.first;
	new_node->dataptr = dst.second;
	new_node->adjptr = nullptr;	
	// Update the tail pointer to the appended element:
	tails[src_ind]->adjptr = new_node;
	tails[src_ind] = new_node;
	return true;
}

template<class T> 
template<typename LAM>
bool Graph<T>::hop(unsigned int ind, LAM lambda) {
	node* prevptr = heads[ind];
	//std::cout<<"prevptr: "<<prevptr<<std::endl;
	node* currptr = heads[ind]->adjptr; 
	//std::cout<<"currptr: "<<currptr<<std::endl;
	while (currptr!=nullptr) {
		node* nextptr = currptr->adjptr;
		//std::cout<<"   currptr going in: "<<currptr<<", prevptr going in: "<<prevptr<<std::endl;
		lambda(currptr, prevptr);
		//std::cout<<"   currptr coming out: "<<currptr<<", prevptr coming out: "<<prevptr<<std::endl;
		prevptr = currptr;
		currptr = nextptr;
	}
	return true;
}

template<class T>
int Graph<T>::size() const {
	return heads.size();
}

template<class T>
void Graph<T>::remove(unsigned int ind_) {
	auto rmLAM = [&](Graph<T>::node*& dst, Graph<T>::node*& prv){
		//std::cout<<"       dst->nodeind: "<<dst->nodeind<<", ind: "<<ind_<<std::endl;
		if (dst->nodeind == ind_) {
			prv->adjptr = dst->adjptr;
			std::cout<<"Deleted: "<<dst<<std::endl;
			//prv = prv->adjptr;
			node* tempptr = dst->adjptr;
			delete dst;
			dst = tempptr;
		}
	};
	auto deleteLAM = [](Graph<T>::node* dst, Graph<T>::node* prv){std::cout<<"deleting: "<<dst<<std::endl; delete dst;};
	for (int i=0; i<heads.size(); ++i) {
		//std::cout<<"CURRENT NODE IND: "<<i<<std::endl;
		if (i == ind_) {
			//std::cout<<"DELETING ENTIRE LIST"<<std::endl;
			hop(i, deleteLAM);	
			if (!isEmpty(heads[i])) {
				std::cout<<"Deleting: "<<heads[i]<<std::endl;
				delete heads[i];
				heads[i] = nullptr;
			}
		} else {
			//std::cout<<"SEARCHING FOR ELEMENT"<<std::endl;
			hop(i, rmLAM);
		}
	}
}

template<class T>
void Graph<T>::getConnectedNodes(unsigned int ind, std::vector<int>& node_list) {
	node_list.clear();
	auto fillNodesLAM = [&node_list](Graph<T>::node* dst, Graph<T>::node* prv){node_list.push_back(dst->nodeind);};
	hop(ind, fillNodesLAM);
}

template<class T>
void Graph<T>::getConnectedData(unsigned int ind, std::vector<T*>& data_list) {
	data_list.clear();
	auto fillDataLAM = [&data_list](Graph<T>::node* dst, Graph<T>::node* prv){data_list.push_back(dst->dataptr);};
	hop(ind, fillDataLAM);
}

template<class T>
void Graph<T>::print() {
	auto printLAM = [](Graph<T>::node* dst, Graph<T>::node* prv){std::cout<<"   ~> "<<dst->nodeind<<"\n";};
	for (int i=0; i<heads.size(); i++) {
		if (!isEmpty(heads[i])) {
			if (!isEmpty(heads[i]->adjptr)) {
				std::cout<<"Node: "<<i<<" is connected to:\n";
				hop(i, printLAM);
			} else {
				std::cout<<"Node: "<<i<<" has no outgoing transitions.\n";
			}
		}
	}
}

template<class T>
void Graph<T>::updateData(unsigned int ind_from, unsigned int ind_to, T* dataptr_) {
	if (ind_from < heads.size()) {
		auto updDataLAM = [&](Graph<T>::node* dst, Graph<T>::node* prv){
			if (dst->nodeind == ind_to) {
				dst->dataptr = dataptr_;
			}
		};
		hop(ind_from, updDataLAM);
	}
}

//template<class T>
//void Graph<T>::updateWeight(unsigned int ind_from, unsigned int ind_to, float weight_) {
//	if (ind_from < heads.size()) {
//		auto currptr = heads[ind_from]->adjptr;
//		//std::cout<<"Node: "<<currptr->nodeind<<" "<<currptr<<" connects to:\n";
//		bool found = false;
//		while (currptr!=nullptr) {
//			if (currptr->nodeind == ind_to) {
//				found = true;
//				currptr->weight = weight_;
//				break; 
//				// REMOVE THIS BREAK STATEMENT TO UPDATE ALL
//				// EDGES THAT CONNECT TWO STATES. THIS WILL 
//				// DECREASE THE EFFICIENCY
//			}
//			currptr = currptr->adjptr;
//		}
//		if (!found) {
//			std::cout<<"Error: Update ind_to not found within list\n";
//		}
//	} else {
//		std::cout<<"Error: Update ind_from out of bounds\n";
//	}
//}

template<class T>
int Graph<T>::augmentedStateFunc(int i, int j, int n, int m) {
	int ret_int;
	ret_int = m*i+j;
	if (ret_int<=n*m){
		return ret_int;
	} else {
		std::cout<<"Error: augmentedStateFunc mapping out of bounds\n";
		return -1;
	}
}

template<class T>
void Graph<T>::augmentedStateMap(unsigned int ind_product, int n, int m, std::pair<unsigned int, unsigned int>& ret_indices) {
	unsigned int i = 0;
	unsigned int j;
	while (m*(i+1)<(ind_product+1)){
		i++; 
	}
	j = ind_product % m; 	
	ret_indices.first = i;
	ret_indices.second = j;
}

template<class T>
void Graph<T>::clear() {
	std::cout<< "Clearing " << heads.size() << " lists...\n";

}

//template<class T>
//void Graph<T>::clear() {
//	std::cout<< "Clearing " << heads.size() << " lists...\n";
//
//	for (int i=0; i<heads.size(); i++) {
//		auto currptr = heads[i];
//		while (currptr!=nullptr) {
//			auto nextptr = currptr->adjptr;
//			//std::cout<<"PTR DELETE: "<<currptr<<std::endl;
//			delete currptr;
//			currptr = nextptr;
//		}
//	}
//	prevs.clear();
//	heads.clear();
//	head = nullptr;
//	prev = nullptr;
//	ind = 0;
//	ind_checkout = 0;
//	checking = false;
//}



template<class T>
Graph<T>::~Graph() {
	auto deleteLAM = [](Graph<T>::node* dst, Graph<T>::node* prv){
		//std::cout<<"deleting: "<<dst<<std::endl; 
		delete dst;
	};
	std::cout<< "Deconstructing " << heads.size() << " lists...\n";
	for (int i=0; i<heads.size(); i++) {
		//std::cout<<"CURRENT LIST: "<<i<<std::endl;
		if (!isEmpty(heads[i])) {
			hop(i, deleteLAM);
			//std::cout<<"deleting: "<<heads[i]<<std::endl;
			delete heads[i];
		}
	}
}

template class Graph<int>;
template class Graph<unsigned int>;
template class Graph<float>;
template class Graph<double>;
template class Graph<std::string>;
template class Graph<WL>;
template class Graph<WLI>;

