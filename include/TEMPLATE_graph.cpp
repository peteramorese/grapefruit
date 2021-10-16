#include<string>
#include<vector>
#include<iostream>
#include<map>
#include "TEMPLATE_graph.h"

//template<class T>
//const static auto typename printLAMB = [](Graph<T>::node* dst){std::cout<<dst->nodeind;};

template<class T>
Graph<T>::Graph(bool ordered_) : ordered(ordered_), heads(0, nullptr), tails(0, nullptr){
	ind = 0;
	ind_checkout = 0;
	head = nullptr;
	prev = nullptr;
	//heads.clear();
	tails.clear();
	checking = false;

}

template<class T>
bool Graph<T>::isOrdered() const {
	return ordered;
}

//template<class T>
//bool Graph<T>::isEmpty() {
//	return (head == nullptr) ? true : false;
//	//if (head==nullptr) {
//	//	return true;
//	//} else {
//	//	return false;
//	//}
//}

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
bool Graph<T>::connect(std::pair<unsigned int, T*> src, std::pair<unsigned int, T*> dst) {
	unsigned int max_ind = (src.first >= dst.first) ? src.first : dst.first;
	if (max_ind >= heads.size()) {
		heads.resize(max_ind + 1);
		tails.resize(max_ind + 1);
	}

	if (isEmpty(heads[src.first])) {
		Graph<T>::node* new_node_e = new Graph<T>::node;
		std::cout<<"PTR ADDED: "<<new_node_e<<std::endl;
		new_node_e->nodeind = src.first;
		new_node_e->dataptr = src.second;
		new_node_e->adjptr = nullptr;
		heads[src.first] = new_node_e;
		tails[src.first] = new_node_e;
	} 
	if (isEmpty(heads[dst.first])) {
		Graph<T>::node* new_node_e = new Graph<T>::node;
		std::cout<<"PTR ADDED: "<<new_node_e<<std::endl;
		new_node_e->nodeind = dst.first;
		new_node_e->dataptr = dst.second;
		new_node_e->adjptr = nullptr;
		heads[dst.first] = new_node_e;
		tails[dst.first] = new_node_e;
	}

	Graph<T>::node* new_node = new Graph<T>::node;
	//std::cout<<"PTR ADDED: "<<newNode<<std::endl;
	new_node->nodeind = dst.first;
	new_node->dataptr = dst.second;
	new_node->adjptr = nullptr;	
	// Update the tail pointer to the appended element:
	tails[src.first]->adjptr = new_node;
	tails[src.first] = new_node;
	return true;
}

template<class T> 
template<typename LAM>
bool Graph<T>::hop(unsigned int ind, LAM lambda) {
	node* currptr = heads[ind]->adjptr; 
	while (currptr!=nullptr) {
		node* nextptr = currptr->adjptr;
		//std::cout<<"PTR DELETE: "<<currptr<<std::endl;
		lambda(currptr);
		currptr = nextptr;
	}
}

//template<class T>
//void Graph<T>::checkout(int ind_checkout_) {
//	if (ind_checkout_<=heads.size()){
//		if (heads[ind_checkout] == head) {
//			// reset the pointer keeping track of last node in list using the current checkout
//			ind_checkout = ind_checkout_;
//			head = heads[ind_checkout];
//			prev = prevs[ind_checkout];	
//			checking = true;
//		} else {
//			std::cout<<"Error: Heads are mismatched"<<std::endl;
//		}
//
//		//}
//	} else {
//		std::cout << "Index out of bounds for number of lists\n";
//	}
//
//}
//
//template<class T>
//void Graph<T>::newlist(){
//	if (checking) {
//		//std::cout<< "checking head:"<<heads[ind_checkout]<< ", reset head:"<<head<<std::endl;
//		if (heads[ind_checkout] == head) {
//			// reset the pointer keeping track of last node in list
//			ind_checkout = ind;
//			checking = false;
//		} else {
//			std::cout<<"Error: Heads are mismatched"<<std::endl;
//		}
//	} else {
//		heads.push_back(head);
//		prevs.push_back(prev);
//		ind = heads.size()-1;
//		ind_checkout = ind;
//		head = nullptr;
//		prev = nullptr;
//		append(ind, 0, "none");
//	}
//} 


