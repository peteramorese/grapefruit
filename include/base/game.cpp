#include<string>
#include<vector>
#include<array>
#include<iostream>
#include "game.h"
//#include "game.h"

template<class T>
Game<T>::Game(unsigned num_players_, bool UNIQUE_ACTION_, bool manual_) : TransitionSystem<T>(UNIQUE_ACTION_, manual_), num_players(num_players_), player_conditions(num_players) {}

 
template<class T>
void Game<T>::addCondition(Condition* condition, Condition* player_condition_) {
	TransitionSystem<T>::conditions.push_back(condition);
	player_conditions.push_back(player_condition_);
}

template<class T>
void Game<T>::setConditions(const std::vector<Condition*>& conditions_, const std::vector<Condition*>& player_conditions_) {
	if (conditions_.size() == player_conditions_.size()) {
		TransitionSystem<T>::conditions = conditions_;
		player_conditions = player_conditions_;
	} else {
		std::cout<<"Error: Number of conditions must match the number of player conditions. \n";
	}
}


template <class T>
void Game<T>::setInitState(T* init_state_, unsigned init_player_) {
	this->init_state = init_state_;
	this->all_states.clear();
	if (this->manual) {
		this->all_states.push_back(*this->init_state);
		this->state_map.push_back(this->init_state);
		player_map.push_back(init_player_);
	} else {
		init_player = init_player_;
		this->init_state->generateAllPossibleStates(this->all_states);
		this->state_added.resize(num_players*this->all_states.size(), false);
		this->state_added_ind.resize(num_players*this->all_states.size(), 0);
		//for (int i=0; i<this->state_added.size(); ++i) {
		//	this->state_added[i] = false;
		//}
	}
}

template <class T>
void Game<T>::safeAddState(int q_i, T* add_state, int add_state_ind, int add_state_player, Condition* cond){
	std::string action = cond->getActionLabel();
	float action_cost = cond->getActionCost();
	int p_add_state_ind = Graph<int>::augmentedStateImage({add_state_ind, add_state_player}, {static_cast<int>(this->all_states.size()), static_cast<int>(num_players)});
	//for(int i=0; i<this->player_map.size(); ++i) {
	//	std::cout<<"     player_map["<<i<<"]: "<<this->player_map[i]<<std::endl;
	//}
	//std::cout<<"cur state s: "<<q_i<<" adding state: "<<std::endl;
	//add_state->print();
	//int pause;
	//std::cin>>pause;

	if (!this->state_added[p_add_state_ind]) {
		//std::cout<<"  p_add_state_ind (new): "<<p_add_state_ind<<std::endl;
		this->state_map.push_back(add_state);
		player_map.push_back(add_state_player);
		this->state_added[p_add_state_ind] = true;
		unsigned int new_ind = this->state_map.size()-1;
		this->state_added_ind[p_add_state_ind] = new_ind;
		//graph_TS->Graph<WL>::connect(q_i, new_ind, 1.0f, action);
		
		// New state structure: 
		//WL* wl_struct = new WL;
		//node_container.push_back(wl_struct);
		std::shared_ptr<WL> wl_struct_shptr = std::make_shared<WL>();
		std::weak_ptr<WL> wl_struct = wl_struct_shptr;
		this->node_container.push_back(std::move(wl_struct_shptr));


		wl_struct.lock()->weight = action_cost;
		//wl_struct->label = action;
		if (this->UNIQUE_ACTION) {
			wl_struct.lock()->label = action + "_" + std::to_string(q_i) + "_" + std::to_string(new_ind);
		} else {
			wl_struct.lock()->label = action;
		}
		//if (player_map[q_i] == add_state_player){
		//	std::cout<<"ERROR THEY THE SAME"<<std::endl;
		//	*((char*)0) = 0;
		//}
		//std::cout<<"     -connecting: "<<q_i<<" player: "<<player_map[q_i]<<", to: "<<new_ind<<" player: "<<add_state_player<<std::endl;
		Graph<WL>::connect(q_i, {new_ind, wl_struct.lock().get()});
	} else {
		//for (int i=0; i<state_map.size(); ++i) {
		//	if (add_state == state_map[i]) {

		// New state structure:
		//WL* wl_struct = new WL;
		//node_container.push_back(wl_struct);
		unsigned found_state_ind = this->state_added_ind[p_add_state_ind];
		if (add_state_player != player_map[found_state_ind]) {
			//std::cout<<"  p_add_state_ind (found): "<<p_add_state_ind<<std::endl;
			std::cout<<"Error (safeAddState): Mismatched players: add_state_player: "<<add_state_player<<" player_map player:"<< player_map[found_state_ind]<<" \n";
		}

		std::shared_ptr<WL> wl_struct_shptr = std::make_shared<WL>();
		std::weak_ptr<WL> wl_struct = wl_struct_shptr;
		this->node_container.push_back(std::move(wl_struct_shptr));

		wl_struct.lock()->weight = action_cost;
		//std::cout<<"action_cost: "<<action_cost<<std::endl;
		if (this->UNIQUE_ACTION) {
			wl_struct.lock()->label = action + "_" + std::to_string(q_i) + "_" + std::to_string(found_state_ind);
		} else {
			wl_struct.lock()->label = action;
		}
		//if (player_map[q_i] == add_state_player){
		//	std::cout<<"ERROR THEY THE SAME"<<std::endl;
		//}
		//std::cout<<"     -connecting: "<<q_i<<" player: "<<player_map[q_i]<<", to: "<<found_state_ind<<" player: "<<add_state_player<<std::endl;
		Graph<WL>::connect(q_i, {found_state_ind, wl_struct.lock().get()});
		//graph_TS->Edge::connect(q_i, i, 1.0f, action);				

		//	}
		//}
	}
}


