#include<string>
#include<vector>
#include<array>
#include<iostream>
#include "transitionSystem.h"

template <class T>
TransitionSystem<T>::TransitionSystem (Graph<WL>* graph_TS_) : DETERMINISTIC(true), manual(false), has_conditions(false), generated(false) {
	// Transition system uses WL structure: 
	// W: edge weight (action cost)
	// L: action
	if (graph_TS_->isOrdered()) {
		graph_TS = graph_TS_;	
	} else {
		std::cout<<"Error: Transition System must take in an ordered graph\n";
	}
	conditions.clear();
}

template <class T>
TransitionSystem<T>::TransitionSystem (Graph<WL>* graph_TS_, bool DETERMINISTIC_, bool manual_) : DETERMINISTIC(DETERMINISTIC_), manual(manual_), has_conditions(false), generated(false) {
	// Transition system uses WL structure: 
	// W: edge weight (action cost)
	// L: action
	if (graph_TS_->isOrdered()) {
		graph_TS = graph_TS_;	
	} else {
		std::cout<<"Error: Transition System must take in an ordered graph\n";
	}
	conditions.clear();
}

template <class T>
unsigned int TransitionSystem<T>::size() const {
	return graph_TS->size();
}

// This is part of the "Labeling Function"
template <class T>
bool TransitionSystem<T>::parseLabelAndEval(const std::string* label, const T* state) {
	//std::cout<<"received label: "<<*(label)<<std::endl;
	if (*label == "1") {
		return true;
	}
	bool negate_next = false;
	bool is_first = true;
	bool arg;
	//bool arg_conj;
	//std::vector<bool> propositions;
	int prop_i = -1;
	std::string temp_name;
	char prev_operator = '\0';
	for (int i=0; i<label->size(); ++i) {
		char character = (*label)[i];
		bool sub_eval;
		switch (character) {
			case '!':
				negate_next = !negate_next;
				break;
			case '&':
				sub_eval = TransitionSystem<T>::propositions[temp_name]->evaluate(state);	
				if (negate_next) {
					sub_eval = !sub_eval;
				}
				temp_name.clear();
				negate_next = false;
				prev_operator = '&';
				if (is_first) {
					arg = sub_eval;
					is_first = false;
				} else {
					arg = arg && sub_eval;
				}
				break;
			case '|':
				sub_eval = TransitionSystem<T>::propositions[temp_name]->evaluate(state);	
				
				if (negate_next) {
					sub_eval = !sub_eval;
				}
				temp_name.clear();
				negate_next = false;
				prev_operator = '|';
				if (is_first) {
					arg = sub_eval;
					is_first = false;
				} else {
					arg = arg || sub_eval;
				}
				break;
			case ' ':
				break;
			default:
				temp_name.push_back(character);
				if (i == label->size()-1) {
					sub_eval = TransitionSystem<T>::propositions[temp_name]->evaluate(state);	
					if (negate_next) {
						sub_eval = !sub_eval;
					}
					switch (prev_operator) {
						//case '&':	
						case '|':	
							arg = arg || sub_eval;
							break;
						default:
							// Defaults to conjunction because
							// arg is initialized to 'true'
							if (is_first) {
								arg = sub_eval;
								is_first = false;
							} else {
								arg = arg && sub_eval;
							}
					}
				}
		}
		/*
		if (!arg) {
			break;
		}
		*/
	}
	return arg;
	
}

template <class T>
void TransitionSystem<T>::addCondition(Condition* condition_){
	conditions.push_back(condition_);
	has_conditions = true;
}

template <class T>
void TransitionSystem<T>::setConditions(const std::vector<Condition*>& conditions_) {
	conditions = conditions_;
	has_conditions = true;
}

template <class T>
void TransitionSystem<T>::addProposition(SimpleCondition* proposition_) {
	if (proposition_->getLabel() != Condition::FILLER) {
		propositions[proposition_->getLabel()] = proposition_;
	} else {
		std::cout<<"Error: Must name proposition before including in Product System\n";
	}
}

template <class T>
void TransitionSystem<T>::setPropositions(const std::vector<SimpleCondition*>& propositions_) {
	for (int i=0; i<propositions_.size(); ++i) {
		if (propositions_[i]->getLabel() != Condition::FILLER) {
			//std::cout<<"Info: Found proposition: "<<propositions_[i]->getLabel()<<std::endl;
			propositions[propositions_[i]->getLabel()] = propositions_[i];
		} else {
			std::cout<<"Error: Must name all propositions before including in Product System\n";
		}
	}
}

