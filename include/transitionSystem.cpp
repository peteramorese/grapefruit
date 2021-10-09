#include<string>
#include<vector>
#include<array>
#include<iostream>
#include "transitionSystem.h"
#include "edge.h"
#include "state.h"
#include "condition.h"
#include "astar.h"

template <class T>
TransitionSystem<T>::TransitionSystem (Edge* graph_TS_) : has_conditions(false), generated(false) {
	if (graph_TS_->isOrdered()) {
		graph_TS = graph_TS_;	
	} else {
		std::cout<<"Error: Transition System must take in an ordered graph\n";
	}
	conditions.clear();
	
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
void TransitionSystem<T>::setInitState(T* init_state_) {
	init_state = init_state_;
	has_init_state = true;
	all_states.clear();
	init_state->generateAllPossibleStates(all_states);
	state_added.resize(all_states.size());
	for (int i=0; i<state_added.size(); ++i) {
		state_added[i] = false;
	}
}

template <class T>
void TransitionSystem<T>::safeAddState(T* add_state, int add_state_ind, Condition* cond){
	std::string action = cond->getActionLabel();
	if (!state_added[add_state_ind]) {
		state_map.push_back(add_state);
		state_added[add_state_ind] = true;
		unsigned int new_ind = state_map.size()-1;
		graph_TS->Edge::connect(q_i, new_ind, 1.0f, action);
	} else {
		for (int i=0; i<state_map.size(); ++i) {
			if (add_state == state_map[i]) {
				graph_TS->Edge::connect(q_i, i, 1.0f, action);				
			}
		}
	}
}

template <class T>
T* TransitionSystem<T>::getState(int node_index) {
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
	q_i = 0; // State index for current state
	while (q_i<state_map.size() && init_state_found) {
		T* curr_state = state_map[q_i];
		for (unsigned int i=0; i<state_count; ++i) {
			T* new_state = &all_states[i];
			if (!(new_state == curr_state)) {
				for (int ii=0; ii<cond_count; ++ii) {
					bool satisfied;
					satisfied = conditions[ii]->evaluate(curr_state, new_state);
					if (satisfied) {
						safeAddState(new_state, i, conditions[ii]);
					}
				}
			}	
		}
		q_i++;
	}
	} else {
		std::cout<<"Error: Must set init state and conditions before calling generate()\n";
	}
	generated = true;
}

template <class T>
void TransitionSystem<T>::clearTS() {
	graph_TS->clear();
	state_map.clear();	
	generated = false;
}

template <class T>
void TransitionSystem<T>::printTS() const {
	if (state_map.size() > 1) {
	for (int i=0; i<state_map.size(); ++i) {
		T* curr_state = state_map[i];
		std::vector<int> list_nodes; 
		std::vector<std::string> list_actions; 
		graph_TS->returnListNodes(i, list_nodes);
		graph_TS->returnListLabels(i, list_actions);
		std::cout<<"State "<<i<<": ";
		std::vector<std::string> state_i; 
		curr_state->getState(state_i);
		for (int ii=0; ii<state_i.size(); ++ii) {
			std::cout<<state_i[ii]<<", ";
		}
		std::cout<<"connects to:\n";
		for (int ii=0; ii<list_nodes.size(); ++ii) {
			T* con_state = state_map[list_nodes[ii]];
			std::cout<<"   ~>State "<<list_nodes[ii]<<": ";
			con_state->getState(state_i);
			for (int iii=0; iii<state_i.size(); ++iii) {
				std::cout<<state_i[iii]<<", ";
			}
			std::cout<<" with action: "<<list_actions[ii]<<"\n";
		}
	}
	} else {
		std::cout<<"Warning: Transition has not been generated, or has failed to generate. Cannot print\n";
	}
}


template class TransitionSystem<State>;
template class TransitionSystem<BlockingState>;

template <class T>
ProductSystem<T>::ProductSystem(Edge* graph_TS_, Edge* graph_DA_, Edge* graph_product_) : 
	TransitionSystem<T>(graph_TS_) {
		if (graph_product_->isOrdered()) {
			graph_product = graph_product_;
		} else {
			std::cout<<"Error: Product System must return an ordered graph\n";
		}
		if (graph_DA_->isOrdered()) {
			graph_DA = graph_DA_;
		} else {
			std::cout<<"Error: Product System must take in an ordered graph (DA)\n";
		}
		is_DA_accepting.resize(graph_DA->size());
		for (int i=0; i<graph_DA->size(); ++i) {
			is_DA_accepting[i] = false;	
		}
	}

template <class T>
void ProductSystem<T>::addProposition(SimpleCondition* proposition_) {
	if (proposition_->getLabel() != Condition::FILLER) {
		propositions[proposition_->getLabel()] = proposition_;
	} else {
		std::cout<<"Error: Must name proposition before including in Product System\n";
	}
}

