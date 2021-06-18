#include<string>
#include<vector>
#include<iostream>
#include "edge.h"
#include<map>


Edge::Edge(bool ordered_) : ordered(ordered_) {
	ind = 0;
	ind_checkout = 0;
	head = nullptr;
	prev = nullptr;
	heads.clear();
	prevs.clear();
	checking = false;

}

bool Edge::isOrdered() const {
	return ordered;
}

bool Edge::isEmpty() {
	if (head==nullptr) {
		return true;
	} else {
		return false;
	}
}

bool Edge::isListEmpty(Edge::edgelist* head_) const {
	if (head_==nullptr) {
		return true;
	} else {
		return false;
	}
}

void Edge::append(unsigned int nodeind_, float weight_, std::string label_) {
	if (isEmpty()) {
		Edge::edgelist* newNode = new Edge::edgelist;
		//std::cout<<"PTR: "<<newNode<<std::endl;
		newNode->nodeind = nodeind_;
		newNode->weight = 0; 
		newNode->label = "none"; 
		head = newNode;
		heads[ind_checkout] = head;
		prev = newNode;
		prevs[ind_checkout] = prev;
		newNode->adjptr = nullptr;
	} else {
		Edge::edgelist* newNode = new Edge::edgelist;
		//std::cout<<"PTR: "<<newNode<<std::endl;
		newNode->nodeind = nodeind_;
		newNode->weight = weight_;
		newNode->label = label_;

		prev->adjptr = newNode;
		prev = newNode;
		prevs[ind_checkout] = prev;
		newNode->adjptr = nullptr;	
	}
}

void Edge::checkout(int ind_checkout_) {
	if (ind_checkout_<=heads.size()){
		if (isEmpty()) {
			//std::cout<<"im checking out empty"<<std::endl;
			ind_checkout = ind_checkout_;
			//std::cout<<" head ptr:"<<head<<std::endl;
			head = heads[ind_checkout];
			prev = prevs[ind_checkout];	
			checking = true;
		} else {
			if (heads[ind_checkout] == head) {
				//heads[ind_checkout] = head;
				//prevs[ind_checkout] = prev; 
				// reset the pointer keeping track of last node in list using the current checkout
				ind_checkout = ind_checkout_;
				head = heads[ind_checkout];
				prev = prevs[ind_checkout];	
				checking = true;
			} else {
				std::cout<<"Error: Heads are mismatched"<<std::endl;
			}

		}
	} else {
		std::cout << "Index out of bounds for number of lists\n";
	}

}

void Edge::newlist(){
	if (checking) {
		//std::cout<< "checking head:"<<heads[ind_checkout]<< ", reset head:"<<head<<std::endl;
		if (heads[ind_checkout] == head) {
			//heads[ind_checkout] = head;
			//prevs[ind_checkout] = prev; 
			// reset the pointer keeping track of last node in list
			ind_checkout = ind;
			checking = false;
		} else {
			std::cout<<"Error: Heads are mismatched"<<std::endl;
		}
	} else {
		heads.push_back(head);
		prevs.push_back(prev);
		ind = heads.size()-1;
		ind_checkout = ind;
	}
	head = nullptr;
	prev = nullptr;
	append(ind, 0, "none");
} 


int Edge::returnListCount() const {
	if (ind+1 == heads.size()){
		return ind+1;
	} else if (heads.size() == 0) {
		return 0;	
	} else {
		std::cout<<"Error: Number of heads does not match number of lists\n";
		return 0;
	}
}

const std::vector<Edge::edgelist*> Edge::getHeads() const {
	const std::vector<Edge::edgelist*> heads_out = heads;
	return heads_out;

}

void Edge::connect(unsigned int ind_from, unsigned int ind_to, float weight_, std::string label_){
	// Add lists until ind_from and ind_to are included in the graph
	while (ind_from > ind){
		newlist();	
	}
	while (ind_to > ind){
		newlist();
	}

	checkout(ind_from);
	/*
	if (isEmpty()){
		append(ind_from, 0, "none");
	}
	*/
	append(ind_to, weight_, label_);
	if (!ordered){
		checkout(ind_to);
		/*
		if (isEmpty()){
			append(ind_to, 0, "none");
		}
		*/
		append(ind_from, weight_, label_);
	}
}

