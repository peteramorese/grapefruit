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

	class BijectiveObservationContainer {
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

	class TransitionSystem : public NodeGenericGraph<State, TransitionSystemLabel> {
		public:
			TransitionSystem() = default;
			TransitionSystem(const std::string& filepath);

			
			virtual void print() const override;

		 	bool parseObservationAndEvaluate(const State& state, const std::string& observation) const;

			void mapStatesToLabels(const FormalMethods::Alphabet& alphabet);

			void listPropositions() const {
				LOG("Listing proposition names");
				for (const auto&[name, _] : m_propositions) PRINT(" - " << name);
			}

			const BijectiveObservationContainer& getStateContainer() const {return m_observation_container;}

		private:
			void deserialize(const std::string& filepath);

		protected:
			const Condition& getProposition(const std::string& name) const {
				ASSERT(m_propositions.contains(name), "Proposition '" << name << "' was not found");
				return m_propositions.at(name);
			}

		protected:
		 	BijectiveObservationContainer m_observation_container;
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