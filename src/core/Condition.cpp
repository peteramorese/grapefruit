#include "Condition.h"

#include<iostream>
#include<vector>
#include<unordered_map>

#include "core/State.h"
#include "tools/Logging.h"

namespace DiscreteModel {
	std::pair<bool, StateAccessCapture> _ConditionBase::subEvaluate(const State& state, const SubCondition& cond) {
		
		StateAccessCapture sac = state.getStateAccessCapture();

		bool eval = false;
		bool negate = cond.logical == ConditionLogical::Negate;
		switch (cond.condition_operator) {
			case ConditionOperator::Equals: 
			 	ASSERT(cond.rhs_type == ConditionArg::Variable
						|| cond.rhs_type == ConditionArg::Label
						|| cond.rhs_type == ConditionArg::ArgVariable
						|| cond.rhs_type == ConditionArg::ArgLabel
						, "(operator 'Equals') rhs_type must of type 'Variable', 'Label', 'ArgVariable', or 'ArgLabel'");
			 	switch (cond.rhs_type) {
					case ConditionArg::Variable:
					 	ASSERT(cond.lhs_type == ConditionArg::Label, "(operator 'Equals') lhs_type is not allowed");
						eval = sac[cond.lhs] == cond.rhs;
						return {eval, sac};
					case ConditionArg::Label:
					 	ASSERT(cond.lhs_type == ConditionArg::Variable || cond.lhs_type == ConditionArg::Label, "(operator 'Equals') lhs_type is not allowed");
						if (cond.lhs_type == ConditionArg::Variable) {
							eval = (!negate) ? sac[cond.rhs] == cond.lhs : !(sac[cond.rhs] == cond.lhs);
							return {eval, sac};
						} else if (cond.lhs_type == ConditionArg::Label) {
							eval = (!negate) ? sac[cond.lhs] == sac[cond.rhs] : !(sac[cond.lhs] == sac[cond.rhs]);
							return {eval, sac};
						}
					case ConditionArg::ArgVariable:
						ASSERT(!cond.condition_name.empty(), "(operator 'Equals') Comparisons with arg values must match a condition name");
					 	ASSERT(cond.lhs_type == ConditionArg::Variable || cond.lhs_type == ConditionArg::Label, "(operator 'Equals') lhs_type is not allowed");
						if (cond.lhs_type == ConditionArg::Variable) {
							eval = (!negate) ? cond.lhs == *(getArgValues(cond.condition_name).variable) : !(cond.lhs == *(getArgValues(cond.condition_name).variable));
							return {eval, sac};
						} else if (cond.lhs_type == ConditionArg::Label) {
							eval = (!negate) ? sac[cond.lhs] == *(getArgValues(cond.condition_name).variable) : !(sac[cond.lhs] == *(getArgValues(cond.condition_name).variable));
							return {eval, sac};
						}
					case ConditionArg::ArgLabel:
						ASSERT(!cond.condition_name.empty(), "(operator 'Equals') Comparisons with arg values must match a condition name");
					 	ASSERT(cond.lhs_type == ConditionArg::Label, "(operator 'Equals') lhs_type is not allowed");
						// ADD VARIABLE CASE
						eval = (!negate) ? cond.lhs == getArgValues(cond.condition_name).label : !(cond.lhs == getArgValues(cond.condition_name).label);
						return {eval, sac};
				}
			case ConditionOperator::InDomain:
			 	ASSERT(cond.rhs_type == ConditionArg::Domain, "(operator 'InDomain') rhs_type must be 'Domain'");
			 	ASSERT(cond.lhs_type == ConditionArg::Label || cond.lhs_type == ConditionArg::Group, "(operator 'InDomain') lhs_type must be 'Label' or 'Group'");
				switch (cond.lhs_type) {
					case ConditionArg::Label:
					  	eval = (!negate) ? state.getStateSpace()->inDomain(cond.rhs, sac[cond.lhs]) : !(state.getStateSpace()->inDomain(cond.rhs, sac[cond.lhs]));
					 	return {eval, sac};
					case ConditionArg::Group: 
						const std::unordered_set<std::string>& group = state.getStateSpace()->getGroup(cond.lhs);
						for (const auto& label : group) {
							if (!state.getStateSpace()->inDomain(cond.rhs, sac[label])) {
								return {(!negate) ? false : true, sac};
							}
						}
						return {(!negate) ? true : false, sac};
				}
			case ConditionOperator::ArgFind:
			 	ASSERT(cond.lhs_type == ConditionArg::Label || cond.lhs_type == ConditionArg::Group, "(operator 'ArgFind' lhs_type must be 'Label' or 'Group'");
				switch (cond.lhs_type) {
					case ConditionArg::Label:
						ASSERT(cond.rhs_type == ConditionArg::None, "(operator 'ArgFind') When finding and storing a variable using ArgFind with lhs_type 'Label', the rhs_type must be 'None'");
						addArgValues(cond.condition_name, &state[cond.lhs], cond.lhs); // Access thru state since storing variable is not needed for excl eq
						return {true, sac};
					case ConditionArg::Group:
						ASSERT(cond.rhs_type == ConditionArg::Variable || cond.rhs_type == ConditionArg::Label, "(operator 'ArgFind') ArgFind on a 'Group' can only search for rhs_type 'Variable' or rhs_type 'Label'");
						switch (cond.rhs_type) {
							case ConditionArg::Variable:
							  	{
									auto[found, label] = state.argFindGroup(cond.rhs, cond.lhs);
									if (found) addArgValues(cond.condition_name, &sac[label], label);
									return {(!negate) ? found : !found, sac};
								}
							case ConditionArg::Label:
							  	{
									auto[found, label] = state.argFindGroup(sac[cond.rhs], cond.lhs);
									if (found) addArgValues(cond.condition_name, &sac[label], label);
									return {(!negate) ? found : !found, sac};
								}
						}
				}
		}
		ASSERT(false, "Unrecognized logic path");
		return {false, sac};
	}	

