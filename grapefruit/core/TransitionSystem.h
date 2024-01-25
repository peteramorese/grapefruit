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

namespace GF {
namespace DiscreteModel {

    using Action = std::string;

	struct TransitionSystemLabel {
		public:
			typedef float cost_t;
			typedef Action action_t;
		public:
			TransitionSystemLabel(float cost_, const Action& action_) 
				: cost(cost_)
				, action(action_)
				{}
			float cost;
			Action action;

			// Cost Conversion operator;
			operator float() const {return cost;}
			operator float&&() {return std::move(cost);}

			operator Action&() {return action;}
			operator const Action&() const {return action;}
			operator Action&&() {return std::move(action);}

			std::string to_str() const {return "(action: " + action + ", cost: " + std::to_string(cost) + ")";}
	};

	/// @brief Maps the node to a set of observations that have been added to the alphabet. This container
	/// allows for checking if an observation is true at a certain state in constant time
	class ObservationContainer {
		public:
			void addObservationToNode(Node node, const std::string& observation) {
				if (!nodeInContainer(node)) {
					resize(node + 1);
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
			inline void resize(std::size_t size) {m_observations.resize(size);}
			inline bool nodeInContainer(Node node) const {return node < m_observations.size();}
			void print() const {
				LOG("Printing observation container");
				for (uint32_t i=0; i<m_observations.size(); ++i) {
					std::string obs_str = std::string();
					for (auto obs : m_observations[i]) {
						obs_str += obs + ", ";
					}
					PRINT_NAMED("Node " << i << " contains observations", obs_str);
				}
			}
		private:
			std::vector<std::unordered_set<std::string>> m_observations;
	};

	class TransitionSystem : public NodeGenericGraph<State, TransitionSystemLabel, Node, true, true> {
		public:
		 	/// @brief  For deserialization only (undefined state)
		 	TransitionSystem() {}

			/// @brief Create with a state space that has already been generated
			/// @param ss State space
			TransitionSystem(const std::shared_ptr<StateSpace>& ss) 
				: m_ss(ss)
				{};
			virtual ~TransitionSystem() {};

			// Parses observation and evaluates it according to state
		 	bool parseAndObserve(const State& state, const FormalMethods::Observation& observation) const;

			// Evalues observation based on pre-parsed labels
		 	inline bool observe(const State& state, const FormalMethods::Observation& observation) const {return m_observation_container.observe(m_node_container[state], observation);}
		 	inline bool observe(Node node, const FormalMethods::Observation& observation) const {return m_observation_container.observe(node, observation);}
			
			// TODO: Checks if the observation is in the alphabet, if not parse and observe. Cache the observation in observation container if cache is true
			bool smartObserve(Node node, const FormalMethods::Observation& observation, bool cache = false);

			void addAlphabet(const FormalMethods::Alphabet& alphabet);

			inline std::weak_ptr<StateSpace> getStateSpace() const {return std::weak_ptr(m_ss);}

			void print() const;
			void rprint() const;

			void listPropositions() const {
				LOG("Listing proposition names");
				for (const auto&[name, _] : m_propositions) 
					PRINT(" - " << name);
			}

			const ObservationContainer& getObservationContainer() const {return m_observation_container;}

			void addProposition(const Condition& prop) {m_propositions[prop.getName()].push_back(prop);}

            void serialize(GF::Serializer& szr) const;
            void deserialize(const GF::Deserializer& dszr);

			/// @brief Get the proposition by name
			/// @param label Proposition label
			/// @return Proposition condition
			inline const std::vector<Condition>& getProposition(const std::string& label) const {
				ASSERT(m_propositions.contains(label), "Proposition '" << label << "' was not found");
				return m_propositions.at(label);
			}

			/// @brief Determines if a proposition is true at a given state
			/// @param prop_label Proposition name
			/// @param state Test state
			/// @return True if proposition holds, false otherwise
			bool evaluatePropositionAtState(const std::string& prop_label, const State& state) const {
				for (const Condition& prop : getProposition(prop_label)) {
					if (prop.evaluate(state))
						return true;
				}
				return false;
			}

		private:
			void addObservationsToNode(Node node, const FormalMethods::Alphabet& alphabet);

		protected:
			std::shared_ptr<StateSpace> m_ss;

		  	FormalMethods::Alphabet m_alphabet;
		 	ObservationContainer m_observation_container;

			// Key: Proposition label, Value: array of propositions have have that label
			std::unordered_map<std::string, std::vector<Condition>> m_propositions;
			
			friend class TransitionSystemGenerator;
	};

 
	// Generator
	struct TransitionSystemProperties {
		TransitionSystemProperties(const std::shared_ptr<StateSpace>& ss_) : ss(ss_), init_state(ss_.get()) {}

		std::vector<Condition> propositions;
		std::vector<TransitionCondition> conditions;
		State init_state;
		bool deterministic_action_labels = true;
		FormalMethods::Alphabet alphabet;
		std::shared_ptr<StateSpace> ss;
	};

	class TransitionSystemGenerator {
		public:
			static std::shared_ptr<TransitionSystem> generate(TransitionSystemProperties& specs);
		private:
			static std::string getDeterministicActionLabel(const Action& action, Node dst);
	};
} // namespace DiscreteModel
} // namespace GF