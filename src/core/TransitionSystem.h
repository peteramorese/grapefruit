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
		TransitionSystemLabel(float cost_, const std::string& action_) 
			: cost(cost_)
			, action(action_)
			{}
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
					//inline State& operator[](uint32_t state_ind) {return m_ind_to_state[state_ind];}
					inline State& operator[](uint32_t state_ind) {return m_ind_to_state.at(state_ind);}
					//inline const State& operator[](uint32_t state_ind) const {return m_ind_to_state[state_ind];}
					inline const State& operator[](uint32_t state_ind) const {return m_ind_to_state.at(state_ind);}
					inline uint32_t operator[](const State& state) const {return m_state_to_ind.at(state);}
					std::pair<uint32_t, bool> tryInsert(const State& state) {
						if (!m_state_to_ind.contains(state)) {
							m_ind_to_state.push_back(state);
							uint32_t ind = m_ind_to_state.size() - 1;
							m_state_to_ind[state] = ind;
							return {ind, true};
						} else {
							return {m_state_to_ind.at(state), false};
						}
					}
					inline std::size_t size() const {return m_ind_to_state.size();}
				private:
					std::vector<State> m_ind_to_state;
					std::unordered_map<State, uint32_t> m_state_to_ind;
			};

		public:
			TransitionSystem() = default;
			TransitionSystem(const std::string& filepath);

			
			bool connect(const State& src_state, const State& dst_state, const TransitionSystemLabel& edge) {
				return Graph<TransitionSystemLabel>::connect(m_state_container.tryInsert(src_state).first, m_state_container.tryInsert(dst_state).first, edge);
			}

			virtual void print() const override;

		private:
			void deserialize(const std::string& filepath);

		protected:
		 	BijectiveStateContainer m_state_container;
			std::unordered_map<std::string, Condition> m_propositions;
			
			friend class TransitionSystemGenerator;
	};

 
	// Generator
	struct TransitionSystemProperties {
		TransitionSystemProperties(const StateSpace* ss) : init_state(ss) {}
		std::vector<Condition> propositions;
		std::vector<TransitionCondition> conditions;
		State init_state;
		bool global_unique_actions = true;
	};

	class TransitionSystemGenerator {
		public:
			static const std::shared_ptr<TransitionSystem> generate(TransitionSystemProperties& specs);
	};
}