template <class T>
void TransitionSystem<T>::setInitState(T* init_state_) {
	init_state = init_state_;
	has_init_state = true;
	all_states.clear();
	if (manual) {
		all_states.push_back(*init_state);
		state_map.push_back(init_state);
	} else {
		init_state->generateAllPossibleStates(all_states);
		state_added.resize(all_states.size());
		for (int i=0; i<state_added.size(); ++i) {
			state_added[i] = false;
		}
	}
}

template <class T>
void TransitionSystem<T>::safeAddState(int q_i, T* add_state, int add_state_ind, Condition* cond){
	std::string action = cond->getActionLabel();
	float action_cost = cond->getActionCost();
	if (!state_added[add_state_ind]) {
		state_map.push_back(add_state);
		state_added[add_state_ind] = true;
		unsigned int new_ind = state_map.size()-1;
		//graph_TS->Graph<WL>::connect(q_i, new_ind, 1.0f, action);
		
		// New state structure: 
		WL* wl_struct = new WL;
		node_container.push_back(wl_struct);
		wl_struct->weight = action_cost;
		//wl_struct->label = action;
		if (DETERMINISTIC) {
			wl_struct->label = action + "_" + std::to_string(q_i) + "_" + std::to_string(new_ind);
		} else {
			wl_struct->label = action;
		}
		graph_TS->Graph<WL>::connect(q_i, {new_ind, wl_struct});
	} else {
		for (int i=0; i<state_map.size(); ++i) {
			if (add_state == state_map[i]) {
				// New state structure:
				WL* wl_struct = new WL;
				node_container.push_back(wl_struct);
				wl_struct->weight = action_cost;
				//std::cout<<"action_cost: "<<action_cost<<std::endl;
				if (DETERMINISTIC) {
					wl_struct->label = action + "_" + std::to_string(q_i) + "_" + std::to_string(i);
				} else {
					wl_struct->label = action;
				}
				graph_TS->Graph<WL>::connect(q_i, {i, wl_struct});

				//graph_TS->Edge::connect(q_i, i, 1.0f, action);				
			}
		}
	}
}

template <class T>
bool TransitionSystem<T>::connect(T* src, T* dst, float action_cost, const std::string& action) {
	if (manual) {
		if (state_map.size() != 0) {
			state_map.clear();
		}
		bool src_found, dst_found;
		int src_ind, dst_ind;
		src_found = false;
		dst_found = false;
		for (int i=0; i<all_states.size(); ++i) {
			//std::cout<<"printing state map element i: "<<i<<std::endl;
			//all_states[i]->print();
			if (*src == all_states[i]) {
				src_found = true;
				src_ind = i;
			}
			if (*dst == all_states[i]) {
				dst_found = true;
				dst_ind = i;
			}
			if (src_found && dst_found) {
				break;
			}
		}
		//std::pair<int, WL*> src_node;
		std::pair<int, WL*> dst_node;
		if (!src_found) {
			all_states.push_back(*src);
			//state_map.push_back(&all_states.back());
			src_ind = all_states.size() -1;
		}
		if (!dst_found) {
			all_states.push_back(*dst);
			//state_map.push_back(&all_states.back());
			dst_ind = all_states.size() -1;
		}
		WL* wl_struct = new WL;
		node_container.push_back(wl_struct);
		wl_struct->weight = action_cost;
		//std::cout<<"action_cost: "<<action_cost<<std::endl;
		if (DETERMINISTIC) {
			wl_struct->label = action + "_" + std::to_string(src_ind) + "_" + std::to_string(dst_ind);
		} else {
			wl_struct->label = action;
		}
		dst_node.first = dst_ind;
		dst_node.second = wl_struct;
		//std::cout<<"connecting: "<<src_ind<<" to: "<<dst_node.first<<std::endl;
		graph_TS->Graph<WL>::connect(src_ind, dst_node);
		generated = true;
		return true;
	} else {
		std::cout<<"Error: Cannot use connect() when not in manual mode\n";
		return false;
	}
}

