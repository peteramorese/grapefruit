#pragma once
#include "transitionSystem.h"

template<class T>
class Game : public TransitionSystem<T> {
    private: 
        //const int ALL_PLAYERS = -1; // -1 for conditions that should apply regardless of player
        const unsigned num_players;
        unsigned init_player;
        std::vector<unsigned> player_map;
        std::vector<Condition*> player_conditions;
		void safeAddState(int q_i, T* add_state, int add_state_ind, int add_state_player, Condition* cond);
        using TransitionSystem<T>::getState;
        using TransitionSystem<T>::connect;
        using TransitionSystem<T>::addCondition;
        using TransitionSystem<T>::setConditions;
        using TransitionSystem<T>::setInitState;
    public:
        struct Strategy {
            bool success;
            std::vector<std::string> policy; // Maps state index to action
            std::vector<bool> region; // Determines if the state is within the attractor 'O'
        };
        Game(unsigned num_players_, bool UNIQUE_ACTION_ = false, bool manual_ = false);
        void addCondition(Condition* condition_, Condition* player_condition_);
        void setConditions(const std::vector<Condition*>& conditions_, const std::vector<Condition*>& player_conditions_);
        bool generate();
        bool connect(T* src, unsigned src_player, T* dst, unsigned dst_player, float action_cost, const std::string& action);
        //using TransitionSystem<T>::finishConnecting;
        //using TransitionSystem<T>::addProposition;
        //using TransitionSystem<T>::setPropositions;
        //using TransitionSystem<T>::getParentNodes;
        //using TransitionSystem<T>::getParentData;
        //using TransitionSystem<T>::getParentNodes;
        //using TransitionSystem<T>::getParentData;
        //using TransitionSystem<T>::returnStateLabels;
        void setInitState(T* init_state_, unsigned init_player);
        //using TransitionSystem<T>::setInitState;
		const std::pair<T*, unsigned> getState(int node_index) const;
        void print();
        void clear();
};

