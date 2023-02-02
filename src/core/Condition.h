#pragma once
#include<vector>
#include<string>
#include<unordered_map>

#include "tools/Logging.h"

namespace DiscreteModel {

	class State;
	class StateAccessCapture;

	enum class ConditionType {
		// Type of condition (for TransitionCondition)
		Pre, 			// Applies to the anterior state
		Post, 			// Applies to the posterior state
	};

	enum class ConditionArg {
		None = 0,

		// Operand
		Variable,		// Argument is a 'Variable'
		Label,			// Argument is a 'Label'
		Domain,			// Argument is a 'Domain'
		Group, 			// Argument is a 'Group'

		// Argument
		ArgVariable,	// Argument is the variable determined from the ARG_FIND operation performed in the precondition
		ArgLabel		// Argument is the label determined from the ARG_FIND operation performed in the precondition

	};

	enum class ConditionLogical {
		// Logical
		True,			// Argument must evaluate to 'true'
		Negate, 		// Argument must evaluate to 'false'
	};
	
	enum class ConditionJunction {
		Conjunction,	// Conditions are conjoined
		Disjunction,	// Conditions are disjoined
	};

	enum class ConditionOperator {
		// Operator
		Equals,			// Check if a 'Label' equals a 'Variable'
		InDomain,			// Check if a 'Variable' or the variable inside a 'Label' is inside a 'Domain'
		ArgFind,		// Check if an instance of 'Variable' or the variable inside a 'Label' is found within a 'Group'. If found, the variable/label is stored
		EqualsArg		// Check if a 'Label' or 'Variable' is equal to the found argument
	};

	// Houses common types and methods for Condition and TransitionCondition
	class _ConditionBase {
		protected:
			struct SubCondition {
				SubCondition() = delete;
				SubCondition(ConditionArg lhs_type_, const std::string& lhs_, ConditionOperator condition_operator_, ConditionArg rhs_type_, const std::string& rhs_, const std::string& condition_name_, ConditionLogical logical_)
					: lhs_type(lhs_type_)
					, rhs_type(rhs_type_)
					, condition_operator(condition_operator_)
					, lhs(lhs_)
					, rhs(rhs_)
					, condition_name(condition_name_)
					, logical(logical_) {}
				ConditionArg lhs_type;
				ConditionArg rhs_type;
				ConditionOperator condition_operator;
				std::string lhs, rhs;
				std::string condition_name;
				ConditionLogical logical;
			};
			struct ArgumentValues {
				ArgumentValues(const std::string* variable_, const std::string& label_) : variable(variable_), label(label_) {}
				ArgumentValues(const ArgumentValues& other) = default;
				const std::string* variable;
				std::string label;
				bool operator==(const ArgumentValues& other) const {return *(variable) == *(other.variable) && label == other.label;}
			};

		protected:
			std::unordered_map<std::string, ArgumentValues> m_arg_values;

		private:
			void addArgValues(const std::string& condition_name, const std::string* variable, const std::string& label) {
				m_arg_values.emplace(std::piecewise_construct, std::forward_as_tuple(condition_name), std::forward_as_tuple(variable, label));
			}
			const ArgumentValues& getArgValues(const std::string& condition_name) const {
				ASSERT(m_arg_values.find(condition_name) != m_arg_values.end(), "Condition name: " << condition_name << " unrecognized");
				return m_arg_values.at(condition_name);
			}

		protected:
		  	_ConditionBase() = default;
		 	virtual ~_ConditionBase() {}
			std::pair<bool, StateAccessCapture> subEvaluate(const State& state, const SubCondition& cond);
			inline void clearArgValues() {m_arg_values.clear();}
			virtual void print() const = 0;
	};

	class Condition : protected _ConditionBase {
		public:
		 	Condition(ConditionJunction junction_type = ConditionJunction::Conjunction) : m_junction_type(junction_type) {}

			inline void addCondition(ConditionArg lhs_type, const std::string& lhs, ConditionOperator condition_operator, ConditionArg rhs_type, const std::string& rhs, ConditionLogical logical = ConditionLogical::True) {
				m_sub_conditions.emplace_back(lhs_type, lhs, condition_operator, rhs_type, rhs, "", logical);
			}
			inline void addCondition(ConditionArg lhs_type, const std::string& lhs, ConditionOperator condition_operator, ConditionArg rhs_type, const std::string& rhs, const std::string& condition_name, ConditionLogical logical = ConditionLogical::True) {
				m_sub_conditions.emplace_back(lhs_type, lhs, condition_operator, rhs_type, rhs, condition_name, logical);
			}

			inline void setName(const std::string& name) {m_name = name;}
			inline const std::string& getName() const {return m_name;}

			bool evaluate(const State& state);

			// TODO
			virtual void print() const override {}
		private:
		 	std::string m_name;
			std::vector<SubCondition> m_sub_conditions;
			ConditionJunction m_junction_type = ConditionJunction::Conjunction;
	};

	class TransitionCondition : protected _ConditionBase {
		public:	
			TransitionCondition(const std::string& action_label, float action_cost, ConditionJunction pre_junction_type = ConditionJunction::Conjunction, ConditionJunction post_junction_type = ConditionJunction::Conjunction) 
				: m_pre_junction_type(pre_junction_type)
				, m_post_junction_type(post_junction_type) 
				, m_action_label(action_label)
				, m_action_cost(action_cost)
				{}

			inline void addCondition(ConditionType type, ConditionArg lhs_type, const std::string& lhs, ConditionOperator condition_operator, ConditionArg rhs_type, const std::string& rhs, ConditionLogical logical = ConditionLogical::True) {
				ASSERT(condition_operator != ConditionOperator::ArgFind && condition_operator != ConditionOperator::EqualsArg, "Must provide a name when using operator 'ArgFind' or 'ArgEquals'");
				switch (type) {
					case ConditionType::Pre:
						m_pre_conditions.emplace_back(lhs_type, lhs, condition_operator, rhs_type, rhs, "", logical);
						return;
					case ConditionType::Post:
						m_post_conditions.emplace_back(lhs_type, lhs, condition_operator, rhs_type, rhs, "", logical);
						return;
				}
			}
			inline void addCondition(ConditionType type, ConditionArg lhs_type, const std::string& lhs, ConditionOperator condition_operator, ConditionArg rhs_type, const std::string& rhs, const std::string& condition_name, ConditionLogical logical = ConditionLogical::True) {
				switch (type) {
					case ConditionType::Pre:
                        m_pre_conditions.emplace_back(lhs_type, lhs, condition_operator, rhs_type, rhs, condition_name, logical);
						return;
					case ConditionType::Post:
                        m_post_conditions.emplace_back(lhs_type, lhs, condition_operator, rhs_type, rhs, condition_name, logical);
						return;
				}
			}
			inline void toggleExclusionComparision(bool excl_eq) {m_excl_eq = excl_eq;}

			inline const std::string& getActionLabel() const {return m_action_label;}
			inline float getActionCost() const {return m_action_cost;}

			bool evaluate(const State& pre_state, const State& post_state);

			// TODO
			virtual void print() const override {}

		private:
			std::vector<SubCondition> m_pre_conditions;
			std::vector<SubCondition> m_post_conditions;
			ConditionJunction m_pre_junction_type = ConditionJunction::Conjunction;
			ConditionJunction m_post_junction_type = ConditionJunction::Conjunction;
			bool m_excl_eq = true;
			std::string m_action_label;
			float m_action_cost;
	};


}