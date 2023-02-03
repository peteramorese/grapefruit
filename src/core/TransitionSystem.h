#pragma once

#include<string>
#include<vector>
#include<unordered_set>
#include<unordered_map>
#include<memory>

#include "core/Graph.h"
#include "core/State.h"
#include "core/Condition.h"

namespace DiscreteModel {
	struct TransitionSystemLabel {
		float cost;
		std::string action;
	};

	class TransitionSystem : public Graph<TransitionSystemLabel> {
		public:
		 	class BijectiveStateContainer {
				public:
					void addStateIfUnique(const State& state) {
						if (!m_state_to_ind.contains(state)) {
							m_ind_to_state.push_back(state);
							m_state_to_ind[state] = m_ind_to_state.size() - 1;
						}
					}
					State& operator[](uint32_t state_ind) {return m_ind_to_state[state_ind];}
					const State& operator[](uint32_t state_ind) const {return m_ind_to_state[state_ind];}
					uint32_t operator[](const State& state) const {return m_state_to_ind.at(state);}
				private:
					std::vector<State> m_ind_to_state;
					std::unordered_map<State, uint32_t> m_state_to_ind;
			};

		public:
			TransitionSystem();
			TransitionSystem(const std::string& filepath);

			bool parseLabelAndEval(const std::string& label, const T* state);
			int getInitStateInd();
			const T* getState(int node_index) const;
			void mapStatesToLabels(const std::vector<const DFA::alphabet_t*>& alphabet);
			const std::vector<std::string>* returnStateLabels(int state_ind) const;
			virtual bool generate();
			void clear();
			void print();
			void writeToFile(const std::string& filepath);
			std::shared_ptr<StateSpace> readFromFile(const std::string& filename);

		private:
			void safeAddState(int q_i, T* add_state, int add_state_ind, Condition* cond);
			void deserialize(const std::string& filepath);

		protected:
		 	BijectiveStateContainer m_state_container;
			std::unordered_map<std::string, Condition> m_propositions;
			
			friend class TransitionSystemGenerator;
	};

	class TransitionSystemGenerator {
		public:
			struct TransitionSystemProperties {
				std::vector<Condition> propositions;
				std::vector<TransitionCondition> conditions;
				State init_state;
				bool global_unique_actions = true;
			};

		public:
			static const std::shared_ptr<TransitionSystem> generate(const TransitionSystemProperties& specs);

		private:
			static void addStateIfUnique(const State& state, std::vector<State>& state_container, std::unordered_map<State, uint32_t>& unique_state_container);
	};
}