template <class T>
bool Game<T>::generate() {
	if (this->init_state != nullptr && this->conditions.size() > 0) {
		int state_count = this->all_states.size();
		int cond_count = this->conditions.size();
		/*
		   for (int i=0; i<all_states.size(); i++) {
		   all_states[i].print();
		   }
		   */
		T* init_state_in_set;
		bool init_state_found = false;
		for (int i=0; i<state_count; ++i) {
			if (this->all_states[i] == this->init_state) {
				int p_init_state_ind = Graph<int>::augmentedStateImage({i, static_cast<int>(init_player)}, {static_cast<int>(this->all_states.size()), static_cast<int>(num_players)});
				init_state_in_set = &(this->all_states[i]);
				this->state_added[p_init_state_ind] = true;
				init_state_found = true;
				break;
			}	
		}
		if (!init_state_found) {
			std::cout<<"Error: Init State not found in "<<this->all_states.size()<< " generated states\n";
			return false;
		}

		StateSpace player_space;
		std::vector<std::string> player_lbls(num_players);
		for (int i=0; i<num_players; ++i) {
			player_lbls[i] = std::to_string(i);
		}
		player_space.setStateDimension(player_lbls, 0);
		player_space.setStateDimensionLabel(0, "player");


		this->state_map.clear();
		this->state_map.push_back(init_state_in_set);
		player_map.clear();
		player_map.push_back(init_player);

		int q_i = 0; // State index for current state
		while (q_i<this->state_map.size() && init_state_found) {
			T* curr_state = this->state_map[q_i];
			
			// Curr Player state
			std::string curr_player = std::to_string(player_map[q_i]);
			T curr_player_state(&player_space);
			curr_player_state.setState(curr_player, 0);
			//std::cout<<"\n CURR PLAYER STATE: "<<std::endl;
			//curr_player_state.print();

			for (unsigned int i=0; i<state_count; ++i) {
				T* new_state = &(this->all_states[i]);
				//if (!(new_state == curr_state)) {
				for (int ii=0; ii<cond_count; ++ii) {
					bool c_satisfied; // State condition
					c_satisfied = this->conditions[ii]->evaluate(curr_state, new_state);
					if (!c_satisfied) {
						//std::cout<<"C NOT SATISFIED ii: "<<ii<<std::endl;
						continue;
					}
					for (int iii=0; iii<num_players; iii++){

						// New Player state
						std::string new_player = std::to_string(iii);
						T new_player_state(&player_space);
						new_player_state.setState(new_player, 0);

						bool p_satisfied; // Player condition
						if (player_conditions[ii] == nullptr) { // null applies to all
							p_satisfied = true;
						} else {
							//std::cout<<"-curr_player_state:"<<std::endl;
							//curr_player_state.print();
							//std::cout<<"-new_player_state:"<<std::endl;
							//new_player_state.print();
							//std::cout<<"  evaluating condition: "<<player_conditions[ii]->getActionLabel()<<std::endl;
							p_satisfied = player_conditions[ii]->evaluate(&curr_player_state, &new_player_state);
							//std::cout<<"satisfied? "<<p_satisfied<<std::endl;
						}

						//std::cout<<" c_satisfied: "<<c_satisfied<<", p_satisfied: "<<p_satisfied<<std::endl;

						if (c_satisfied && p_satisfied) {
							//std::cout<<"Adding!"<<std::endl;
							//new_state->print();
							//std::cout<<"  cur player: "<<player_map[q_i]<<std::endl;
							//std::cout<<"  new player: "<<iii<<std::endl;
							//if (player_map[q_i] == iii) {
							//	std::cout<< "ERROR THEY THE SAME";
							//	return 1;
							//}
							safeAddState(q_i, new_state, i, iii, this->conditions[ii]);

							//int pause;
							//std::cin>>pause;
						}

					}
				}
				//}
			}
			q_i++;
		}
		this->generated = true;
	} else {
		std::cout<<"Error: Must set init state and conditions before calling generate()\n";
		return false;
	}
}