template<class T>
int Graph<T>::size() const {
	return heads.size();
	//if (ind+1 == heads.size()){
	//	return ind+1;
	//} else if (heads.size() == 0) {
	//	return 0;	
	//} else {
	//	std::cout<<"Error: Number of heads does not match number of lists\n";
	//	return 0;
	//}
}

//template<class T>
//const std::vector<Graph<T>::node*>* Graph<T>::getHeads() const {
//	const std::vector<Graph<T>::node*> heads_out = heads;
//	return &heads_out;
//}

//template<class T>
//bool Graph<T>::connect(unsigned int ind_from, unsigned int ind_to, float weight_, std::string label_){
//	// Add lists until ind_from and ind_to are included in the graph
//	while (ind_from > ind){
//		newlist();	
//	}
//	while (ind_to > ind){
//		newlist();
//	}
//	if (heads.size() == 0) {
//		newlist();
//	}
//
//	checkout(ind_from);
//	append(ind_to, weight_, label_);
//	if (!ordered){
//		checkout(ind_to);
//		append(ind_from, weight_, label_);
//	}
//	return true;
//}

template<class T>
bool Graph<T>::remove(unsigned int ind_) {
	for (int i=0; i<heads.size(); i++) {
		std::cout<<"i: "<<i<<std::endl;
		if (i == ind_) {
			std::cout<<"Deleting list: "<<ind_<<std::endl;
			//deleteList(heads[ind_]);
			std::cout<<"done deleting list... "<<std::endl;
			//remove element from vector
		} else {
			auto prevptr = heads[i];
			auto currptr = heads[i]->adjptr;
			currptr = currptr->adjptr;
			std::cout<<"currptr: "<<currptr<<std::endl;
			while (currptr!=nullptr) {
				auto nextptr = currptr->adjptr;
				std::cout<<"Curr node: "<<currptr->nodeind<<std::endl;
				if (currptr->nodeind == ind_) {
					std::cout<<"Found node to delete: "<<currptr->nodeind<<std::endl;
					prevptr->adjptr = currptr->adjptr;
					std::cout<<"Deleted: "<<currptr<<std::endl;
					delete currptr;

				}
				prevptr = currptr;
				currptr = nextptr;
			}
		}
	}
}

//template<class T>
//void Graph<T>::returnListNodes(unsigned int ind_, std::vector<int>& node_list) const {
//	auto currptr = heads[ind_];
//	node_list.clear();
//	node_list.resize(0);
//	if (!isListEmpty(currptr)) {
//		currptr = currptr->adjptr;
//		while (currptr!=nullptr) {
//			auto nextptr = currptr->adjptr;
//			node_list.push_back(currptr->nodeind);
//			currptr = nextptr;
//		}
//	}
//}

//template<class T>
//void Graph<T>::returnListLabels(unsigned int ind_, std::vector<std::string>& label_list) const {
//	auto currptr = heads[ind_];
//	label_list.clear();
//	label_list.resize(0);
//	if (!isListEmpty(currptr)) {
//		currptr = currptr->adjptr;
//		while (currptr!=nullptr) {
//			auto nextptr = currptr->adjptr;
//			label_list.push_back(currptr->label);
//			currptr = nextptr;
//		}
//	}
//}

//template<class T>
//void Graph<T>::returnListWeights(unsigned int ind_, std::vector<float>& weights_list) const {
//	auto currptr = heads[ind_];
//	weights_list.clear();
//	if (!isListEmpty(currptr)) {
//		currptr = currptr->adjptr;
//		while (currptr!=nullptr) {
//			auto nextptr = currptr->adjptr;
//			weights_list.push_back(currptr->weight);
//			currptr = nextptr;
//		}
//	}
//}