	bool Condition::evaluate(const State& state) {
		clearArgValues();
	
		if (m_junction_type == ConditionJunction::Conjunction) {
			for (const auto& cond : m_sub_conditions) {
				auto[result, _] = subEvaluate(state, cond);
				if (!result) {return false;}
			}
			return true;
		} else {
			for (const auto& cond : m_sub_conditions) {
				auto[result, _] = subEvaluate(state, cond);
				if (result) {return true;}
			}
			return false;
		}
	}

	bool TransitionCondition::evaluate(const State& pre_state, const State& post_state) {
		clearArgValues();
	
	 	// Pre-Conditions
		bool pre_eval = false;
		StateAccessCapture all_sac = pre_state.getStateAccessCapture();
		if (m_pre_junction_type == ConditionJunction::Conjunction) {
			for (const auto& cond : m_pre_conditions) {
				auto[result, single_sac] = subEvaluate(pre_state, cond);
				all_sac |= single_sac;
				if (!result) {return false;}
			}
		} else {
			for (const auto& cond : m_pre_conditions) {
				auto[result, single_sac] = subEvaluate(pre_state, cond);
				all_sac |= single_sac;
				if (result) {
					pre_eval = true;
					break;
				}
			}
			if (!pre_eval) return false;
		}

	 	// Post-Conditions
		bool post_eval = false;
		if (m_post_junction_type == ConditionJunction::Conjunction) {
			for (const auto& cond : m_post_conditions) {
				auto[result, single_sac] = subEvaluate(post_state, cond);
				all_sac |= single_sac;
				if (!result) {return false;}
			}
		} else {
			for (const auto& cond : m_post_conditions) {
				auto[result, single_sac] = subEvaluate(post_state, cond);
				all_sac |= single_sac;
				if (result) {
					post_eval = true;
					break;
				}
			}
			if (!post_eval) return false;
		}
		return (m_excl_eq) ? pre_state.exclEquals(post_state, all_sac) : true;
	}
} // namespace DiscreteModel