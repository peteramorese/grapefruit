#include<string>
#include<vector>
#include<array>
#include<iostream>
#include "transitionSystem.h"

// Transition system uses WL structure: 
// W: edge weight (action cost)
// L: action


template <class T>
TransitionSystem<T>::TransitionSystem(bool UNIQUE_ACTION_, bool manual_) : Graph<WL>(true, true), UNIQUE_ACTION(UNIQUE_ACTION_), manual(manual_), generated(false), mapped(false), init_state(nullptr) {}


// This is part of the "Labeling Function"
template <class T>
bool TransitionSystem<T>::parseLabelAndEval(const std::string& label, const T* state) {
	if (label == "1") {
		return true;
	}
	int prop_i = -1;
	
	bool debug = false;
	//if (label=="!obj_1_L3&!obj_2_L2") {
	//	std::cout<<"INVESTIGATING"<<std::endl;
	//	state->print();
	//	std::cout<<"\n";
	//	debug = true;
	//}

	// Use stack data structures to account for parenthesis:
	std::vector<std::string> prop_buffer(1);
	std::vector<bool> bool_buffer(1);
	//std::vector<bool> is_first_buffer(1);
	std::vector<bool> negate_next_buffer(1);
	std::vector<char> prev_operator_buffer(1);
	//is_first_buffer[0] = true;
	negate_next_buffer[0] = false;
	prev_operator_buffer[0] = '\0';
	//char prev_operator = '\0';
	bool collapse = false;
	for (int i=0; i<label.size(); ++i) {
		char character = label[i];
		bool sub_eval;

		//std::cout<<"Printing buffers: "<<std::endl;
		//std::cout<<" bool: ";
		//for (int bb=0; bb<bool_buffer.size(); bb++) {
		//	std::cout<<bool_buffer[bb]<<", ";
		//}
		//std::cout<<"\n prop: ";
		//for (int bb=0; bb<prop_buffer.size(); bb++) {
		//	std::cout<<prop_buffer[bb]<<", ";
		//}
		//std::cout<<"\n prev: ";
		//for (int bb=0; bb<prev_operator_buffer.size(); bb++) {
		//	std::cout<<prev_operator_buffer[bb]<<", ";
		//}
		//std::cout<<"\n neg : ";
		//for (int bb=0; bb<negate_next_buffer.size(); bb++) {
		//	std::cout<<negate_next_buffer[bb]<<", ";
		//}
		//std::cout<<"\n";

		if (character == '!') {
			negate_next_buffer.back() = !negate_next_buffer.back();
			continue;
		} else if (character == ' ') { 
			continue;
		} 
		if (character == '(') { // Push to stack
			//is_first_buffer.push_back(true); // First item on stack
			negate_next_buffer.push_back(false); 
			prev_operator_buffer.push_back('\0');
			bool_buffer.push_back(true); 
			prop_buffer.push_back("");
		} else {
			if (character == ')') { // Pop from stack
				if (bool_buffer.size() <= 1) {
						std::cout<<"Error (parseLabelAndEval): Found ')' without an opening bracket.\n";
						return false;
				}
				// Evaluate the top of the stack
				if (!collapse) {
				//std::cout<<"Evaluating label ')': "<<prop_buffer.back()<<std::endl;
				sub_eval = TransitionSystem<T>::propositions.at(prop_buffer.back())->evaluate(state);	
				//std::cout<<"  Result: "<<sub_eval<<std::endl;
				} 
				if (negate_next_buffer.back()) {
					sub_eval = !sub_eval;
				}
				prop_buffer.back().clear();
				if (prev_operator_buffer.back() == '\0') { // First variable seen
					bool_buffer.back() = sub_eval;
				} else {
					if (prev_operator_buffer.back() == '|') {
						bool_buffer.back() = bool_buffer.back() || sub_eval;
					} else if (prev_operator_buffer.back() == '&') {
						bool_buffer.back() = bool_buffer.back() && sub_eval;
					} 
				}
				
				// Pop items off stack
				negate_next_buffer.pop_back();
				prev_operator_buffer.pop_back();
				prop_buffer.pop_back();
				sub_eval = bool_buffer.back(); // Get the value from the top of the stack
				bool_buffer.pop_back();
				collapse = true; // Do not re eval sub_eval, it already has its value
			} 
			bool at_end = i == label.size()-1;
			if (character == '|' || character == '&' || at_end) { // If operator is seen, or at the end of the label
				if (at_end) {
					prop_buffer.back().push_back(character);
				}
				if (!collapse) {
					sub_eval = TransitionSystem<T>::propositions.at(prop_buffer.back())->evaluate(state);	
					if (debug){
						std::cout<<"Evaluating label: "<<prop_buffer.back()<<std::endl;
						std::cout<<"  Result: "<<sub_eval<<std::endl;
					}
				} else {
					collapse = false;
				}
				if (negate_next_buffer.back()) {
					sub_eval = !sub_eval;
					negate_next_buffer.back() = false;
				}
				prop_buffer.back().clear();
				if (prev_operator_buffer.back() == '\0') { // First variable seen
					bool_buffer.back() = sub_eval;
					prev_operator_buffer.back() = character;
				} else {
					if (prev_operator_buffer.back() == '|') {
						bool_buffer.back() = bool_buffer.back() || sub_eval;
					} else if (prev_operator_buffer.back() == '&') {
						bool_buffer.back() = bool_buffer.back() && sub_eval;
					} 
					prev_operator_buffer.back() = character;
				}
			} else {
				prop_buffer.back().push_back(character);
			}
		}
	}
	if (debug) {
		std::cout<<"----returning: "<<bool_buffer[0]<<std::endl;
	}
	return bool_buffer[0];
}

