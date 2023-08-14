#include "Condition.h"

#include<iostream>
#include<vector>
#include<unordered_map>

#include "core/State.h"
#include "tools/Logging.h"
#include "tools/Debug.h"

namespace GF {
namespace DiscreteModel {

	std::pair<bool, StateAccessCapture> _ConditionBase::subEvaluate(const State& state, const SubCondition& cond) const {
		//LOG("evaluating state: " << state.to_str());
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
					 	ASSERT(cond.lhs_type == ConditionArg::Variable || cond.lhs_type == ConditionArg::Label, "(operator 'Equals') lhs_type is not allowed");
						if (cond.lhs_type == ConditionArg::Variable) {
							eval = (!negate) ? cond.lhs == sac[getArgValues(cond.condition_name).label] : !(cond.lhs == sac[getArgValues(cond.condition_name).label]);
							return {eval, sac};
						} else if (cond.lhs_type == ConditionArg::Label) {
							eval = (!negate) ? sac[cond.lhs] == sac[getArgValues(cond.condition_name).label] : !(sac[cond.lhs] == sac[getArgValues(cond.condition_name).label]);
							return {eval, sac};
						}
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
									if (found) addArgValues(cond.condition_name, &state[label], label); // Access thru state since storing variable is not needed for excl eq
									return {(!negate) ? found : !found, sac};
								}
							case ConditionArg::Label:
							  	{
									auto[found, label] = state.argFindGroup(state[cond.rhs], cond.lhs);
									if (found) addArgValues(cond.condition_name, &state[label], label); // Access thru state since storing variable is not needed for excl eq
									return {(!negate) ? found : !found, sac};
								}
						}
				}
		}
		ASSERT(false, "Unrecognized logic path");
		return {false, sac};
	}	

	bool Condition::evaluate(const State& state) const {
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

	void Condition::serialize(GF::Serializer& szr) const {
        YAML::Emitter& out = szr.get();
		
		out << YAML::Key << "Name" << YAML::Value << m_name;
		out << YAML::Key << "Junction Type" << YAML::Value << m_junction_type;
		out << YAML::Key << "Sub Conditions" << YAML::Value; 
		out << YAML::BeginSeq;
		for (auto[lhs_type, rhs_type, condition_operator, lhs, rhs, condition_name, logical] : m_sub_conditions) {
			out << YAML::BeginMap;
			
			out << YAML::Key << "LHS Type" << YAML::Value << lhs_type;
			out << YAML::Key << "RHS Type" << YAML::Value << rhs_type;
			out << YAML::Key << "Condition Operator" << YAML::Value << condition_operator;
			out << YAML::Key << "LHS" << YAML::Value << lhs;
			out << YAML::Key << "RHS" << YAML::Value << rhs;
			out << YAML::Key << "Condition Name" << YAML::Value << condition_name;
			out << YAML::Key << "Logical" << YAML::Value << logical;


			out << YAML::EndMap;
		};
		out << YAML::EndSeq;
	}

	void Condition::deserialize(const GF::Deserializer& dszr) {
		const YAML::Node& node = dszr.get();
		m_name = node["Name"].as<std::string>();
		m_junction_type = static_cast<ConditionJunction>(node["Junction Type"].as<uint16_t>());

		m_sub_conditions.clear();
		const YAML::Node sub_condition_node = node["Sub Conditions"];
		for (auto it = sub_condition_node.begin(); it != sub_condition_node.end(); ++it) {
			const YAML::Node& sub_condition = *it;
			//LOG("key: " << it->first.as<std::string>());
			m_sub_conditions.emplace_back(
				static_cast<ConditionArg>(sub_condition["LHS Type"].as<uint16_t>()),
				sub_condition["LHS"].as<std::string>(),
				static_cast<ConditionOperator>(sub_condition["Condition Operator"].as<uint16_t>()),
				static_cast<ConditionArg>(sub_condition["RHS Type"].as<uint16_t>()),
				sub_condition["RHS"].as<std::string>(),
				sub_condition["Condition Name"].as<std::string>(),
				static_cast<ConditionLogical>(sub_condition["Logical"].as<uint16_t>())
			);
		}
	}

	// TransitionCondition

	bool TransitionCondition::evaluate(const State& pre_state, const State& post_state) const {
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

		if (m_excl_eq) {
			// Omit exclusion comparison labels:
			for (const auto& lbl : m_omit_excl_eq_labels) all_sac[lbl];

			// Apply force exclusion comparison labels	
			all_sac.removeAccess(m_force_excl_eq_labels);

			return pre_state.exclEquals(post_state, all_sac);
		}

		return true;
	}

} // namespace DiscreteModel
} // namespace GF