void Edge::returnListNodes(unsigned int ind_, std::vector<int>& node_list) const {
	auto currptr = heads[ind_];
	node_list.clear();
	node_list.resize(0);
	if (!isListEmpty(currptr)) {
		currptr = currptr->adjptr;
		while (currptr!=nullptr) {
			auto nextptr = currptr->adjptr;
			node_list.push_back(currptr->nodeind);
			currptr = nextptr;
		}
	}
}

void Edge::returnListLabels(unsigned int ind_, std::vector<std::string>& label_list) const {
	auto currptr = heads[ind_];
	label_list.clear();
	label_list.resize(0);
	if (!isListEmpty(currptr)) {
		currptr = currptr->adjptr;
		while (currptr!=nullptr) {
			auto nextptr = currptr->adjptr;
			label_list.push_back(currptr->label);
			currptr = nextptr;
		}
	}
}

void Edge::returnListWeights(unsigned int ind_, std::vector<float>& weights_list) const {
	auto currptr = heads[ind_];
	weights_list.clear();
	if (!isListEmpty(currptr)) {
		currptr = currptr->adjptr;
		while (currptr!=nullptr) {
			auto nextptr = currptr->adjptr;
			weights_list.push_back(currptr->weight);
			currptr = nextptr;
		}
	}
}

void Edge::print() const {
	for (int i=0; i<heads.size(); i++) {
		auto currptr = heads[i];
		std::cout<<"Node: "<<currptr->nodeind<<" connects to:\n";
		currptr = currptr->adjptr;
		while (currptr!=nullptr) {
			auto nextptr = currptr->adjptr;
			std::cout<<"  ~> "<<currptr->nodeind<<"     with label: "<<currptr->label<<"     and weight: "<<currptr->weight<<"\n";

			currptr = nextptr;
		}
	}

}

int Edge::augmentedStateFunc(int i, int j, int n, int m) {
	int ret_int;
	ret_int = m*i+j;
	if (ret_int<=n*m){
		return ret_int;
	} else {
		std::cout<<"Error: augmentedStateFunc mapping out of bounds\n";
	}
}

void Edge::compose(const Edge &mult_graph, Edge& product_graph){
	int n = heads.size();
	int m = mult_graph.returnListCount();
	auto mult_heads = mult_graph.getHeads();
	int ind_from, ind_to;
	for (int i = 0; i<n; i++){
		for (int j = 0; j<m; j++){
			auto currptr_i = heads[i]->adjptr;	
			auto currptr_j = mult_heads[j]->adjptr;	
			ind_from = augmentedStateFunc(i, j, n, m);
			while (currptr_i!=nullptr){
				int i_to = currptr_i->nodeind;
				while (currptr_j!=nullptr){
					int j_to = currptr_j->nodeind;
					ind_to = augmentedStateFunc(i_to, j_to, n, m);

					// Edge weights on composed graph are just the sum of the
					// corresponding edge weights. This line below can be
					// changed if the composition edge weight operator is
					// defined to be something else
					float prod_weight = currptr_i->weight + currptr_j->weight;

					// Edge labels on composed are just the edge labels of the
					// "this" graph. This line below can be edited if the 
					// composition labeling rules needs to be customized/changed
					std::string prod_label = currptr_i->label;
					product_graph.connect(ind_from, ind_to, prod_weight, prod_label);
					currptr_j = currptr_j->adjptr;
				}
				currptr_i = currptr_i->adjptr;
			}
		}
	}
}


void Edge::augmentedStateMap(unsigned int ind_product, int n, int m, std::pair<unsigned int, unsigned int>& ret_indices) {
	unsigned int i = 0;
	unsigned int j;
	while (m*(i+1)<(ind_product+1)){
		i++; 
	}
	j = ind_product % m; 	
	ret_indices.first = i;
	ret_indices.second = j;
}

Edge::~Edge() {
	std::cout<< "Deconstructing " << heads.size() << " lists...\n";

	for (int i=0; i<heads.size(); i++) {
		auto currptr = heads[i];
		while (currptr!=nullptr) {
			auto nextptr = currptr->adjptr;
			//std::cout<<"PTR DELETE: "<<currptr<<std::endl;
			delete currptr;
			currptr = nextptr;
		}
	}

}