template <class T>
void TransitionSystem<T>::addCondition(Condition* condition_){
	conditions.push_back(condition_);
}

template <class T>
void TransitionSystem<T>::setConditions(const std::vector<Condition*>& conditions_) {
	conditions = conditions_;
}

template <class T>
void TransitionSystem<T>::addProposition(SimpleCondition* proposition_) {
	if (proposition_->getLabel() != Condition::FILLER) {
		//std::cout<<"Info: Found proposition: "<<propositions_[i]->getLabel()<<std::endl;
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
	all_states.clear();
	if (manual) {
		all_states.push_back(*init_state);
		state_map.push_back(init_state);
	} else {
		init_state->generateAllPossibleStates(all_states);
		state_added.resize(all_states.size());
		state_added_ind.resize(all_states.size());
		for (int i=0; i<state_added.size(); ++i) {
			state_added[i] = false;
		}
	}
}

template <class T>
int TransitionSystem<T>::getInitStateInd() {
	return 0; //Based off of generate
}

template <class T>
void TransitionSystem<T>::safeAddState(int q_i, T* add_state, int add_state_ind, Condition* cond){
	std::string action = cond->getActionLabel();
	float action_cost = cond->getActionCost();
	if (!state_added[add_state_ind]) {
		state_map.push_back(add_state);
		state_added[add_state_ind] = true;
		state_added_ind[add_state_ind] = state_map.size() - 1;
		unsigned int new_ind = state_map.size()-1;
		//graph_TS->Graph<WL>::connect(q_i, new_ind, 1.0f, action);
		
		// New state structure: 
		//WL* wl_struct = new WL;
		//node_container.push_back(wl_struct);
		std::shared_ptr<WL> wl_struct_shptr = std::make_shared<WL>();
		std::weak_ptr<WL> wl_struct = wl_struct_shptr;
		node_container.push_back(std::move(wl_struct_shptr));


		wl_struct.lock()->weight = action_cost;
		//wl_struct->label = action;
		if (UNIQUE_ACTION) {
			wl_struct.lock()->label = action + "_" + std::to_string(q_i) + "_" + std::to_string(new_ind);
		} else {
			wl_struct.lock()->label = action;
		}
		Graph<WL>::connect(q_i, {new_ind, wl_struct.lock().get()});
	} else {
		//for (int i=0; i<state_map.size(); ++i) {
		//	if (add_state == state_map[i]) {

		// New state structure:
		//WL* wl_struct = new WL;
		//node_container.push_back(wl_struct);
		unsigned found_state_ind = state_added_ind[add_state_ind];

		std::shared_ptr<WL> wl_struct_shptr = std::make_shared<WL>();
		std::weak_ptr<WL> wl_struct = wl_struct_shptr;
		node_container.push_back(std::move(wl_struct_shptr));

		wl_struct.lock()->weight = action_cost;
		//std::cout<<"action_cost: "<<action_cost<<std::endl;
		if (UNIQUE_ACTION) {
			wl_struct.lock()->label = action + "_" + std::to_string(q_i) + "_" + std::to_string(found_state_ind);
		} else {
			wl_struct.lock()->label = action;
		}
		Graph<WL>::connect(q_i, {found_state_ind, wl_struct.lock().get()});
		//graph_TS->Edge::connect(q_i, i, 1.0f, action);				

		//	}
		//}
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

		std::shared_ptr<WL> wl_struct_shptr = std::make_shared<WL>();
		std::weak_ptr<WL> wl_struct = wl_struct_shptr;
		node_container.push_back(std::move(wl_struct_shptr));

		//WL* wl_struct = new WL;
		//node_container.push_back(wl_struct);
		wl_struct.lock()->weight = action_cost;
		//std::cout<<"action_cost: "<<action_cost<<std::endl;
		if (UNIQUE_ACTION) {
			wl_struct.lock()->label = action + "_" + std::to_string(src_ind) + "_" + std::to_string(dst_ind);
		} else {
			wl_struct.lock()->label = action;
		}
		dst_node.first = dst_ind;
		dst_node.second = wl_struct.lock().get();
		//std::cout<<"connecting: "<<src_ind<<" to: "<<dst_node.first<<std::endl;
		Graph<WL>::connect(src_ind, dst_node);
		generated = true;
		return true;
	} else {
		std::cout<<"Error: Cannot use connect() when not in manual mode\n";
		return false;
	}
}

template <class T> // TODO: Get rid of this method someday by removing 'state_map'
void TransitionSystem<T>::finishConnecting() {
	state_map.clear();
	state_map.resize(all_states.size());
	for (int i=0; i<all_states.size(); ++i) {
		state_map[i] = &all_states[i];
	}
}


template <class T>
const T* TransitionSystem<T>::getState(int node_index) const {
	return state_map.at(node_index);
}


template <class T>
void TransitionSystem<T>::mapStatesToLabels(const std::vector<const DFA::alphabet_t*>& alphabet) {
	if (!mapped) {
		state_to_label_map.clear();
		//std::cout<<"Info: Mapping states to labels...\n";
		for (int si=0; si<TransitionSystem<T>::state_map.size(); ++si) {
			std::unordered_map<std::string, bool> incl;
			std::vector<std::string> temp_labels;
			temp_labels.clear();
			for (int i=0; i<alphabet.size(); ++i) {
				for (int ii=0; ii<alphabet[i]->size(); ++ii) {
					// Make sure the label is not already in the set to prevent duplicates
					if (incl[alphabet[i]->operator[](ii)]) {
						continue;
					}

					//bool found = false;
					////std::cout<<"b4 check"<<std::endl;
					//for (int iii=0; iii<temp_labels.size(); ++iii) {
					//	if (temp_labels[iii] == alphabet[i]->operator[](ii)) {
					//		found = true;
					//		break;
					//	}
					//}
					//std::cout<<"af check, b4 plneval"<<std::endl;

					//std::cout<<"checking lbl: "<<alphabet[i]->operator[](ii)<<std::endl;
					//if (!found) {
					if (TransitionSystem<T>::parseLabelAndEval(alphabet[i]->operator[](ii), TransitionSystem<T>::state_map[si])) {
						//std::cout<<"state: "<<si<<" satisfies letter: "<<alphabet[i]->operator[](ii)<<std::endl;
						//state_map[si]->print();
						temp_labels.push_back(alphabet[i]->operator[](ii));
						incl[alphabet[i]->operator[](ii)] = true;
					}
					//}
					//std::cout<<"af plneval"<<std::endl;
				}
			}
			//std::cout<<"mapping state: "<<si<<" to: "<<std::endl;
			//for (int i=0; i<temp_labels.size(); ++i) {
			//	std::cout<<"  label: "<<temp_labels[i]<<std::endl;
			//}

			//int pause;
			//std::cin>>pause;

			state_to_label_map[si] = temp_labels;
		}
		//std::cout<<"Info: Done mapping states to labels.\n";
		mapped = true;
	}
}

template <class T>
const std::vector<std::string>* TransitionSystem<T>::returnStateLabels(int state_ind) const {
	if (state_ind > TransitionSystem<T>::state_map.size()-1) {
		std::cout<<"Error: State ind map out of bounds\n";
		return nullptr;
	} else if (!mapped) {
		std::cout<<"Error: Must call mapStatesToLabels before calling returnStateLabels\n";
		return nullptr;
	} else {
		//mapStatesToLabels();
		return &state_to_label_map.at(state_ind);
	}
}


template <class T>
bool TransitionSystem<T>::generate() {
	if (init_state != nullptr && conditions.size() > 0) {
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
		return true;
	} else {
		std::cout<<"Error: Must set init state and conditions before calling generate()\n";
		return false;
	}
}

template <class T>
void TransitionSystem<T>::clear() {
	state_map.clear();	
	node_container.clear();
	generated = false;
	Graph<WL>::clear();
}

template <class T>
void TransitionSystem<T>::print() {
	//graph_TS->print();
	if (state_map.size() > 1) {
		for (int i=0; i<state_map.size(); ++i) {
			std::vector<WL*> con_data; 
			std::vector<int> con_nodes; 
			//std::vector<std::string> list_actions; 
			//graph_TS->returnListLabels(i, list_actions);
			this->getConnectedData(i, con_data);
			this->getConnectedNodes(i, con_nodes);
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

template class TransitionSystem<State>;
template class TransitionSystem<BlockingState>;





		////////////////////////
		/* GRAPH EVAL CLASSES */
		////////////////////////


//template class TS_EVAL<State>;
//template class TS_EVAL<BlockingState>;


template <class T>
TS_EVAL<T>::TS_EVAL(int init_node_) : TransitionSystem<T>(), init_node(init_node_) {
	curr_node = init_node; //ts is generated around 0 being init state
}

template <class T>
TS_EVAL<T>::TS_EVAL(bool UNIQUE_ACTION_, bool manual_, int init_node_) : TransitionSystem<T>(UNIQUE_ACTION_, manual_), init_node(init_node_) {
	curr_node = init_node; //ts is generated around 0 being init state
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
	if (this->hopS(curr_node, evalLAM)) {
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
	if (this->parentHopS(curr_node, evalLAM)) {
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


//template <class T>
//bool TS_EVAL<T>::isReversible() const {
//	return TransitionSystem<T>::graph_TS->isReversible();
//}

template <class T>
int TS_EVAL<T>::getCurrNode() const {
	return curr_node;
}

template <class T>
void TS_EVAL<T>::getConnectedDataEVAL(std::vector<WL*>& con_data) {
	this->getConnectedData(curr_node, con_data);
}

template <class T>
void TS_EVAL<T>::getConnectedNodesEVAL(std::vector<int>& con_nodes) {
	this->getConnectedNodes(curr_node, con_nodes);
}

template <class T>
void TS_EVAL<T>::getParentDataEVAL(std::vector<WL*>& con_data) {
	this->getParentData(curr_node, con_data);
}

template <class T>
void TS_EVAL<T>::getParentNodesEVAL(std::vector<int>& con_nodes) {
	this->getParentNodes(curr_node, con_nodes);
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