template <class T>
void ProductSystem<T>::setPropositions(const std::vector<SimpleCondition*>& propositions_) {
	for (int i=0; i<propositions_.size(); ++i) {
		if (propositions_[i]->getLabel() != Condition::FILLER) {
			propositions[propositions_[i]->getLabel()] = propositions_[i];
		} else {
			std::cout<<"Error: Must name all propositions before including in Product System\n";
		}
	}
}

template <class T>
void ProductSystem<T>::setAutomatonInitStateIndex(int init_state_DA_ind_) {
	init_state_DA_ind = init_state_DA_ind_;
	automaton_init = true;
}

template <class T>
void ProductSystem<T>::setAutomatonAcceptingStateIndices(const std::vector<int>& accepting_DA_states_) {
	bool in_bounds = true;
	for (int i=0; i<accepting_DA_states_.size(); ++i) {
		if (accepting_DA_states_[i] > graph_DA->size()-1) {
			std::cout<<"Error: All accepting state indices have to appear in automaton";	
			in_bounds = false;
		}
	}
	if (in_bounds) {
		accepting_DA_states = accepting_DA_states_;
		for (int ii=0; ii<accepting_DA_states.size(); ++ii) {
			is_DA_accepting[accepting_DA_states_[ii]] = true;
		}
	}
}

template <class T>
void ProductSystem<T>::addAutomatonAcceptingStateIndex(int accepting_DA_state_) {
	if (accepting_DA_state_ > graph_DA->size()-1) {
		std::cout<<"Error: Accepting state must appear in automaton";	
	} else {
		std::cout<<"is_DA_accepting size:"<<is_DA_accepting.size()<<std::endl;
		is_DA_accepting[accepting_DA_state_] = true;
	}
}