template <class T> // Get rid of this method someday by removing 'state_map'
void TransitionSystem<T>::finishConnecting() {
	state_map.clear();
	state_map.resize(all_states.size());
	for (int i=0; i<all_states.size(); ++i) {
		state_map[i] = &all_states[i];
	}
}


template <class T>
const T* TransitionSystem<T>::getState(int node_index) const {
	return state_map[node_index];
}

template <class T>
void TransitionSystem<T>::generate() {
	if (has_init_state && has_conditions) {
		int state_count = all_states.size();
		int cond_count = conditions.size();
		/*
		   for (int i=0; i<all_states.size(); i++) {
		   all_states[i].print();
		   }
		   */
		T* init_state_in_set;
		bool init_state_found = false;
		for (int i=0; i<state_count; ++i) {
			if (all_states[i] == init_state) {
				init_state_in_set = &all_states[i];
				state_added[i] = true;
				init_state_found = true;
			}	
		}
		if (!init_state_found) {
			std::cout<<"Error: Init State not found in "<<all_states.size()<< " generated states\n";
		}

		state_map.clear();
		state_map.push_back(init_state_in_set);

		int q_i = 0; // State index for current state
		while (q_i<state_map.size() && init_state_found) {
			T* curr_state = state_map[q_i];
			for (unsigned int i=0; i<state_count; ++i) {
				T* new_state = &all_states[i];
				if (!(new_state == curr_state)) {
					for (int ii=0; ii<cond_count; ++ii) {
						bool satisfied;
						satisfied = conditions[ii]->evaluate(curr_state, new_state);
						if (satisfied) {
							safeAddState(q_i, new_state, i, conditions[ii]);
						}
					}
				}
			}
			q_i++;
		}
		generated = true;
	} else {
		std::cout<<"Error: Must set init state and conditions before calling generate()\n";
	}
}

template <class T>
void TransitionSystem<T>::clearTS() {
	graph_TS->clear();
	state_map.clear();	
	for (int i=0; i<node_container.size(); ++i) {
		delete node_container[i];
	}	
	generated = false;
}

template <class T>
void TransitionSystem<T>::printTS() const {
	//graph_TS->print();
	if (state_map.size() > 1) {
		for (int i=0; i<state_map.size(); ++i) {
			std::vector<WL*> con_data; 
			std::vector<int> con_nodes; 
			//std::vector<std::string> list_actions; 
			//graph_TS->returnListLabels(i, list_actions);
			graph_TS->getConnectedData(i, con_data);
			graph_TS->getConnectedNodes(i, con_nodes);
			std::cout<<"State "<<i<<": ";
			T* curr_state = state_map[i];
			std::vector<std::string> state_i; 
			curr_state->getState(state_i);
			for (int ii=0; ii<state_i.size(); ++ii) {
				std::cout<<state_i[ii]<<", ";
			}
			std::cout<<"connects to:\n";
			for (int ii=0; ii<con_data.size(); ++ii) {
				T* con_state = state_map[con_nodes[ii]];
				std::cout<<"   ~>State "<<con_nodes[ii]<<": ";
				con_state->getState(state_i);
				for (int iii=0; iii<state_i.size(); ++iii) {
					std::cout<<state_i[iii]<<", ";
				}
				std::cout<<" with action: "<<con_data[ii]->label<<" (cost: "<<con_data[ii]->weight<<")"<<"\n";
			}
		}
	} else {
		std::cout<<"Warning: Transition has not been generated, or has failed to generate. Cannot print\n";
	}
}

template <class T>
TransitionSystem<T>::~TransitionSystem() {
	for (int i=0; i<node_container.size(); ++i) {
		delete node_container[i];
	}	
}

template class TransitionSystem<State>;
template class TransitionSystem<BlockingState>;





		////////////////////////
		/* GRAPH EVAL CLASSES */
		////////////////////////


//template class TS_EVAL<State>;
//template class TS_EVAL<BlockingState>;


template <class T>
TS_EVAL<T>::TS_EVAL(Graph<WL>* graph_TS_, int init_node_) : TransitionSystem<T>(graph_TS_), init_node(init_node_) {
	curr_node = init_node; //ts is generated around 0 being init state
}

