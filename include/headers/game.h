#pragma once
#include "transitionSystem.h"

template<class T>
class Game : protected TransitionSystem<T> {
    private: 
        const int ALL_PLAYERS = -1; // -1 for conditions that should apply regardless of player
        const unsigned num_players;
        std::vector<unsigned> player_map;
        std::vector<std::vector<Condition*>> player_condition_map;
    public:
        Game(unsigned num_players_);
        void addCondition(Condition* condition_, int player = -1);
        void setConditions(const std::vector<Condition*>& conditions_, const std::vector<int>& players);
        void generate();
        bool connect(T* src, unsigned src_player, T* dst, unsigned dst_player, float action_cost, const std::string& action);
        using TransitionSystem<T>::parseLabelAndEval;
        using TransitionSystem<T>::finishConnecting;
        using TransitionSystem<T>::setInitState;
		const std::pair<T*, unsigned> getState(int node_index) const;
        void print();
        void clear();
};