template <class T>
bool ProductSystem<T>::parseLabelAndEval(const std::string& label, const T* state) {
	bool negate_next = false;
	bool is_first = true;
	bool arg;
	//bool arg_conj;
	//std::vector<bool> propositions;
	int prop_i = -1;
	std::string temp_name;
	char prev_operator;
	for (int i=0; i<label.size(); ++i) {
		char character = label[i];
		bool sub_eval;
		switch (character) {
			case '!':
				negate_next = !negate_next;
				break;
			case '&':
				sub_eval = propositions[temp_name]->evaluate(state);	
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
				sub_eval = propositions[temp_name]->evaluate(state);	
				
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
				if (i == label.size()-1) {
					sub_eval = propositions[temp_name]->evaluate(state);	
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
void ProductSystem<T>::safeAddProdState(T* add_state, int add_state_ind, float weight, const std::string& action){
	if (!prod_state_added[add_state_ind]) {
		// State map keeps track of the state pointers
		prod_state_map.push_back(add_state);
		// TS_index map keeps track of the TS state index at each prod state
		prod_TS_index_map.push_back(TS_f);
		// DA_index map keeps track of the DA state index at each prod state
		prod_DA_index_map.push_back(DA_f);
		prod_state_added[add_state_ind] = true;
		unsigned int new_ind = prod_state_map.size()-1;
		// If the add state is accepting on the DA, keep track
		if (is_DA_accepting[DA_f]) {
			is_accepting.push_back(true);
			std::cout<<"Info: Found accepting product state (index): "<<new_ind<<"\n";
		} else {
			is_accepting.push_back(false);
		}
		graph_product->Edge::connect(p_i, new_ind, weight, action);
	} else {
		for (int i=0; i<prod_state_map.size(); ++i) {
			if (add_state == prod_state_map[i]) {
				graph_product->Edge::connect(p_i, i, weight, action);				
			}
		}
	}
}

template <class T> 
void ProductSystem<T>::compose() {
	if (automaton_init && TransitionSystem<T>::generated) {
		int possible_states = graph_DA->size() * TransitionSystem<T>::graph_TS->size();
		prod_state_added.resize(possible_states);
		for (int i=0; i<prod_state_added.size(); i++) {
			prod_state_added[i] = false;	
		}
		int n = TransitionSystem<T>::graph_TS->size();
		int m = graph_DA->size();
		std::cout<<"n = "<<n<<std::endl;
		std::cout<<"m = "<<m<<std::endl;
		auto heads_TS = TransitionSystem<T>::graph_TS->getHeads();
		auto heads_DA = graph_DA->getHeads();
		prod_state_map.clear();
		// Init state in TS will be the same in the Product Graph
		prod_state_map.push_back(TransitionSystem<T>::state_map[0]);
		// The init transition state is the 0th element
		prod_TS_index_map.push_back(0);
		prod_DA_index_map.push_back(init_state_DA_ind);
		// Init state cannot be accepting or else the solution is trivial
		is_accepting.push_back(false);
		prod_state_added[Edge::augmentedStateFunc(0, init_state_DA_ind, n, m)] = true;
		p_i = 0; // State index for current state
		while (p_i<prod_state_map.size()) {
			int TS_i = prod_TS_index_map[p_i];
			int DA_i = prod_DA_index_map[p_i];
			auto currptr_TS = heads_TS[TS_i]->adjptr;	
			while (currptr_TS!=nullptr){
				TS_f = currptr_TS->nodeind;
				auto currptr_DA = heads_DA[DA_i]->adjptr;	
				while (currptr_DA!=nullptr){
					DA_f = currptr_DA->nodeind;
					//ind_from = Edge::augmentedStateFunc(i, j, n, m);
					int add_state_ind = Edge::augmentedStateFunc(TS_f, DA_f, n, m);

					int to_state_ind = currptr_TS->nodeind;
					bool connecting;
					// This function will evaluate edge label in the DA
					// for an observation and compare that observation
					// to the one seen in the connected state in the TS.
					// This determines the grounds for connection in the
					// product graph
					T* add_state = TransitionSystem<T>::state_map[to_state_ind];
					
					connecting = parseLabelAndEval(currptr_DA->label, add_state);
					if (connecting) {
						// Use the edge labels in the TS because 
						// those represent actions. Also use the 
						// weights in the TS so that the action 
						// cost function can be preserved
						std::string prod_label = currptr_TS->label;
						float prod_weight = currptr_TS->weight;
						safeAddProdState(add_state, add_state_ind, prod_weight, currptr_TS->label);
					}
					currptr_DA = currptr_DA->adjptr;
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
		std::pair<unsigned int, unsigned int> temp_TS_DA_indices;
		for (int i=0; i<graph_product->size(); ++i) {
			Edge::augmentedStateMap(i, TransitionSystem<T>::graph_TS->size(), graph_DA->size(), temp_TS_DA_indices);
			T* curr_state = TransitionSystem<T>::state_map[temp_TS_DA_indices.first];
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
					Edge::augmentedStateMap(list_nodes[ii], TransitionSystem<T>::graph_TS->size(), graph_DA->size(), temp_TS_DA_indices);
					T* con_state = TransitionSystem<T>::state_map[temp_TS_DA_indices.first];
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
	Astar planner;
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
	std::cout<<"HELLO: "<<plan.size()<<std::endl;
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
	Astar planner;
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
		std::cout<<"HELLO: "<<stored_plan.size()<<std::endl;
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
			auto currptr = prod_heads[stored_plan[i]]->adjptr;	
			T* curr_state = prod_state_map[stored_plan[i]];
			state_sequence[i] = curr_state;
			while (currptr != nullptr) {
				if (currptr->nodeind == stored_plan[i+1]) {
					action_sequence[i] = currptr->label;
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

template <class T>
float ProductSystem<T>::getEdgeWeight(unsigned int action_ind) const {
	int ind_from, ind_to;
	ind_from = stored_plan[action_ind];
	ind_to = stored_plan[action_ind + 1];
	return graph_product->getWeight(ind_from, ind_to);
}

template <class T>
void ProductSystem<T>::updateEdgeWeight(unsigned int action_ind, float weight) {
	int ind_from, ind_to;
	ind_from = stored_plan[action_ind];
	ind_to = stored_plan[action_ind + 1];
	graph_product->updateWeight(ind_from, ind_to, weight);
}

template <class T>
void ProductSystem<T>::clearPS() {
	// UNFINISHED
	graph_product->clear();
	prod_state_map.clear();
}


template <class T>
void ProductSystem<T>::printPS() const {
	if (prod_state_map.size() > 1) {
		for (int i=0; i<prod_state_map.size(); ++i) {
			T* curr_state = prod_state_map[i];
			std::vector<int> list_nodes; 
			std::vector<std::string> list_actions; 
			graph_product->returnListNodes(i, list_nodes);
			graph_product->returnListLabels(i, list_actions);
			std::cout<<"Product State "<<i<<": ";
			std::vector<std::string> state_i; 
			curr_state->getState(state_i);
			for (int ii=0; ii<state_i.size(); ++ii) {
				std::cout<<state_i[ii]<<", ";
			}
			std::cout<<"connects to:\n";
			for (int ii=0; ii<list_nodes.size(); ++ii) {
				T* con_state = prod_state_map[list_nodes[ii]];
				std::cout<<"   ~>Product State "<<list_nodes[ii]<<": ";
				con_state->getState(state_i);
				for (int iii=0; iii<state_i.size(); ++iii) {
					std::cout<<state_i[iii]<<", ";
				}
				std::cout<<" with action: "<<list_actions[ii]<<"\n";
			}
		}
	} else {
		std::cout<<"Warning: Product System has not been generated, or has failed to generate. Cannot print\n";
	}
}


template class ProductSystem<State>;
template class ProductSystem<BlockingState>;
