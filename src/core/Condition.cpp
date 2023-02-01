#include "Condition.h"

#include<iostream>
#include<vector>
#include<unordered_map>

#include "core/State.h"
#include "tools/Logging.h"

namespace DiscreteModel {
	bool _ConditionBase::subEvaluate(const State& state, const SubCondition& cond) const {
		bool sub_eval = false;
		bool negate = cond.logical == ConditionLogical::Negate;
		switch (cond.condition_operator) {
			case ConditionOperator::Equals: 
			 	switch (cond.rhs_type) {
					case ConditionArg::Variable:
					 	ASSERT(cond.lhs_type == ConditionArg::Label, "(operator 'Equals') lhs_type is not allowed");
						return state[cond.lhs] == cond.rhs;
					case ConditionArg::Label:
					 	ASSERT(cond.lhs_type == ConditionArg::Variable || cond.lhs_type == ConditionArg::Label, "(operator 'Equals') lhs_type is not allowed");
						if (cond.lhs_type == ConditionArg::Variable) {
							return (!negate) ? state[cond.rhs] == cond.lhs : !(state[cond.rhs] == cond.lhs);
						} else if (cond.lhs_type == ConditionArg::Label) {
							return (!negate) ? state[cond.lhs] == state[cond.rhs] : !(state[cond.lhs] == state[cond.rhs]);
						}
				}
			case ConditionOperator::InDomain:
			 	ASSERT(cond.rhs_type == ConditionArg::Domain, "(operator 'InDomain') rhs_type must be 'Domain'");
			 	ASSERT(cond.lhs_type == ConditionArg::Label || cond.lhs_type == ConditionArg::Group, "(operator 'InDomain') lhs_type must be 'Label' or 'Group'");
				//bool found;
				//std::vector<std::string> in_domains;
				//std::string dom_var;
				switch (cond.lhs_type) {
					case ConditionArg::Label:
					 	return (!negate) ? state.getStateSpace()->inDomain(cond.rhs, state[cond.lhs]) : !(state.getStateSpace()->inDomain(cond.rhs, state[cond.lhs]));
					case ConditionArg::Group: 
						const std::unordered_set<std::string>& group = state.getStateSpace()->getGroup(cond.lhs);
						for (const auto& label : group) {
							if (!state.getStateSpace()->inDomain(cond.rhs, state[label])) {
								return (!negate) ? false : true;
							}
						}
						return (!negate) ? true : false;
				}
				break;
			case ARG_FIND:
				//if (cond.ARG_1_TYPE == GROUP || cond.ARG_1_TYPE == LABEL) {
				arg_L_i.first = false;
				sub_eval = false;
				switch (cond.ARG_1_TYPE) {
					case GROUP:
						switch (cond.ARG_2_TYPE) {
							case VAR:
								sub_eval = state->argFindGroup(cond.arg_2, cond.arg_1, arg_dimension_label);
								if (sub_eval) {
									arg_L_i.first = true;
									arg_L_i.second = arg_dimension_label;
								}
								break;
							case LABEL:
								temp_var = state->getVar(cond.arg_2);
								sub_eval = state->argFindGroup(temp_var, cond.arg_1, arg_dimension_label);
								if (sub_eval) {
									arg_L_i.first = true;
									arg_L_i.second = arg_dimension_label;
								}
								break;
							default:
								std::cout<<"Error: Condition Syntax error for operator ARG_FIND\n";
						}
						break;
					case LABEL:
						if (cond.ARG_2_TYPE == NONE){
							sub_eval = true;
							arg_V_i.is_set = true;
							arg_V_i.var = state->getVar(cond.arg_1);
							arg_V_i.label = cond.arg_1;
						} else {
							std::cout<<"Error: Condition Syntax error for operator ARG_FIND\n";
						}
						break;
					default:
						std::cout<<"Error: Condition Syntax error for operator ARG_FIND\n";
				} 
				break;
			case ARG_EQUALS:
				//if (cond.ARG_1_TYPE == ARG_L) {
				switch (cond.ARG_1_TYPE){
					case ARG_L:
						switch (cond.ARG_2_TYPE) {
							case VAR:
								if (arg_L_i.first) {
									sub_eval = state->getVar(arg_L_i.second) == cond.arg_2;
								} else {
									std::cout<<"Error: ARG_FIND either did not succeed or was not called. Cannot call ARG_EQUALS\n";
								}
								break;
							case LABEL:
								if (arg_L_i.first) {
									sub_eval = state->getVar(arg_L_i.second) == state->getVar(cond.arg_2);
								} else {
									std::cout<<"Error: ARG_FIND either did not succeed or was not called. Cannot call ARG_EQUALS ARGL LAB\n";
								}
								break;
							default:
								std::cout<<"Error: Condition Syntax error for operator ARG_EQUALS\n";
						}
						break;
					case ARG_V:
						if (arg_V_i.is_set) {
							//if (cond.ARG_2_TYPE == VAR){
							switch (cond.ARG_2_TYPE){
								case VAR:
									sub_eval = arg_V_i.var == cond.arg_2;
									break;
								case LABEL:
									sub_eval = arg_V_i.var == state->getVar(cond.arg_2);
									break;
								default:
									std::cout<<"Error: Condition Syntax error for operator ARG_EQUALS\n";
							} 
						} else {
							std::cout<<"Error: ARG_FIND either did not succeed or was not called. Cannot call ARG_EQUALS ARGV\n";
						}
						break;
					default:
						std::cout<<"Error: Condition Syntax error for operator ARG_EQUALS\n";
				} 
				break;
			default:
				std::cout<<"Error: Condition Syntax error for operator\n";
		}
		if (cond.LOGICAL == NEGATE) {
			sub_eval = !sub_eval;
		}
		return sub_eval;
	}	