template <class T>
TS_EVAL<T>::TS_EVAL(Graph<WL>* graph_TS_, bool DETERMINISTIC_, bool manual_, int init_node_) : TransitionSystem<T>(graph_TS_, DETERMINISTIC_, manual_), init_node(init_node_) {
	curr_node = init_node; //ts is generated around 0 being init state
}

template <class T>
void TS_EVAL<T>::mapStatesToLabels(const std::vector<const std::vector<std::string>*>& alphabet) {
	state_to_label_map.clear();
	//std::cout<<"Info: Mapping states to labels...\n";
	for (int si=0; si<TransitionSystem<T>::state_map.size(); ++si) {
		std::vector<std::string> temp_labels;
		temp_labels.clear();
		for (int i=0; i<alphabet.size(); ++i) {
			for (int ii=0; ii<alphabet[i]->size(); ++ii) {
				bool found = false;
				// Make sure the label is not already in the set to prevent duplicates
				//std::cout<<"b4 check"<<std::endl;
				for (int iii=0; iii<temp_labels.size(); ++iii) {
					if (temp_labels[iii] == alphabet[i]->operator[](ii)) {
						found = true;
						break;
					}
				}
				//std::cout<<"af check, b4 plneval"<<std::endl;
				if (!found) {
					if (TransitionSystem<T>::parseLabelAndEval(&alphabet[i]->operator[](ii), TransitionSystem<T>::state_map[si])) {
						//std::cout<<"state: "<<si<<" satisfies letter: "<<alphabet[i]->operator[](ii)<<std::endl;
						temp_labels.push_back(alphabet[i]->operator[](ii));
					}
				}
				//std::cout<<"af plneval"<<std::endl;
			}
		}
		//std::cout<<"mapping state: "<<si<<" to: "<<std::endl;
		//for (int i=0; i<temp_labels.size(); ++i) {
		//	std::cout<<"  label: "<<temp_labels[i]<<std::endl;
		//}
		state_to_label_map[si] = temp_labels;
	}
	//std::cout<<"Info: Done mapping states to labels.\n";
}

template <class T>
const std::vector<std::string>* TS_EVAL<T>::returnStateLabels(int state_ind) {
	//std::cout<<"\n IN RETURN STATE LABELS"<<std::endl;
	//TransitionSystem<T>::state_map[state_ind]->print();
	//std::cout<<"corresponding labels: "<<std::endl;
	//for (int i=0; i<state_to_label_map[state_ind].size(); ++i) {
	//	std::cout<<state_to_label_map[state_ind][i]<<std::endl;
	//}
	//std::cout<<"RETURN STATE LABELS ind: "<<state_ind<<" size: "<<state_to_label_map[state_ind].size()<<std::endl;

	//	for (int i=0; i<state_to_label_map[state_ind].size(); ++i) {
	//		std::cout<<"  label: "<<state_to_label_map[state_ind][i]<<std::endl;
	//	}
	if (state_ind > TransitionSystem<T>::state_map.size()-1) {
		std::cout<<"Error: State ind map out of bounds\n";
		return nullptr;
	} else {
		return &state_to_label_map[state_ind];
	}
}


template <class T>
bool TS_EVAL<T>::eval(const std::string& action, bool evolve) {
	int curr_node_g = curr_node;
	auto evalLAM = [&curr_node_g, &action](Graph<WL>::node* dst, Graph<WL>::node* prv){
	//auto evalLAM = [&curr_node_g, &letter](Graph<std::string>::node* dst, Graph<std::string>::node* prv){
		if (dst->dataptr->label == action) {
			curr_node_g = dst->nodeind;
			//ret_weight = dst->dataptr->weight;
			return true;
		} else {
			return false;
		}
	};
	if (TransitionSystem<T>::graph_TS->hopS(curr_node, evalLAM)) {
		//std::cout<<"new curr_node:"<<curr_node<<std::endl;
		if (evolve) {
			curr_node = curr_node_g;
		}
		return true;
	} else {
		std::cout<<"Error: Action ("<<action<<") not found at state: "<<curr_node<<std::endl;
		return false;
	}
}