template <class T>
bool Game<T>::connect(T* src, unsigned src_player, T* dst, unsigned dst_player, float action_cost, const std::string& action) {
	if (TransitionSystem<T>::manual) {
		if (TransitionSystem<T>::state_map.size() != 0) {
			TransitionSystem<T>::state_map.clear();
		}
		int src_ind, dst_ind;
		bool src_found = false;
		bool dst_found = false;
		for (int i=0; i<TransitionSystem<T>::all_states.size(); ++i) {
			//std::cout<<"printing state map element i: "<<i<<std::endl;
			//all_states[i]->print();
			if (*src == TransitionSystem<T>::all_states[i] && src_player == player_map[i]) {
                src_found = true;
                src_ind = i;
			}
			if (*dst == TransitionSystem<T>::all_states[i] && dst_player == player_map[i]) {
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
			TransitionSystem<T>::all_states.push_back(*src);
            player_map.push_back(src_player);
			//state_map.push_back(&all_states.back());
			src_ind = TransitionSystem<T>::all_states.size() -1;
		}
		if (!dst_found) {
			TransitionSystem<T>::all_states.push_back(*dst);
            player_map.push_back(dst_player);
			//state_map.push_back(&all_states.back());
			dst_ind = TransitionSystem<T>::all_states.size() -1;
		}

		std::shared_ptr<WL> wl_struct_shptr = std::make_shared<WL>();
		std::weak_ptr<WL> wl_struct = wl_struct_shptr;
		TransitionSystem<T>::node_container.push_back(std::move(wl_struct_shptr));

		//WL* wl_struct = new WL;
		//node_container.push_back(wl_struct);
		wl_struct.lock()->weight = action_cost;
		//std::cout<<"action_cost: "<<action_cost<<std::endl;
		if (TransitionSystem<T>::UNIQUE_ACTION) {
			wl_struct.lock()->label = action + "_" + std::to_string(src_ind) + "_" + std::to_string(dst_ind);
		} else {
			wl_struct.lock()->label = action;
		}
		dst_node.first = dst_ind;
		dst_node.second = wl_struct.lock().get();
		//std::cout<<"connecting: "<<src_ind<<" to: "<<dst_node.first<<std::endl;
		Graph<WL>::connect(src_ind, dst_node);
		TransitionSystem<T>::generated = true;
		return true;
	} else {
		std::cout<<"Error: Cannot use connect() when not in manual mode\n";
		return false;
	}
}

template <class T>
const std::pair<T*, unsigned> Game<T>::getState(int node_index) const {
	return {this->state_map.at(node_index), player_map[node_index]};
}

template <class T>
void Game<T>::print() {
	//graph_TS->print();
	if (TransitionSystem<T>::state_map.size() > 1) {
		for (int i=0; i<TransitionSystem<T>::state_map.size(); ++i) {
			std::vector<WL*> con_data; 
			std::vector<int> con_nodes; 
			//std::vector<std::string> list_actions; 
			//graph_TS->returnListLabels(i, list_actions);
			this->getConnectedData(i, con_data);
			this->getConnectedNodes(i, con_nodes);
			std::cout<<"State "<<i<<" (player: "<<this->player_map[i]<<"): ";
			T* curr_state = this->state_map[i];
			std::vector<std::string> state_i; 
			curr_state->getState(state_i);
			for (int ii=0; ii<state_i.size(); ++ii) {
				std::cout<<state_i[ii]<<", ";
			}
			std::cout<<"connects to:\n";
			for (int ii=0; ii<con_data.size(); ++ii) {
				T* con_state = this->state_map[con_nodes[ii]];
				std::cout<<"   ~>State "<<con_nodes[ii]<<" (player: "<<this->player_map[con_nodes[ii]]<<"): ";
				con_state->getState(state_i);
				for (int iii=0; iii<state_i.size(); ++iii) {
					std::cout<<state_i[iii]<<", ";
				}
				std::cout<<" with action: "<<con_data[ii]->label<<" (cost: "<<con_data[ii]->weight<<")"<<"\n";
			}
		}
	} else {
		std::cout<<"Warning: Game has not been generated, or has failed to generate. Cannot print\n";
	}
}

template <class T>
void Game<T>::clear() {
	player_map.clear();
	TransitionSystem<T>::state_map.clear();	
	TransitionSystem<T>::node_container.clear();
	TransitionSystem<T>::generated = false;
	Graph<WL>::clear();
}


template class Game<State>;
//template class Game<BlockingState>;