	bool Condition::evaluate(const State* pre_state, const State* post_state) {
		if (tautology) {
			return true;
		}
		bool eval = false;
		bool pre_eval;
		bool exit = false;
		arg_L.clear();
		arg_L.resize(0);
		arg_V.clear();
		arg_V.resize(0);
		if (pr_c.size() > 0) {
			switch (pre_cond_junct) {
				case CONJUNCTION:
					pre_eval = true;
					break;
				case DISJUNCTION:
					pre_eval = false;
					break;
			}
			for (int i=0; i<pr_c.size(); i++){
				arg_L_i.second = "no_arg";
				arg_V_i.var = "no_arg";
				bool pre_eval_i = subEvaluate(pre_state, pr_c[i]);
				if (pr_c[i].OPERATOR == ARG_FIND) {
					switch (pr_c[i].ARG_1_TYPE){
						case GROUP:
							arg_L.push_back(arg_L_i);
							// Map the arguments to the corresponding condition label
							arg_L_labels[pr_c[i].condition_label] = arg_L.size()-1;
							break;
						case LABEL:
							arg_V.push_back(arg_V_i);
							arg_V_labels[pr_c[i].condition_label] = arg_V.size()-1;
							break;
					}
					
				}
				switch (pre_cond_junct) {
					case CONJUNCTION:
						pre_eval = pre_eval && pre_eval_i;
						if (pre_eval) {
							break;
						} else {
							return false;
						}
						break;
					case DISJUNCTION:
						pre_eval = pre_eval || pre_eval_i;
						if (pre_eval) {
							exit = true;
						} 
						break;
				}	
				if (exit) {
					break;
				}
			}
		} else {
			pre_eval = true;
		}
	//postcondition:
		exit = false;
		if (!pre_eval) {
			return false;
		}
		bool post_eval;
		bool eq_eval;
		std::vector<std::string> excl_dim_labels;
		switch (post_cond_junct) {
			case CONJUNCTION:
				post_eval = true;
				break;
			case DISJUNCTION:
				post_eval = false;
				break;
		}
		for (int i=0; i<ps_c.size(); i++){
			if (ps_c[i].ARG_1_TYPE == ARG_L) {
				if (ps_c[i].condition_label != FILLER) {
					int arg_L_ind = arg_L_labels[ps_c[i].condition_label];
					arg_L_i = arg_L[arg_L_ind];
				} else {
					std::cout<<"Error: Post condition argument needs a precondition label to refer to\n";
				}
			}
			if (ps_c[i].ARG_1_TYPE == ARG_V) {
				if (ps_c[i].condition_label != FILLER) {
					int arg_V_ind = arg_V_labels[ps_c[i].condition_label];
					arg_V_i = arg_V[arg_V_ind];
				} else {
					std::cout<<"Error: Post condition argument needs a precondition label to refer to\n";
				}
			}
			bool post_eval_i = subEvaluate(post_state, ps_c[i]);
			switch (ps_c[i].ARG_1_TYPE) {
				case LABEL:
				// BUG HERE vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
					excl_dim_labels.push_back(ps_c[i].arg_1);
					break;
				case ARG_L:
					if (arg_L_i.first) {
						excl_dim_labels.push_back(arg_L_i.second);
					} else {
						std::cout<<"Error: Argument not set\n";
					}
					break;
				case ARG_V:
					if (arg_V_i.is_set) {
						excl_dim_labels.push_back(arg_V_i.label);
					} else {
						std::cout<<"Error: Argument not set\n";
					}

			}
			switch (post_cond_junct) {
				case CONJUNCTION:
					post_eval = post_eval && post_eval_i;
					if (post_eval) {
						break;
					} else {
						return false;
					}
				case DISJUNCTION:
					post_eval = post_eval || post_eval_i;
					if (post_eval) {
						exit = true;
					} else {
						break;
					}
			}	
			if (exit) {
				break;
			}
		}
		if (excl_equals) {
			eq_eval = pre_state->exclEquals(post_state, excl_dim_labels);
		} else {
			eq_eval = true;
		}
		// pre_eval: Are the preconditions satisfied?
		// post_eval: Are the post conditions satisfied?
		// eq_eval: Are the other unmentioned dimensions still equal between states?
		// All of these must be true for the condition to be satisfied
		eval = pre_eval && post_eval && eq_eval;
		return eval;
	}

