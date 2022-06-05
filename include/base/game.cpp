#include<string>
#include<vector>
#include<array>
#include<iostream>
#include "game.h"

template<class T>
Game<T>::Game(unsigned num_players_) : num_players(num_players_) {}

 
template<class T>
void Game<T>::addCondition(Condition* condition_, int player) {

}

template<class T>
void Game<T>::setConditions(const std::vector<Condition*>& conditions_, const std::vector<int>& players) {

}

template<class T>
void Game<T>::generate() {

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