template <class T>
bool TS_EVAL<T>::evalReverse(const std::string& action, bool evolve) {
	int curr_node_g = curr_node;
	auto evalLAM = [&curr_node_g, &action](Graph<WL>::node* dst, Graph<WL>::node* prv){
	//auto evalLAM = [&curr_node_g, &letter](Graph<std::string>::node* dst, Graph<std::string>::node* prv){
		if (dst->dataptr->label == action) {
			curr_node_g = dst->nodeind;
			//ret_weight = dst->dataptr->weight;
			return true;
		} else {
			return false;
		}
	};
	if (TransitionSystem<T>::graph_TS->parentHopS(curr_node, evalLAM)) {
		//std::cout<<"new curr_node:"<<curr_node<<std::endl;
		if (evolve) {
			curr_node = curr_node_g;
		}
		return true;
	} else {
		std::cout<<"Error: Action ("<<action<<") not found at state: "<<curr_node<<std::endl;
		return false;
	}
}


template <class T>
bool TS_EVAL<T>::isReversible() const {
	return TransitionSystem<T>::graph_TS->isReversible();
}

template <class T>
int TS_EVAL<T>::getCurrNode() const {
	return curr_node;
}

template <class T>
void TS_EVAL<T>::getConnectedDataEVAL(std::vector<WL*>& con_data) {
	TransitionSystem<T>::graph_TS->getConnectedData(curr_node, con_data);
}

template <class T>
void TS_EVAL<T>::getConnectedNodesEVAL(std::vector<int>& con_nodes) {
	TransitionSystem<T>::graph_TS->getConnectedNodes(curr_node, con_nodes);
}

template <class T>
void TS_EVAL<T>::getParentDataEVAL(std::vector<WL*>& con_data) {
	TransitionSystem<T>::graph_TS->getParentData(curr_node, con_data);
}

template <class T>
void TS_EVAL<T>::getParentNodesEVAL(std::vector<int>& con_nodes) {
	TransitionSystem<T>::graph_TS->getParentNodes(curr_node, con_nodes);
}

template <class T>
void TS_EVAL<T>::set(int set_node) {
	curr_node = set_node;
}

template <class T>
void TS_EVAL<T>::reset() {
	curr_node = init_node;
}

template <class T>
const T* TS_EVAL<T>::getCurrState() const {
	return TransitionSystem<T>::getState(curr_node);
}

template class TS_EVAL<State>;
template class TS_EVAL<BlockingState>;




template <class T>
ProductSystem<T>::ProductSystem(Graph<WL>* graph_TS_, DFA* graph_DFA_, Graph<WL>* graph_product_) : 
	TransitionSystem<T>(graph_TS_), is_DFA_accepting(0, false) {
		if (graph_product_->isOrdered()) {
			graph_product = graph_product_;
		} else {
			std::cout<<"Error: Product System must return an ordered graph\n";
		}
		if (graph_DFA_->isOrdered()) {
			graph_DFA = graph_DFA_;
		} else {
			std::cout<<"Error: Product System must take in an ordered graph (DFA)\n";
		}
		
	}



//template <class T>
//void ProductSystem<T>::setAutomatonInitStateIndex(int init_state_DFA_ind_) {
//	init_state_DFA_ind = init_state_DFA_ind_;
//	automaton_init = true;
//}

//template <class T>
//void ProductSystem<T>::setAutomatonAcceptingStateIndices(const std::vector<int>& accepting_DFA_states_) {
//	bool in_bounds = true;
//	for (int i=0; i<accepting_DFA_states_.size(); ++i) {
//		if (accepting_DFA_states_[i] > graph_DFA->size()-1) {
//			std::cout<<"Error: All accepting state indices have to appear in automaton";	
//			in_bounds = false;
//		}
//	}
//	if (in_bounds) {
//		accepting_DFA_states = accepting_DFA_states_;
//		for (int ii=0; ii<accepting_DFA_states.size(); ++ii) {
//			is_DFA_accepting[accepting_DFA_states_[ii]] = true;
//		}
//	}
//}
//
//template <class T>
//void ProductSystem<T>::addAutomatonAcceptingStateIndex(int accepting_DFA_state_) {
//	if (accepting_DFA_state_ > graph_DFA->size()-1) {
//		std::cout<<"Error: Accepting state must appear in automaton";	
//	} else {
//		std::cout<<"is_DFA_accepting size:"<<is_DFA_accepting.size()<<std::endl;
//		is_DFA_accepting[accepting_DFA_state_] = true;
//	}
//}




