#pragma once

#include<string>
#include<vector>
#include<unordered_set>
#include<unordered_map>
#include<memory>

#include "core/Graph.h"
#include "core/State.h"
#include "core/Condition.h"
#include "core/Automaton.h"

namespace TP {
namespace DiscreteModel {
	struct TransitionSystemLabel {
		TransitionSystemLabel(float cost_, const std::string& action_) 
			: cost(cost_)
			, action(action_)
			{}
		float cost;
		std::string action;
	};

	class BijectiveStateContainer {
		public:
			inline State& operator[](uint32_t state_ind) {return m_ind_to_state[state_ind];}
			inline const State& operator[](uint32_t state_ind) const {return m_ind_to_state[state_ind];}
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
		protected:
			std::vector<State> m_ind_to_state;
			std::unordered_map<State, uint32_t> m_state_to_ind;
	};

	class BijectiveStateObservationContainer : public BijectiveStateContainer {
		public:
			void addObservationToState(const std::string& observation, uint32_t state_ind) {
				if (state_ind > m_ind_to_observations.size()) {
					m_ind_to_observations.resize(state_ind);
				}
				m_ind_to_observations[state_ind].push_back(observation);
			}
			inline const std::vector<std::string>& getObservations(uint32_t state_ind) {return m_ind_to_observations[state_ind];}

		private:
			std::vector<std::vector<std::string>> m_ind_to_observations;
	};

	class TransitionSystem : public Graph<TransitionSystemLabel> {
		public:
			TransitionSystem() = default;
			TransitionSystem(const std::string& filepath);

			
			bool connect(const State& src_state, const State& dst_state, const TransitionSystemLabel& edge) {
				return Graph<TransitionSystemLabel>::connect(m_state_container.tryInsert(src_state).first, m_state_container.tryInsert(dst_state).first, edge);
			}

			virtual void print() const override;

		 	bool parseObservationAndEvaluate(const State& state, const std::string& observation) const;

			void mapStatesToLabels(const FormalMethods::Alphabet& alphabet);

			void listPropositions() const {
				LOG("Listing proposition names");
				for (const auto&[name, _] : m_propositions) PRINT(" - " << name);
			}

			const BijectiveStateContainer& getStateContainer() {return m_state_container;}

		private:
			void deserialize(const std::string& filepath);

		protected:
			const Condition& getProposition(const std::string& name) const {
				ASSERT(m_propositions.contains(name), "Proposition '" << name << "' was not found");
				return m_propositions.at(name);
			}

		protected:
		 	BijectiveStateObservationContainer m_state_container;
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
		FormalMethods::Alphabet alphabet;
	};

	class TransitionSystemGenerator {
		public:
			static std::shared_ptr<TransitionSystem> generate(TransitionSystemProperties& specs);
	};
} // namespace DiscreteModel
} // namespace TP