		void Condition::sub_print(const std::vector<subCondition>& p_c) const {
			for (int i=0; i<p_c.size(); i++){
				std::cout<<"   -"<<i+1<<") ";
				bool logi = p_c[i].LOGICAL;
				switch (p_c[i].ARG_1_TYPE) {
					case LABEL: 
						std::cout<<"Dimension Label '"<<p_c[i].arg_1<<"' ";
						break;
					case GROUP:
						std::cout<<"Dimension Group '"<<p_c[i].arg_1<<"' ";
						break;
					case ARG_L:
						std::cout<<"Found Argument Label ";
						break;
					case ARG_V:
						std::cout<<"Found Argument Variable ";
						break;
				}
				switch (p_c[i].OPERATOR) {
					case EQUALS:
						if (logi) {
							std::cout<<"is equal to ";
						} else {
							std::cout<<"is not equal to ";
						}
						break;
					case IN_DOM:
						if (logi) {
							std::cout<<"is in ";
						} else {
							std::cout<<"is not in ";
						}
						break;
					case ARG_FIND:
						if (logi) {
							std::cout<<"found ";
						} else {
							std::cout<<"didn't find ";
						}
						break;
					case ARG_EQUALS:
						if (logi) {
							std::cout<<"is equal to ";
						} else {
							std::cout<<"is not equal to ";
						}
				}
				switch (p_c[i].ARG_2_TYPE) {
					case LABEL:
						std::cout<<"Dimension Label: '"<<p_c[i].arg_2<<"'\n";
						break;
					case VAR:
						std::cout<<"Dimension Variable: '"<<p_c[i].arg_2<<"'\n";
						break;
					case DOM:
						std::cout<<"Domain: '"<<p_c[i].arg_2<<"'\n";
						break;
				}
			}	
		}

