#include<string>
#include<vector>
#include<iostream>
#include<fstream>
#include<unordered_map>
#include "graph.h"
#include "state.h"

//template<class T>
//const static auto typename printLAM = [](Graph<T>::node* dst){std::cout<<dst->nodeind;};

template<class T>
Graph<T>::Graph(bool ordered_, bool reversible_) : ordered(ordered_), STORE_PARENTS(reversible_), heads(0, nullptr), tails(0, nullptr){
	tails.clear();
	heads.clear();
}

template<class T>
bool Graph<T>::isOrdered() const {
	return ordered;
}

template<class T>
bool Graph<T>::isReversible() const {
	return STORE_PARENTS;
}

template<class T>
bool Graph<T>::isEmpty(Graph<T>::node* head_) const {
	return (head_ == nullptr) ? true : false;
}

template<class T>
bool Graph<T>::connect(const std::pair<unsigned int, T*>& src, const std::pair<unsigned int, T*>& dst) {
	unsigned int max_ind = (src.first >= dst.first) ? src.first : dst.first;
	//std::cout<<"boop"<<std::endl;
	if (max_ind >= heads.size()) {
		heads.resize(max_ind + 1);
		tails.resize(max_ind + 1);
		if (STORE_PARENTS) {
			parent_heads.resize(max_ind + 1);
			parent_tails.resize(max_ind + 1);
		}
	}
	//std::cout<<"boop"<<std::endl;
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
	//std::cout<<"boop parent heads size:"<<parent_heads.size()<<" dst.first "<<dst.first<<std::endl;
	if (STORE_PARENTS) {
		if (isEmpty(parent_heads[dst.first]) ) { // the source for parents is the destination
			Graph<T>::node* new_node_e_par = new Graph<T>::node;
			//std::cout<<"PTR ADDED: "<<new_node_e<<std::endl;
			new_node_e_par->nodeind = dst.first;
			new_node_e_par->dataptr = nullptr; // root head stores node info, no need to store that in parent graph
			new_node_e_par->adjptr = nullptr;
			//std::cout<<"issue here:"<<std::endl;
			parent_heads[dst.first] = new_node_e_par;
			parent_tails[dst.first] = new_node_e_par;
		}

		if (isEmpty(parent_heads[src.first])) { // the destination for parents is the source 
			Graph<T>::node* new_node_e_par = new Graph<T>::node;
			//std::cout<<"PTR ADDED: "<<new_node_e<<std::endl;
			new_node_e_par->nodeind = src.first;
			new_node_e_par->dataptr = nullptr;
			new_node_e_par->adjptr = nullptr;
			parent_heads[src.first] = new_node_e_par;
			parent_tails[src.first] = new_node_e_par;
		}
	}
	Graph<T>::node* new_node = new Graph<T>::node;
	//std::cout<<"PTR ADDED: "<<new_node<<std::endl;
	new_node->nodeind = dst.first;
	new_node->dataptr = dst.second;
	new_node->adjptr = nullptr;	
	// Update the tail pointer to the appended element:
	tails[src.first]->adjptr = new_node;
	tails[src.first] = new_node;
	if (STORE_PARENTS) {
		Graph<T>::node* new_node_par = new Graph<T>::node;
		//std::cout<<"PTR ADDED: "<<new_node<<std::endl;
		new_node_par->nodeind = src.first;
		new_node_par->dataptr = dst.second;
		new_node_par->adjptr = nullptr;	
		// Update the tail pointer to the appended element:
		parent_tails[dst.first]->adjptr = new_node_par;
		std::cout<<"connecting node: "<<dst.first<< " to node: "<<new_node_par->nodeind<<std::endl;
		parent_tails[dst.first] = new_node_par;
	}
	return true;
}