template <class T>
void ProductSystem<T>::safeAddProdState(int p_i, T* add_state, int add_state_ind, float weight, const std::string& action){
	if (!prod_state_added[add_state_ind]) {
		// State map keeps track of the state pointers
		prod_state_map.push_back(add_state);
		// TS_index map keeps track of the TS state index at each prod state
		prod_TS_index_map.push_back(TS_f);
		// DFA_index map keeps track of the DFA state index at each prod state
		prod_DFA_index_map.push_back(DFA_f);
		prod_state_added[add_state_ind] = true;
		unsigned int new_ind = prod_state_map.size()-1;
		// If the add state is accepting on the DFA, keep track
		if (is_DFA_accepting[DFA_f]) {
			is_accepting.push_back(true);
			std::cout<<"Info: Found accepting product state (index): "<<new_ind<<"\n";
		} else {
			is_accepting.push_back(false);
		}
		//graph_product->Edge::connect(p_i, new_ind, weight, action);
		WL* wl_struct = new WL;
		prod_node_container.push_back(wl_struct);
		wl_struct->weight = weight;
		wl_struct->label = action;
		graph_product->Graph<WL>::connect(p_i, {new_ind, wl_struct});
	} else {
		for (int i=0; i<prod_state_map.size(); ++i) {
			if (add_state == prod_state_map[i]) {
				WL* wl_struct = new WL;
				prod_node_container.push_back(wl_struct);
				wl_struct->weight = weight;
				wl_struct->label = action;
				graph_product->Graph<WL>::connect(p_i, {i, wl_struct});
				//graph_product->Edge::connect(p_i, i, weight, action);				
			}
		}
	}
}

template <class T> 
void ProductSystem<T>::compose() {
	if (automaton_init && TransitionSystem<T>::generated) {

		accepting_DFA_states = graph_DFA->getAcceptingStates();
		init_state_DFA_ind = graph_DFA->getInitState();
		is_DFA_accepting.resize(graph_DFA->size());
		for (int i=0; i<accepting_DFA_states->size(); ++i) {
			is_DFA_accepting[accepting_DFA_states->operator[](i)] = true;
		}

		int possible_states = graph_DFA->size() * TransitionSystem<T>::graph_TS->size();
		//int possible_states = graph_DFA->size() * graph_TS->size();
		prod_state_added.resize(possible_states);
		for (int i=0; i<prod_state_added.size(); i++) {
			prod_state_added[i] = false;	
		}
		int n = TransitionSystem<T>::graph_TS->size();
		int m = graph_DFA->size();
		std::cout<<"n = "<<n<<std::endl;
		std::cout<<"m = "<<m<<std::endl;
		auto heads_TS = TransitionSystem<T>::graph_TS->getHeads();
		auto heads_DFA = graph_DFA->getHeads();
		prod_state_map.clear();
		// Init state in TS will be the same in the Product Graph
		prod_state_map.push_back(TransitionSystem<T>::state_map[0]);
		// The init transition state is the 0th element
		prod_TS_index_map.push_back(0);
		prod_DFA_index_map.push_back(init_state_DFA_ind);
		// Init state cannot be accepting or else the solution is trivial
		is_accepting.push_back(false);
		prod_state_added[Graph<WL>::augmentedStateFunc(0, init_state_DFA_ind, n, m)] = true;
		int p_i = 0; // State index for current state
		
		//int t_ind;
		//std::string t_label;
		//auto getIndLabLAM = [&t_ind, &t_label](Graph<WL>::node* dst, Graph<WL>::node* prv) {
		//	ind = dst->nodeind;	
		//	label = dst->dataptr->label;
		//}

		while (p_i<prod_state_map.size()) {
			int TS_i = prod_TS_index_map[p_i];
			int DFA_i = prod_DFA_index_map[p_i];
			auto currptr_TS = heads_TS->operator[](TS_i)->adjptr;	
			while (currptr_TS!=nullptr){
				TS_f = currptr_TS->nodeind;
				auto currptr_DFA = heads_DFA->operator[](DFA_i)->adjptr;	
				while (currptr_DFA!=nullptr){
					DFA_f = currptr_DFA->nodeind;
					//ind_from = Edge::augmentedStateFunc(i, j, n, m);
					int add_state_ind = Graph<T>::augmentedStateFunc(TS_f, DFA_f, n, m);

					int to_state_ind = currptr_TS->nodeind;
					bool connecting;
					// This function will evaluate edge label in the DFA
					// for an observation and compare that observation
					// to the one seen in the connected state in the TS.
					// This determines the grounds for connection in the
					// product graph
					T* add_state = TransitionSystem<T>::state_map[to_state_ind];
					
					connecting = TransitionSystem<T>::parseLabelAndEval(currptr_DFA->dataptr, add_state);
					if (connecting) {
						// Use the edge labels in the TS because 
						// those represent actions. Also use the 
						// weights in the TS so that the action 
						// cost function can be preserved
						std::string prod_label = currptr_TS->dataptr->label;
						float prod_weight = currptr_TS->dataptr->weight;
						safeAddProdState(p_i, add_state, add_state_ind, prod_weight, currptr_TS->dataptr->label);
					}
					currptr_DFA = currptr_DFA->adjptr;
				}
				currptr_TS = currptr_TS->adjptr;
			}
			p_i++;
		}

	} else {
		std::cout<<"Error: Must set automaton and generate TS before calling compose()\n";
	}
}

