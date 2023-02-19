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

	class ObservationContainer {
		public:
			void addObservationToNode(Node node, const std::string& observation) {
				if (nodeInContainer(node)) {
					m_observations.resize(node);
				}
				m_observations[node].insert(observation);
			}
			inline const std::unordered_set<std::string>& getObservations(Node node) {
				return m_observations[node];
			}
			inline bool observe(Node node, const std::string& observation) const {
				ASSERT(nodeInContainer(node), "Node " << node << " is not in observation container");
				return m_observations[node].contains(observation);
			}
			void resize(std::size_t size) {m_observations.resize(size);}
			inline bool nodeInContainer(Node node) const {return node >= m_observations.size();}
		private:
			std::vector<std::unordered_set<std::string>> m_observations;
	};

	class TransitionSystem : public NodeGenericGraph<State, TransitionSystemLabel> {
		public:
			TransitionSystem() = default;
			TransitionSystem(const std::string& filepath);

			// Parses observation and evaluates it according to state
		 	bool parseAndObserve(const State& state, const std::string& observation) const;

			// Evalues observation based on pre-parsed labels
		 	inline bool observe(const State& state, const std::string& observation) const {return m_observation_container.observe(m_node_container[state], observation);}
		 	inline bool observe(Node node, const std::string& observation) const {return m_observation_container.observe(node, observation);}
			
			// TODO: Checks if the observation is in the alphabet, if not parse and observe. Cache the observation in observation container if cache is true
			bool smartObserve(Node node, const std::string& observation, bool cache = false);

			void addAlphabet(const FormalMethods::Alphabet& alphabet);

			virtual void print() const override;

			void listPropositions() const {
				LOG("Listing proposition names");
				for (const auto&[name, _] : m_propositions) PRINT(" - " << name);
			}

			const ObservationContainer& getObservationContainer() const {return m_observation_container;}

		private:
			void deserialize(const std::string& filepath);
			void addObservationsToNode(Node node, const FormalMethods::Alphabet& alphabet);

		protected:
			const Condition& getProposition(const std::string& name) const {
				ASSERT(m_propositions.contains(name), "Proposition '" << name << "' was not found");
				return m_propositions.at(name);
			}

		protected:
		  	FormalMethods::Alphabet m_alphabet;
		 	ObservationContainer m_observation_container;
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