		void Condition::print() const {
			std::cout<<"Pre-Conditions ";
			switch (pre_cond_junct) {
				case CONJUNCTION:
					std::cout<<"(of type conjunction):\n";
					break;
				case DISJUNCTION:
					std::cout<<"(of type disjunction):\n";
					break;
			}
			sub_print(pr_c);
			std::cout<<"Post-Conditions ";
			switch (post_cond_junct) {
				case CONJUNCTION:
					std::cout<<"(of type conjunction):\n";
					break;
				case DISJUNCTION:
					std::cout<<"(of type disjunction):\n";
					break;
			}
			sub_print(ps_c);
		}

	/* SimpleCondition CLASS DEFINITION */ 

	void SimpleCondition::addCondition(int COND_TYPE_, int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_) {
		cond_struct.TAUT = false;
		cond_struct.LOGICAL = TRUE;
		cond_struct.ARG_1_TYPE = ARG_1_TYPE_;
		cond_struct.arg_1 = arg_1_;
		cond_struct.OPERATOR = OPERATOR_;
		cond_struct.ARG_2_TYPE = ARG_2_TYPE_;
		cond_struct.arg_2 = arg_2_;
		cond_struct.condition_label = FILLER;
		s_c.push_back(cond_struct);
		if (COND_TYPE_ != SIMPLE) {
			std::cout<<"WARNING: Invalid condition type\n";
		}
	}

	void SimpleCondition::addCondition(int COND_TYPE_, int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_, bool LOGICAL_) {
		cond_struct.TAUT = false;
		cond_struct.LOGICAL = LOGICAL_;
		cond_struct.ARG_1_TYPE = ARG_1_TYPE_;
		cond_struct.arg_1 = arg_1_;
		cond_struct.OPERATOR = OPERATOR_;
		cond_struct.ARG_2_TYPE = ARG_2_TYPE_;
		cond_struct.arg_2 = arg_2_;
		cond_struct.condition_label = FILLER;
		s_c.push_back(cond_struct);
		if (COND_TYPE_ != SIMPLE) {
			std::cout<<"WARNING: Invalid condition type\n";
		}
	}

	void SimpleCondition::addCondition(int COND_TYPE_, int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_, bool LOGICAL_, std::string condition_label_) {
		cond_struct.TAUT = false;
		cond_struct.LOGICAL = LOGICAL_;
		cond_struct.ARG_1_TYPE = ARG_1_TYPE_;
		cond_struct.arg_1 = arg_1_;
		cond_struct.OPERATOR = OPERATOR_;
		cond_struct.ARG_2_TYPE = ARG_2_TYPE_;
		cond_struct.arg_2 = arg_2_;
		cond_struct.condition_label = condition_label_;
		s_c.push_back(cond_struct);
		if (COND_TYPE_ != SIMPLE) {
			std::cout<<"WARNING: Invalid condition type\n";
		}
	}

	void SimpleCondition::setCondJunctType(int COND_TYPE_, int LOGICAL_OPERATOR) {
		if (COND_TYPE_ != SIMPLE) {
			std::cout<<"WARNING: Invalid condition type\n";
		}
		simple_cond_junct = LOGICAL_OPERATOR;
	}

	bool SimpleCondition::evaluate(const State* state) {
		bool eval;
		switch (simple_cond_junct) {
			case CONJUNCTION:
				eval = true;
				break;
			case DISJUNCTION:
				eval = false;
				break;
		}
		for (int i=0; i<s_c.size(); i++){
			bool eval_i = subEvaluate(state, s_c[i]);
			switch (simple_cond_junct) {
				case CONJUNCTION:
					eval = eval && eval_i;
					if (eval) {
						break;
					} else {
						return eval;
					}
					break;
				case DISJUNCTION:
					eval = eval || eval_i;
					if (eval) {
						return eval;
					} else {
						break;
					}
					break;
			}	
		}
		return eval;
	} 


}