/*
template <class T>
void ProductSystem<T>::print() const {
	if (TransitionSystem<T>::state_map.size() > 1) {
		std::pair<unsigned int, unsigned int> temp_TS_DFA_indices;
		for (int i=0; i<graph_product->size(); ++i) {
			Edge::augmentedStateMap(i, TransitionSystem<T>::graph_TS->size(), graph_DFA->size(), temp_TS_DFA_indices);
			T* curr_state = TransitionSystem<T>::state_map[temp_TS_DFA_indices.first];
			std::vector<int> list_nodes; 
			std::vector<std::string> list_actions; 
			graph_product->returnListNodes(i, list_nodes);
			graph_product->returnListLabels(i, list_actions);
			if (list_nodes.size()>0){
				std::cout<<"Product Node: "<<i<<" with state: ";
				std::vector<std::string> state_i; 
				curr_state->getState(state_i);
				for (int ii=0; ii<state_i.size(); ++ii) {
					std::cout<<state_i[ii]<<", ";
				}
				std::cout<<"connects to:\n";
				for (int ii=0; ii<list_nodes.size(); ++ii) {
					Edge::augmentedStateMap(list_nodes[ii], TransitionSystem<T>::graph_TS->size(), graph_DFA->size(), temp_TS_DFA_indices);
					T* con_state = TransitionSystem<T>::state_map[temp_TS_DFA_indices.first];
					std::cout<<"   ~>Product State "<<list_nodes[ii]<<" with state: ";
					con_state->getState(state_i);
					for (int iii=0; iii<state_i.size(); ++iii) {
						std::cout<<state_i[iii]<<", ";
					}
					std::cout<<" with action: "<<list_actions[ii]<<"\n";
				}
			}
		}
	} else {
		std::cout<<"Warning: Transition has not been generated, or has failed to generate. Cannot print\n";
	}
}
*/

template <class T>
bool ProductSystem<T>::plan(std::vector<int>& plan) {
	Astar<WL> planner;
	planner.setGraph(graph_product);
	// The init state is 0 by construction
	planner.setVInit(0);
	std::vector<int> goal_set;
	goal_set.clear();
	for (int i=0; i<is_accepting.size(); ++i) {
		if (is_accepting[i]) {
			std::cout<<"Info: Planning with goal state index: "<<i<<"\n";
			goal_set.push_back(i);
		}
	}
	if (goal_set.size() >= 1) {
	planner.setVGoalSet(goal_set);
	//std::vector<int> reverse_plan;
	float pathlength;
	plan_found = planner.searchDijkstra(plan, pathlength);
	/*
	plan.resize(reverse_plan.size());
	for (int i=0; i<reverse_plan.size(); ++i) {
		plan[i] = reverse_plan[reverse_plan.size()-1-i];
	}
	*/
	stored_plan = plan;
	return plan_found;
	} else {
		std::cout<<"Warning: No goal states were found, cannot plan\n";
		return false;
	}
}