template<class T>
bool Graph<T>::connect(unsigned int src_ind, const std::pair<unsigned int, T*>& dst) {
	unsigned int max_ind = (src_ind >= dst.first) ? src_ind : dst.first;
	if (max_ind >= heads.size()) {
		heads.resize(max_ind + 1);
		tails.resize(max_ind + 1);
		if (STORE_PARENTS) {
			parent_heads.resize(max_ind + 1);
			parent_tails.resize(max_ind + 1);
		}
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
		new_node_e->dataptr = nullptr;
		new_node_e->adjptr = nullptr;
		heads[dst.first] = new_node_e;
		tails[dst.first] = new_node_e;
	}
	if (STORE_PARENTS) {
		if (isEmpty(parent_heads[dst.first])) { // the source for parents is the destination
			Graph<T>::node* new_node_e_par = new Graph<T>::node;
			//std::cout<<"PTR ADDED: "<<new_node_e<<std::endl;
			new_node_e_par->nodeind = dst.first;
			new_node_e_par->dataptr = nullptr;
			new_node_e_par->adjptr = nullptr;
			parent_heads[dst.first] = new_node_e_par;
			parent_tails[dst.first] = new_node_e_par;
		}

		if (isEmpty(parent_heads[src_ind])) { // the destination for parents is the source 
			Graph<T>::node* new_node_e_par = new Graph<T>::node;
			//std::cout<<"PTR ADDED: "<<new_node_e<<std::endl;
			new_node_e_par->nodeind = src_ind;
			new_node_e_par->dataptr = nullptr;
			new_node_e_par->adjptr = nullptr;
			parent_heads[src_ind] = new_node_e_par;
			parent_tails[src_ind] = new_node_e_par;
		}
	}
	Graph<T>::node* new_node = new Graph<T>::node;
	//std::cout<<"PTR ADDED: "<<new_node<<std::endl;
	new_node->nodeind = dst.first;
	new_node->dataptr = dst.second;
	new_node->adjptr = nullptr;	
	// Update the tail pointer to the appended element:
	tails[src_ind]->adjptr = new_node;
	tails[src_ind] = new_node;
	if (STORE_PARENTS) {
		Graph<T>::node* new_node_par = new Graph<T>::node;
		//std::cout<<"PTR ADDED: "<<new_node<<std::endl;
		new_node_par->nodeind = src_ind;
		new_node_par->dataptr = dst.second;
		new_node_par->adjptr = nullptr;	
		// Update the tail pointer to the appended element:
		parent_tails[dst.first]->adjptr = new_node_par;
		parent_tails[dst.first] = new_node_par;
	}
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
template<typename LAM>
bool Graph<T>::parentHop(unsigned int ind, LAM lambda) {
	node* prevptr = parent_heads[ind];
	//std::cout<<"prevptr: "<<prevptr<<std::endl;
	node* currptr = parent_heads[ind]->adjptr; 
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

// This hop function is used for searching with a break statement
//template<typename LAM>
//bool Graph<T>::hopS(unsigned int ind, LAM lambda) {
template<class T> 
bool Graph<T>::hopS(unsigned int ind, std::function<bool(node*, node*)> lambda) const {
	node* prevptr = heads[ind];
	//std::cout<<"prevptr: "<<prevptr<<std::endl;
	node* currptr = heads[ind]->adjptr; 
	//std::cout<<"currptr: "<<currptr<<std::endl;
	bool breakout;
	while (currptr!=nullptr) {
		node* nextptr = currptr->adjptr;
		//std::cout<<"   currptr going in: "<<currptr<<", prevptr going in: "<<prevptr<<std::endl;
		breakout = lambda(currptr, prevptr);
		//std::cout<<"   currptr coming out: "<<currptr<<", prevptr coming out: "<<prevptr<<std::endl;
		if (breakout) {
			return true;
		}
		prevptr = currptr;
		currptr = nextptr;
	}
	return false;
}

template<class T> 
bool Graph<T>::parentHopS(unsigned int ind, std::function<bool(node*, node*)> lambda) const {
	node* prevptr = parent_heads[ind];
	//std::cout<<"prevptr: "<<prevptr<<std::endl;
	node* currptr = parent_heads[ind]->adjptr; 
	//std::cout<<"currptr: "<<currptr<<std::endl;
	bool breakout;
	while (currptr!=nullptr) {
		node* nextptr = currptr->adjptr;
		//std::cout<<"   currptr going in: "<<currptr<<", prevptr going in: "<<prevptr<<std::endl;
		breakout = lambda(currptr, prevptr);
		//std::cout<<"   currptr coming out: "<<currptr<<", prevptr coming out: "<<prevptr<<std::endl;
		if (breakout) {
			return true;
		}
		prevptr = currptr;
		currptr = nextptr;
	}
	return false;
}

// Use for regular hop outside of class, apparently better runtime performance
template<class T>
bool Graph<T>::hopF(unsigned int ind, std::function<bool(node*, node*)> lambda) {
	node* prevptr = heads[ind];
	//std::cout<<"prevptr: "<<prevptr<<std::endl;
	node* currptr = heads[ind]->adjptr; 
	//std::cout<<"currptr: "<<currptr<<std::endl;
	bool ret;
	while (currptr!=nullptr) {
		node* nextptr = currptr->adjptr;
		//std::cout<<"   currptr going in: "<<currptr<<", prevptr going in: "<<prevptr<<std::endl;
		ret = lambda(currptr, prevptr);
		//std::cout<<"   currptr coming out: "<<currptr<<", prevptr coming out: "<<prevptr<<std::endl;
		prevptr = currptr;
		currptr = nextptr;
	}
	return ret;
}

template<class T>
int Graph<T>::size() const {
	return heads.size();
}

template<class T>
void Graph<T>::remove(unsigned int ind_) {
	if (!STORE_PARENTS) {
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
		if (!isEmpty(heads[i])){
			//std::cout<<"CURRENT NODE IND: "<<i<<std::endl;
			if (i == ind_) {
				//std::cout<<"DELETING ENTIRE LIST"<<std::endl;
				hop(i, deleteLAM);	
				if (!isEmpty(heads[i])) {
					//std::cout<<"Deleting: "<<heads[i]<<std::endl;
					delete heads[i];
					heads[i] = nullptr;
				}
			} else {
				//std::cout<<"SEARCHING FOR ELEMENT"<<std::endl;
				hop(i, rmLAM);
			}
		}
	}
	} else {
		std::cout<<"Error: Graph with stored parents does not currently have working functionality with remove()\n";
	}
}

template<class T>
T* Graph<T>::getNodeDataptr(unsigned int ind_) const {
	return heads[ind_]->dataptr;
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
void Graph<T>::getParentNodes(unsigned int ind, std::vector<int>& node_list) {
	if (STORE_PARENTS) {
		node_list.clear();
		auto fillNodesLAM = [&node_list](Graph<T>::node* dst, Graph<T>::node* prv){node_list.push_back(dst->nodeind);};
		parentHop(ind, fillNodesLAM);
	} else {
		std::cout<<"Error: Cannot retrieve parent nodes, graph does not store parent info\n";
	}
}

template<class T>
void Graph<T>::getParentData(unsigned int ind, std::vector<T*>& data_list) {
	if (STORE_PARENTS) {
		data_list.clear();
		auto fillDataLAM = [&data_list](Graph<T>::node* dst, Graph<T>::node* prv){data_list.push_back(dst->dataptr);};
		parentHop(ind, fillDataLAM);
	} else {
		std::cout<<"Error: Cannot retrieve parent data, graph does not store parent info\n";
	}
}

template<class T>
const std::vector<typename Graph<T>::node*>* Graph<T>::getHeads() const {
	return &heads;
}

template<class T>
template<typename pLAM>
void Graph<T>::print(pLAM printLambda) {
	//auto printLAM = [](Graph<T>::node* dst, Graph<T>::node* prv){std::cout<<"   ~> "<<dst->nodeind<<"\n";};
	for (int i=0; i<heads.size(); i++) {
		if (!isEmpty(heads[i])) {
			if (!isEmpty(heads[i]->adjptr)) {
				std::cout<<"Node: "<<i<<" is connected to:\n";
				hop(i, printLambda);
			} else {
				std::cout<<"Node: "<<i<<" has no outgoing transitions.\n";
			}
		}
	}
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
void Graph<T>::printReverse() {
	if (STORE_PARENTS) {
		auto printLAM = [](Graph<T>::node* dst, Graph<T>::node* prv){std::cout<<"   ~> "<<dst->nodeind<<"\n";};
		for (int i=0; i<parent_heads.size(); i++) {
			if (!isEmpty(parent_heads[i])) {
				if (!isEmpty(parent_heads[i]->adjptr)) {
					std::cout<<"Node: "<<i<<" has parents to:\n";
					parentHop(i, printLAM);
				} else {
					std::cout<<"Node: "<<i<<" has no parents.\n";
				}
			}
		}
	} else {
		std::cout<<"Error: Cannot print reverse graph, graph is not reversible\n";
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
		if (!isEmpty(parent_heads[i]) && STORE_PARENTS) {
			parentHop(i, deleteLAM);
			delete parent_heads[i];
		}
	}
	heads.clear();
	tails.clear();
}



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
		if (STORE_PARENTS) {
			if (!isEmpty(parent_heads[i])) {
				parentHop(i, deleteLAM);
				delete parent_heads[i];
			}
		}
	}
	std::cout<<"made it out of destructor"<<std::endl;
}

template class Graph<int>;
template class Graph<unsigned int>;
template class Graph<float>;
template class Graph<double>;
template class Graph<std::string>;
template class Graph<WL>;
template class Graph<WLI>;
template class Graph<WIV>;
template class Graph<IVFlexLex<LexSet>>;
template class Graph<IVFlexLex<FlexLexSetS>>;
template class Graph<IVFlexLex<REQLex>>;
template class Graph<std::vector<unsigned int>>;
template class Graph<State>;
template class Graph<BlockingState>;




		///////////////////////
		/* AUTOMATON CLASSES */
		///////////////////////




template<class T>
void Automaton<T>::addWord(const std::string& word) {
	if (!word.empty()){
		bool not_found = true;
		for (auto test_word : alphabet) {
			if (word == test_word) {
				not_found = false;
				break;
			}
		}
		if (not_found) {
			alphabet.push_back(word);
		}
	}
}

template<class T>
void Automaton<T>::addAP(const std::string& ap) {
	if (!ap.empty()){
		bool not_found = true;
		for (auto test_ap : AP) {
			if (ap == test_ap) {
				not_found = false;
				break;
			}
		}
		//std::cout<<"did i find? "<<not_found<<std::endl;
		if (not_found) {
			AP.push_back(ap);
		}
	}
}


template<class T>
void Automaton<T>::addAcceptingState(unsigned int accepting_state) {
	//std::cout<<"accepting_state: "<<accepting_state<<std::endl;
	//std::cout<<"graph size: "<<Graph<T>::size()<<std::endl;
	if (accepting_state > max_accepting_state_index) {
		max_accepting_state_index = accepting_state;	
	}
	//std::cout<<"b4 is_accepting resize"<<std::endl;
	//std::cout<<" graph size: "<<Graph<T>::size()<<" max accepting state ind: "<<max_accepting_state_index<<std::endl;
	//std::cout<<"first condition: "<<(Graph<T>::size()-1 <= max_accepting_state_index)<<std::endl;
	//std::cout<<"second condition: "<<(max_accepting_state_index > (is_accepting.size()-1))<<std::endl;
	//std::cout<<"is accepting size"<<is_accepting.size()-1<<std::endl;
	if (Graph<T>::size() > (max_accepting_state_index + 1) && (max_accepting_state_index + 1) > is_accepting.size()) {
		is_accepting.resize(Graph<T>::size());
	} else if (Graph<T>::size() <= (max_accepting_state_index + 1) && (max_accepting_state_index + 1) > is_accepting.size()) {
		is_accepting.resize(max_accepting_state_index + 1);
	}
	//std::cout<<"af is_accepting resize, size:"<<is_accepting.size()<<" accepting_state: "<<accepting_state<<std::endl;
	//std::cout<<"found accepting state: "<<accepting_state<<std::endl;
	is_accepting[accepting_state] = true;
	//std::cout<<"PRINTING IS ACCEPTING IN addAccept:"<<std::endl;
	//for (int i=0; i<is_accepting.size(); ++i) {
	//	std::cout<<"i: "<<i<<", is_accepting[i]: "<<is_accepting[i]<<std::endl;
	//}
	//std::cout<<"af is_accepting index"<<std::endl;

}

template<class T>
Automaton<T>::Automaton(bool reversible) : Graph<T>(true, reversible), max_accepting_state_index(0), max_init_state_index(0), is_accepting(0, false)
{
	//node_data_list = new std::vector<T>;	
}

template<class T>
bool Automaton<T>::isAutomatonValid() {
	bool valid = true;
	int graph_size = Graph<T>::size();
	if (max_accepting_state_index > graph_size || max_init_state_index > graph_size) {
		valid = false;	
	}
	return valid;
}

template<class T>
bool Automaton<T>::inAlphabet(std::string s) {
	for (int i=0; i<alphabet.size(); ++i) {
		if (s == alphabet[i]) {
			return true;
			break;
		}
	}	
	return false;
}

template<class T>
bool Automaton<T>::checkAlphabet(const Automaton* arg_dfa) {
	for (int i=0; i<arg_dfa->alphabet.size(); ++i) {
		if (!inAlphabet(arg_dfa->alphabet[i])) {
			return false;
		}
	}
	return true;
}



template<class T>
void Automaton<T>::setAcceptingStates(const std::vector<unsigned int>& accepting_states_){
	// Determine the max accepting state index to easily verify that all
	// accepting states exist in the graph

	//is_accepting.resize(Graph<T>::size());
	accepting_states = accepting_states_;
	for (int i=0; i<accepting_states.size(); ++i) {
		if (accepting_states[i] > max_accepting_state_index) {
			max_accepting_state_index = accepting_states[i];	
		}
	}	
	is_accepting.resize(max_accepting_state_index);
	for (int i=0; i<accepting_states.size(); ++i) {
		is_accepting[accepting_states[i]] = true;
	}
}

template<class T>
bool Automaton<T>::isAccepting(unsigned int ind) const {
	// This function is only called once the graph has been constructed,
	// resize when this function is first called after constructing the 
	// graph to make sure all graph inds have an index in is_accepting
	
	//std::cout<<"isAccepting input: "<<ind<<std::endl;
	if (is_accepting.size() < ind + 1) {
		return false;
	}
	//std::cout<<"PRINTING IS ACCEPTING:"<<std::endl;
	//for (int i=0; i<is_accepting.size(); ++i) {
	//	std::cout<<"i: "<<i<<", is_accepting[i]: "<<is_accepting[i]<<std::endl;
	//}
	//std::cout<<"isAccepting output: "<<is_accepting[ind]<<std::endl;
	return is_accepting[ind];
}

template<class T>
const std::vector<unsigned int>* Automaton<T>::getAcceptingStates() const {
	return &accepting_states;
}

template<class T>
void Automaton<T>::setInitStates(const std::vector<unsigned int>& init_states_){
	// Determine the max init state index to easily verify that all
	// init states exist in the graph
	//std::cout<<"b4 vec eq and size:"<<init_states.size()<<std::endl;
	init_states = init_states_;
	//std::cout<<"input init states size: "<<init_states.size()<<std::endl;
	for (int i=0; i<init_states.size(); ++i) {
		//init_states[i] = init_states_[i];
		if (init_states[i] > max_init_state_index) {
			max_init_state_index = init_states[i];	
		}
	}	
}

template<class T>
const std::vector<unsigned int>* Automaton<T>::getInitStates() const {
	return &init_states;
}

template<class T>
void Automaton<T>::setAlphabet(const std::vector<std::string>& alphabet_) {
	alphabet = alphabet_;
}

template<class T>
const std::vector<std::string>* Automaton<T>::getAlphabet() const {
	return &alphabet;
}

template<class T>
void Automaton<T>::setAP(const std::vector<std::string>& aps) {
	AP = aps;
}

template<class T>
const std::vector<std::string>* Automaton<T>::getAP() const {
	return &AP;
}


template class Automaton<int>;
template class Automaton<unsigned int>;
template class Automaton<float>;
template class Automaton<double>;
template class Automaton<std::string>;
template class Automaton<WL>;
template class Automaton<WLI>;
template class Automaton<State>;
template class Automaton<BlockingState>;




DFA::DFA(bool reversible) : check_det(true), Automaton(reversible) {}

void DFA::toggleCheckDeterminism(bool check_det_) {
	check_det = check_det_;
}

int DFA::getInitState() const {
	//Assume only a single init state:
	return init_states[0];
}

bool DFA::connectDFA(unsigned int ind_from, unsigned int ind_to, const std::string& label_) {
	//Allocate the memory on the heap
	node_data_list.push_back(new std::string(label_));
	if (check_det){
		bool new_state_from, new_state_to;
		new_state_from = (ind_from > size()) ? true : false;
		if (new_state_from) {
			std::vector<std::string*> label_list;
			getConnectedData(ind_from, label_list);
			for (int i=0; i<label_list.size(); ++i) {
				if (label_ == *label_list[i]) {
					std::cout<<"Error: Determinism check failed when connecting state "<<ind_from<<" to "<<ind_to<<" with letter: "<<label_<<"\n";
					return false;
				}
			}
		}
	}
	// If the letter is not seen among all other outgoing edges
	// from the 'from state', the connection is deterministic and
	// the states can be connected. DFA is unweighted by default.

	//std::cout<<"data list"<<std::endl;
	//for (int i=0; i<node_data_list.size(); ++i) {
	//	//std::cout<<(*node_data_list)[i]<<std::endl;
	//	std::cout<<"    ptr: "<<node_data_list[i]<<std::endl;
	//}
	//std::cout<<"connecting: "<<ind_from<<" to: "<<ind_to<<" with ptr: "<<&(node_data_list->back())<<std::endl;
	//std::cout<<"yo b4"<<std::endl;
	Graph::connect({ind_from, nullptr}, {ind_to, node_data_list.back()});
	return true;
}

bool DFA::readFileSingle(const std::string& filename) {
	std::string line;
	std::ifstream dfa_file(filename);
	std::vector<std::string> modes = {"Alphabet", "InitialStates", "Graph", "AcceptingStates"};
	int entry_mode = 0; // First mode (0) dictates start of a new automaton
	if (dfa_file.is_open()) {
		int curr_mode = -1;
		unsigned int source_state, to_state;
		source_state = -1;
		bool seen_entry_mode = false;
		int entry_num = 0;
		while (std::getline(dfa_file, line)) {
			//std::cout<<line<<"\n";	
			// Get the current data type:
			std::string temp_word;
			std::string line_dec; // Line deconstructed into words
			bool is_element = false; // Quickly determine if the line is an element
			for (int i=0; i<line.size(); ++i) {
				switch (line.at(i)) {
					case '-':
						is_element = true;
						break;
					case ':':
					       break;
					case ' ':
					//       if (line_dec.size()==0) {
					//	       line_dec = temp_word;
					//	       temp_word.clear();
					       line_dec = line_dec + temp_word;
					       temp_word.clear();
					       break;
					default:
					       temp_word.push_back(line.at(i));
				}
				if (is_element) {
					break;
				}
				
			}
			if (temp_word.size() != 0) {
				line_dec += temp_word;
			}
			// Cycle through different modes depending on the current data type:
			bool found = false;
			if (!is_element) {
				for (int i=0; i<modes.size(); ++i) {
					if (modes[i] == line_dec) {
						curr_mode = i;
						found = true;
						break;
					}	
				}
				if (!found) {
					curr_mode = -1;
					if (line_dec.size() > 0) {
						std::cout<<"Warning: Unrecognized Automaton field: "<<line_dec<<"\n";
					}
					continue;
				}
			}
			if (!is_element) {
				continue;
			}
			std::string temp_word_2;
			if (curr_mode == entry_mode) {
				if (!seen_entry_mode) {
					seen_entry_mode = true;
					entry_num++;
				} 
				if (entry_num > 1) {
					std::cout<<"Warning: Can only read a single DFA into data structure. Ignoring extra DFA's in file...\n";
					break;
				}
			} else {
				if (seen_entry_mode) {
					seen_entry_mode = false;	
				}
			}
			//std::cout<<"currmode: "<<curr_mode<<std::endl;
			switch (curr_mode) {
				case -1: 
					std::cout<<"Error: No field specified\n";
					break;
				case 0: // ALPHABET
					for (int i=0; i<line.size(); ++i) {
						switch (line.at(i)) {
							case '-':
								break;
							case ' ':
								break;
							default:
								temp_word_2.push_back(line[i]);
						}
					}
					std::cout<<"adding temp word: "<<temp_word_2<<std::endl;
					addAP(temp_word_2);
					break;
				case 1: // INIT STATES
					for (int i=0; i<line.size(); ++i) {
						switch (line.at(i)) {
							case '-':
								break;
							case ' ':
								break;
							default:
								temp_word_2.push_back(line[i]);
						}	
					}
					{
						std::string::size_type sz;
						int init_state = std::stoi(temp_word_2,&sz);
						// Only include functionality for 1 initial state
						std::vector<unsigned int> init_states(1);
						init_states[0] = init_state;
						//std::cout<<"b4 setinitstates init_state value: "<<init_states[0]<<std::endl;
						setInitStates(init_states);
						//std::cout<<"af setinitstates"<<std::endl;
					}
					break;
				case 2: // GRAPH
					{
						bool is_label = false;
						bool is_source_state = true;
						std::string temp_label;
						for (int i=0; i<line.size(); ++i) {
							switch (line[i]) {
								case '-':
									break;
								case ' ':
									break;
								case '>':
									temp_word_2.clear(); //disregard anything before the arrow
									is_source_state = false;
									break;
								case ':':
									is_label = true;
									break;
								default:
									if (is_label) {
										temp_label.push_back(line[i]);
									} else {
										temp_word_2.push_back(line[i]);
									}
							}	
						}
						if (is_source_state) {
							{
								std::string::size_type sz;
								source_state = std::stoi(temp_word_2,&sz);
							}
						} else {
							{
								std::string::size_type sz;
								to_state = std::stoi(temp_word_2,&sz);
								//std::cout<<"b4 connectdfa"<<std::endl;
								connectDFA(source_state, to_state, temp_label);
								//std::cout<<"af connectdfa"<<std::endl;
								addWord(temp_label);
							}
						}
					}
					break;
				case 3: // ACCEPTING STATES
					//std::cout<<"in accepting states"<<std::endl;
					for (int i=0; i<line.size(); ++i) {
						switch (line[i]) {
							case '-':
								break;
							case ' ':
								break;
							default:
								temp_word_2.push_back(line[i]);
						}	
					}
					{
						std::string::size_type sz;
						int accepting_state = std::stoi(temp_word_2,&sz);
						//std::cout<<"b4 add accept state: "<<accepting_state<<std::endl;
						addAcceptingState(accepting_state);
					}
			}
				//std::cout<<"af currmode"<<std::endl;
			}	
			//std::cout<<"made it out of while"<<std::endl;
		dfa_file.close();
		return true;
	} else {
		std::cout<<"Cannot open file: "<<filename<<"\n";
		return false;
	}
}

void DFA::print() {
	
	auto printLAM = [](Graph<std::string>::node* dst, Graph<std::string>::node* prv){std::cout<<"   ~> "<<dst->nodeind<<" : "<<*(dst->dataptr)<<"\n";};

	//std::cout<<"Alphabet Size: "<<alphabet.size()<<std::endl;
	std::cout<<"Atomic Propositions: ";
	for (int i=0; i<AP.size(); ++i) {
		std::cout<<AP[i]<<" ";
	}
	std::cout<<"\nAlphabet: ";
	for (int i=0; i<alphabet.size(); ++i) {
		std::cout<<alphabet[i]<<" ";
	}
	std::cout<<"\nInitial State: ";
	std::cout<<init_states[0];
	//std::cout<<"Accepting states Size: "<<accepting_states.size()<<std::endl;
	std::cout<<"\nAccepting States: ";
	for (int i=0; i<is_accepting.size(); ++i) {
		//std::cout<<"i: "<<i<<std::endl;
		if (is_accepting[i]) {
			std::cout<<i<<" ";
		}
	}
	std::cout<<"\n";
	Graph<std::string>::print(printLAM);

	//for (int i=0; i<heads.size(); i++) {
	//	if (!isEmpty(heads[i])) {
	//		if (!isEmpty(heads[i]->adjptr)) {
	//			std::cout<<"Node: "<<i<<" is connected to:\n";
	//			Graph<std::string>::hop(i, printLAM);
	//		} else {
	//			std::cout<<"Node: "<<i<<" has no outgoing transitions.\n";
	//		}
	//	}
	//}
}

DFA::~DFA() {
	std::cout<<"Deleting node data...\n";
	for (int i=0; i<node_data_list.size(); ++i) {
		//std::cout<<"DELETING: "<<node_data_list[i]<<std::endl;
		delete node_data_list[i];
	}
}







		////////////////////////
		/* GRAPH EVAL CLASSES */
		////////////////////////



DFA_EVAL::DFA_EVAL(DFA* dfaptr_) : dfaptr(dfaptr_), accepting(false) {
//DFA_EVAL::DFA_EVAL(const DFA* dfaptr_) : dfaptr(dfaptr_), accepting(false) {
	// Set the current node to be the initial state
	curr_node = dfaptr->getInitState();
}

const DFA* DFA_EVAL::getDFA() const {
	return dfaptr;
}

const std::vector<std::string>* DFA_EVAL::getAlphabetEVAL() const {
	return dfaptr->getAlphabet();
}

bool DFA_EVAL::eval(const std::string& letter, bool evolve) {
	int curr_node_g = curr_node;
	auto evalLAM = [&curr_node_g, &letter](Graph<std::string>::node* dst, Graph<std::string>::node* prv){
	//auto evalLAM = [&curr_node_g, &letter](Graph<std::string>::node* dst, Graph<std::string>::node* prv){
		if (*(dst->dataptr) == letter || *(dst->dataptr) == "1") {
			curr_node_g = dst->nodeind;
			return true;
		} else {
			return false;
		}
	};
	//Graph<std::string>* grphptr = &dfaptr;
	if (dfaptr->hopS(curr_node, evalLAM)) {
		//std::cout<<"new curr_node:"<<curr_node<<std::endl;
		if (dfaptr->isAccepting(curr_node_g)) {
			accepting = true;
		}
		if (evolve) {
			curr_node = curr_node_g;
		}
		return true;
	} else {
		//std::cout<<"Error: Letter ("<<letter<<") not found at state: "<<curr_node<<std::endl;
		return false;
	}
}

bool DFA_EVAL::evalReverse(const std::string& letter, bool evolve) {
	int curr_node_g = curr_node;
	bool found_true = false;
	//std::cout<<"INPUT curr_node:"<<curr_node<<" WITH LABEL: "<<letter<<std::endl;
	auto evalLAM = [&curr_node_g, &letter, &found_true](Graph<std::string>::node* dst, Graph<std::string>::node* prv){
	//auto evalLAM = [&curr_node_g, &letter](Graph<std::string>::node* dst, Graph<std::string>::node* prv){
		//std::cout<<" hopping parent node: "<<dst->nodeind<<std::endl;
		//std::cout<<" hopping parent label: "<<*(dst->dataptr)<<std::endl;
		if (*(dst->dataptr) == letter) {
			curr_node_g = dst->nodeind;
			//std::cout<<"returning true"<<std::endl;
			return true;
		} else if (*(dst->dataptr) == "1") {
			found_true = true;
			//std::cout<<"returning false"<<std::endl;
			return false;
		} else {
			//std::cout<<"returning false"<<std::endl;
			return false;
		}
	};
	//Graph<std::string>* grphptr = &dfaptr;
	//std::vector<int> parent_nodes;
	//dfaptr->getParentNodes(curr_node, parent_nodes);
	//for (int i=0; i<parent_nodes.size(); ++i) {
	//	std::cout<<"           parent node: "<<parent_nodes[i]<<std::endl;
	//}
	if (dfaptr->parentHopS(curr_node, evalLAM)) {
		//std::cout<<"new curr_node:"<<curr_node<<std::endl;
		if (dfaptr->isAccepting(curr_node_g)) {
			accepting = true;
		}
		if (evolve) {
			curr_node = curr_node_g;
		}
		return true;
	} else {
		// If "true" was found, but no other matching letter was found, accept it as found
		if (found_true) {
			//return true;
		}
		//std::cout<<"Error: Letter ("<<letter<<") not found at state: "<<curr_node<<std::endl;
		return false;
	}
}

int DFA_EVAL::getCurrNode() const {
	return curr_node;
}

void DFA_EVAL::set(int set_node) {
	curr_node = set_node;
	accepting = dfaptr->isAccepting(set_node);
}

void DFA_EVAL::reset() {
	curr_node = dfaptr->getInitState();
	accepting = false;
}

bool DFA_EVAL::isCurrAccepting() const {
	//dfaptr->print();
	return dfaptr->isAccepting(curr_node);
}



//bool DFA::syncProduct(const DFA* dfa1, const DFA* dfa2, DFA* product) {
//	if (!dfa1->checkAlphabet(dfa2)) {
//		std::cout<<"Warning: Alphabet of dfa1 does not match that of dfa2\n";
//		return false;
//	}
//	int n = dfa1->size();
//	int m = dfa2->size();
//	auto heads_dfa1 = dfa1->getHeads();
//	auto heads_dfa2 = dfa2->getHeads();
//	int init_state_dfa1 = dfa1->getInitState();
//	int init_state_dfa2 = dfa2->getInitState();
//	int p_i = 0;
//	// THIS IS UNFINISHED
//}

/*
bool DFA::readFileMultiple(const std::string& filename, std::array<DFA, int>& dfa_arr) {
	std::string line;
	std::ifstream dfa_file(filename);
	std::vector<std::string> modes = {"Alphabet", "InitialStates", "Graph", "AcceptingStates"};
	int entry_mode = 0; // First mode (0) dictates start of a new automaton
	if (dfa_file.is_open()) {
		int curr_mode = -1;
		unsigned int source_state, to_state;
		source_state = -1;
		int dfa_counter = -1;
		bool seen_entry_mode = false;
		while (std::getline(dfa_file, line)) {
			std::cout<<line<<"\n";	
			// Get the current data type:
			std::string temp_word;
			std::string line_dec; // Line deconstructed into words
			bool is_element = false; // Quickly determine if the line is an element
			for (int i=0; i<line.size(); ++i) {
				switch (line.at(i)) {
					case '-':
						is_element = true;
						break;
					case ':':
					       break;
					case ' ':
					//       if (line_dec.size()==0) {
					//	       line_dec = temp_word;
					//	       temp_word.clear();
					       line_dec = line_dec + temp_word;
					       temp_word.clear();
					       break;
					default:
					       temp_word.push_back(line.at(i));
				}
				if (is_element) {
					break;
				}
				
			}
			if (temp_word.size() != 0) {
				line_dec += temp_word;
			}
			// Cycle through different modes depending on the current data type:
			bool found = false;
			if (!is_element) {
				for (int i=0; i<modes.size(); ++i) {
					if (modes[i] == line_dec) {
						curr_mode = i;
						found = true;
						break;
					}	
				}
				if (!found) {
					curr_mode = -1;
					if (line_dec.size() > 0) {
						std::cout<<"Warning: Unrecognized Automaton field: "<<line_dec<<"\n";
					}
					continue;
				}
			}
			if (!is_element) {
				continue;
			}
			if (curr_mode == entry_mode) {
				if (!seen_entry_mode) {
					dfa_counter++;
					if (dfa_counter > dfa_arr.size()) {
						std::cout<<"Error: More automata in file than input array size.\n";
					}
					seen_entry_mode = true;
				}
			} else {
				if (seen_entry_mode) {
					seen_entry_mode = false;	
				}
			}
			std::string temp_word_2;
			switch (curr_mode) {
				case -1: 
					std::cout<<"Error: No field specified\n";
					break;
				case 0:
					for (int i=0; i<line.size(); ++i) {
						switch (line.at(i)) {
							case '-':
								break;
							case ' ':
								break;
							default:
								temp_word_2.push_back(line[i]);
						}
					}
					//std::cout<<"adding to dfa: "<<dfa_counter<<std::endl;
					dfa_arr[dfa_counter].addWord(temp_word);
					break;
				case 1:
					for (int i=0; i<line.size(); ++i) {
						switch (line.at(i)) {
							case '-':
								break;
							case ' ':
								break;
							default:
								temp_word_2.push_back(line[i]);
						}	
					}
					{
						std::string::size_type sz;
						int init_state = std::stoi(temp_word_2,&sz);
						// Only include functionality for 1 initial state
						std::vector<unsigned int> init_states(1);
						init_states[0] = init_state;
						//std::cout<<"adding to dfa: "<<dfa_counter<<std::endl;
						dfa_arr[dfa_counter].setInitStates(init_states);
					}
					break;
				case 2:
					{
						bool is_label = false;
						bool is_source_state = true;
						std::string temp_label;
						for (int i=0; i<line.size(); ++i) {
							switch (line[i]) {
								case '-':
									break;
								case ' ':
									break;
								case '>':
									temp_word_2.clear(); //disregard anything before the arrow
									is_source_state = false;
									break;
								case ':':
									is_label = true;
									break;
								default:
									if (is_label) {
										temp_label.push_back(line[i]);
									} else {
										temp_word_2.push_back(line[i]);
									}
							}	
						}
						if (is_source_state) {
							{
								std::string::size_type sz;
								source_state = std::stoi(temp_word_2,&sz);
							}
						} else {
							{
								std::string::size_type sz;
								to_state = std::stoi(temp_word_2,&sz);
								//std::cout<<"adding to dfa: "<<dfa_counter<<std::endl;
								dfa_arr[dfa_counter].connect(source_state, to_state, 0, temp_label);
							}
						}
					}
					break;
				case 3:
					for (int i=0; i<line.size(); ++i) {
						switch (line[i]) {
							case '-':
								break;
							case ' ':
								break;
							default:
								temp_word_2.push_back(line[i]);
						}	
					}
					{
						std::string::size_type sz;
						int accepting_state = std::stoi(temp_word_2,&sz);
						dfa_arr[dfa_counter].addAcceptingState(accepting_state);
					}
			}

		}	
		dfa_file.close();
		return true;
	} else {
		std::cout<<"Cannot open file: "<<filename<<"\n";
		return false;
	}
}
*/