template<class T>
void Graph<T>::print() {
	auto printLAMB = [](Graph<T>::node* dst){std::cout<<"   ~> "<<dst->nodeind<<"\n";};
	for (int i=0; i<heads.size(); i++) {
		if (!isEmpty(heads[i]->adjptr)) {
			std::cout<<"Node: "<<i<<" is connected to:\n";
			hop(i, printLAMB);
		} else {
			std::cout<<"Node: "<<i<<" has no outgoing transitions.\n";
		}
		//auto currptr = heads[i];
		//std::cout<<"Node: "<<currptr->nodeind<<" "<<currptr<<" connects to:\n";
		//currptr = currptr->adjptr;
		//while (currptr!=nullptr) {
		//	auto nextptr = currptr->adjptr;
		//	std::cout<<"  ~> "<<currptr->nodeind<<"     with label: "<<currptr->label<<"     and weight: "<<currptr->weight<<"\n";

		//	currptr = nextptr;
		//}
	}

}

//template<class T>
//float Graph<T>::getWeight(unsigned int ind_from, unsigned int ind_to) const {
//	if (ind_from < heads.size()) {
//		auto currptr = heads[ind_from]->adjptr;
//		//std::cout<<"Node: "<<currptr->nodeind<<" "<<currptr<<" connects to:\n";
//		bool found = false;
//		float ret_weight;
//		while (currptr!=nullptr) {
//			if (currptr->nodeind == ind_to) {
//				found = true;
//				ret_weight = currptr->weight;
//				break; 
//				// REMOVE THIS BREAK STATEMENT TO UPDATE ALL
//				// EDGES THAT CONNECT TWO STATES. THIS WILL 
//				// DECREASE THE EFFICIENCY
//			}
//			currptr = currptr->adjptr;
//		}
//		if (!found) {
//			std::cout<<"Error: Update ind_to not found within list\n";
//			return 0;
//		} else {
//			return ret_weight;
//		}
//	} else {
//		std::cout<<"Error: Update ind_from out of bounds\n";
//		return 0;
//	}
//}

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

//template<class T>
//void Graph<T>::compose(const Graph &mult_graph, Graph& product_graph){
//	int n = heads.size();
//	int m = mult_graph.size();
//	auto mult_heads = mult_graph.getHeads();
//	int ind_from, ind_to;
//	for (int i = 0; i<n; i++){
//		for (int j = 0; j<m; j++){
//			auto currptr_i = heads[i]->adjptr;	
//			auto currptr_j = mult_heads[j]->adjptr;	
//			ind_from = augmentedStateFunc(i, j, n, m);
//			while (currptr_i!=nullptr){
//				int i_to = currptr_i->nodeind;
//				while (currptr_j!=nullptr){
//					int j_to = currptr_j->nodeind;
//					ind_to = augmentedStateFunc(i_to, j_to, n, m);
//
//					// Graph weights on composed graph are just the sum of the
//					// corresponding edge weights. This line below can be
//					// changed if the composition edge weight operator is
//					// defined to be something else
//					float prod_weight = currptr_i->weight + currptr_j->weight;
//
//					// Graph labels on composed are just the edge labels of the
//					// "this" graph. This line below can be edited if the 
//					// composition labeling rules needs to be customized/changed
//					std::string prod_label = currptr_i->label;
//					product_graph.connect(ind_from, ind_to, prod_weight, prod_label);
//					currptr_j = currptr_j->adjptr;
//				}
//				currptr_i = currptr_i->adjptr;
//			}
//		}
//	}
//}


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

//template<class T>
//void Graph<T>::deleteList(node* head) {
//	auto currptr = head;
//	while (currptr!=nullptr) {
//		auto nextptr = currptr->adjptr;
//		//std::cout<<"PTR DELETE: "<<currptr<<std::endl;
//		delete currptr;
//		currptr = nextptr;
//	}
//}

template<class T>
Graph<T>::~Graph() {
	auto deleteLAMB = [](Graph<T>::node* dst){std::cout<<"deleting: "<<dst<<std::endl; delete dst;};
	std::cout<< "Deconstructing " << heads.size() << " lists...\n";
	for (int i=0; i<heads.size(); i++) {
		//deleteList(heads[i]);
		std::cout<<"deleting: "<<heads[i]<<std::endl;
		delete heads[i];
		//auto currptr = heads[i];
		//while (currptr!=nullptr) {
		//	auto nextptr = currptr->adjptr;
		//	//std::cout<<"PTR DELETE: "<<currptr<<std::endl;
		//	delete currptr;
		//	currptr = nextptr;
		//}
	}
}



//PrioQueue

template class Graph<int>;