template <class T>
bool ProductSystem<T>::plan() {
	Astar<WL> planner;
	planner.setGraph(graph_product);
	// The init state is 0 by construction
	planner.setVInit(0);
	std::vector<int> goal_set;
	goal_set.clear();
	for (int i=0; i<is_accepting.size(); ++i) {
		if (is_accepting[i]) {
			std::cout<<"Info: Planning with goal state index: "<<i<<"\n";
			goal_set.push_back(i);
		}
	}
	if (goal_set.size() >= 1) {
		planner.setVGoalSet(goal_set);
		//std::vector<int> reverse_plan;
		float pathlength;
		plan_found = planner.searchDijkstra(stored_plan, pathlength);
		/*
		   stored_plan.resize(reverse_plan.size());
		   for (int i=0; i<reverse_plan.size(); ++i) {
		   stored_plan[i] = reverse_plan[reverse_plan.size()-1-i];
		   }
		   */
		return plan_found;
	} else {
		std::cout<<"Warning: No goal states were found, cannot plan\n";
		return false;
	}
}

template <class T>
void ProductSystem<T>::getPlan(std::vector<T*>& state_sequence, std::vector<std::string>& action_sequence) {
	if (plan_found) {
		state_sequence.resize(stored_plan.size());
		action_sequence.resize(stored_plan.size()-1);
		auto prod_heads = graph_product->getHeads();
		int i;
		for (i=0; i<stored_plan.size()-1; ++i) {
			auto currptr = prod_heads->operator[](stored_plan[i])->adjptr;	
			T* curr_state = prod_state_map[stored_plan[i]];
			state_sequence[i] = curr_state;
			while (currptr != nullptr) {
				if (currptr->nodeind == stored_plan[i+1]) {
					action_sequence[i] = currptr->dataptr->label;
					break;
				}
				currptr = currptr->adjptr;
			}
		}
		state_sequence[state_sequence.size()-1] = prod_state_map[stored_plan[state_sequence.size()-1]];
	} else {
		std::cout<<"Error: Plan was not found, cannot return plan\n";
	}
}

//template <class T>
//float ProductSystem<T>::getEdgeWeight(unsigned int action_ind) const {
//	int ind_from, ind_to;
//	ind_from = stored_plan[action_ind];
//	ind_to = stored_plan[action_ind + 1];
//	return graph_product->getWeight(ind_from, ind_to);
//}

//template <class T>
//void ProductSystem<T>::updateEdgeWeight(unsigned int action_ind, float weight) {
//	int ind_from, ind_to;
//	ind_from = stored_plan[action_ind];
//	ind_to = stored_plan[action_ind + 1];
//	graph_product->updateWeight(ind_from, ind_to, weight);
//}

template <class T>
void ProductSystem<T>::clearPS() {
	// UNFINISHED
	graph_product->clear();
	prod_state_map.clear();
	for (int i=0; i<prod_node_container.size(); ++i) {
		delete prod_node_container[i];
	}	
}


template <class T>
void ProductSystem<T>::printPS() const {
	if (prod_state_map.size() > 1) {
		for (int i=0; i<prod_state_map.size(); ++i) {
			T* curr_state = prod_state_map[i];
			std::vector<WL*> con_data; 
			std::vector<int> con_nodes; 
			graph_product->getConnectedData(i, con_data);
			graph_product->getConnectedNodes(i, con_nodes);
			std::cout<<"Product State "<<i<<": ";
			std::vector<std::string> state_i; 
			curr_state->getState(state_i);
			for (int ii=0; ii<state_i.size(); ++ii) {
				std::cout<<state_i[ii]<<", ";
			}
			std::cout<<"connects to:\n";
			for (int ii=0; ii<con_nodes.size(); ++ii) {
				T* con_state = prod_state_map[con_nodes[ii]];
				std::cout<<"   ~>Product State "<<con_nodes[ii]<<": ";
				con_state->getState(state_i);
				for (int iii=0; iii<state_i.size(); ++iii) {
					std::cout<<state_i[iii]<<", ";
				}
				std::cout<<" with action: "<<con_data[ii]->label<<"\n";
			}
		}
	} else {
		std::cout<<"Warning: Product System has not been generated, or has failed to generate. Cannot print\n";
	}
}

template <class T>
ProductSystem<T>::~ProductSystem() {
	for (int i=0; i<prod_node_container.size(); ++i) {
		delete prod_node_container[i];
	}	
}

template class ProductSystem<State>;
template class ProductSystem<BlockingState>;
