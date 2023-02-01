#pragma once
#include<vector>
#include "state.h"
#include<string>
#include<unordered_map>

namespace DiscreteModel {
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
	}

	enum class ConditionOperator {
		// Operator
		Equals,			// Check if a 'Label' equals a 'Variable'
		InDomain,			// Check if a 'Variable' or the variable inside a 'Label' is inside a 'Domain'
		ArgFind,		// Check if an instance of 'Variable' or the variable inside a 'Label' is found within a 'Group'. If found, the variable/label is stored
		EqualsArg		// Check if a 'Label' or 'Variable' is equal to the found argument
	}

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
					, condition_label(condition_label_)
					, logical(logical_) {}
				ConditionArg lhs_type;
				ConditionArg rhs_type;
				ConditionOperator condition_operator;
				std::string lhs, rhs;
				std::string condition_label;
				ConditionLogical logical;
			};
		protected:
			bool subEvaluate(const State& state, const subCondition& cond);
			virtual void print() const = 0;
	}

	class Condition : protected _ConditionBase {
		public:
		 	Condition(ConditionJunction junction_type) : m_junction_type(junction_type) {}

			inline void addCondition(ConditionArg lhs_type, const std::string& lhs, ConditionOperator condition_operator, ConditionArg rhs_type, const std::string& rhs, ConditionLogical logical = ConditionLogical::True) {
				m_sub_conditions.emplace(lhs_type, lhs, condition_operator, rhs_type, rhs, "", logical);
			}
			inline void addCondition(ConditionArg lhs_type, const std::string& lhs, ConditionOperator condition_operator, ConditionArg rhs_type, const std::string& rhs, const std::string& condition_name, ConditionLogical logical = ConditionLogical::True) {
				m_sub_conditions.emplace(lhs_type, lhs, condition_operator, rhs_type, rhs, condition_name, logical);
			}
			inline void setName(const std::string& name) {m_name = name;}
			inline const std::string& getName() const {return m_name;}

			bool evaluate(const State& state) const;

			virtual void print() const override;
		private:
		 	std::string m_name;
			std::vector<SubCondition> m_sub_conditions;
			ConditionJunction m_junction_type = ConditionJunction::Conjunction;
	};

	class TransitionCondition {
		private:
			struct ArgumentProperties {
				bool is_set;
				const std::string* variable;
				const std::string* label;
				bool operator==(const ArgumentProperties& other) const {return is_set == other.is_set && *(variable) == *(other.variable) && *(label) == *(other.label);}
			};
		private:
			std::vector<SubCondition> m_pre_conditions;
			std::vector<SubCondition> m_post_conditions;
			ConditionJunction m_pre_junction_type = ConditionJunction::Conjunction;
			ConditionJunction m_post_junction_type = ConditionJunction::Conjunction;
			std::unordered_map<std::string, ArgumentProperties> m_arg_variables;
			std::unordered_map<std::string, ArgumentProperties> m_arg_labels;
			bool m_excl_eq = true;
			std::string m_action_label;
			float m_action_cost;


			std::vector<std::pair<bool, std::string>> arg_L;
			std::unordered_map<std::string, int> arg_L_labels;
			std::vector<arg_V_struct> arg_V;
			std::unordered_map<std::string, int> arg_V_labels;
			std::pair<bool, std::string> arg_L_i;
			arg_V_struct arg_V_i;
			void sub_print(const std::vector<subCondition>& p_c) const;
			std::string label;
		public:	
			TransitionCondition(ConditionJunction pre_junction_type, ConditionJunction post_junction_type, const std::string& action_label, float action_cost) 
				: m_pre_junction_type(pre_junction_type)
				, m_post_junction_type(post_junction_type) 
				, m_action_label(action_label)
				, m_action_cost(action_cost)
				{}

			inline void addCondition(ConditionType type, ConditionArg lhs_type, const std::string& lhs, ConditionOperator condition_operator, ConditionArg rhs_type, const std::string& rhs, ConditionLogical logical = ConditionLogical::True) {
				switch (type) {
					case ConditionType::Pre:
						m_pre_conditions.emplace(lhs_type, lhs, condition_operator, rhs_type, rhs, "", logical);
						return;
					case ConditionType::Post:
						m_post_conditions.emplace(lhs_type, lhs, condition_operator, rhs_type, rhs, "", logical);
						return;
				}
			}
			inline void addCondition(ConditionType type, ConditionArg lhs_type, const std::string& lhs, ConditionOperator condition_operator, ConditionArg rhs_type, const std::string& rhs, const std::string& condition_name, ConditionLogical logical = ConditionLogical::True) {
				switch (type) {
					case ConditionType::Pre:
                        m_pre_conditions.emplace(lhs_type, lhs, condition_operator, rhs_type, rhs, condition_name, logical);
						return;
					case ConditionType::Post:
                        m_post_conditions.emplace(lhs_type, lhs, condition_operator, rhs_type, rhs, condition_name, logical);
						return;
				}
			}
			inline void toggleExclusionComparision(bool excl_eq) {m_excl_eq = excl_eq;}

			inline const std::string& getActionLabel() const {return m_action_label;}
			inline float getActionCost() const {return m_action_cost;}

			bool evaluate(const State& pre_state, const State& post_state) const;

			virtual void print() const override;
	};


}