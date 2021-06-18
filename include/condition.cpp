#include<iostream>
#include<vector>
#include<unordered_map>
#include "state.h"
#include "condition.h"

const bool Condition::TRUE = true;
const bool Condition::NEGATE = false;
const std::string Condition::FILLER = "FILLER";
const int Condition::NONE = -1;
const int Condition::LABEL = 0;
const int Condition::VAR = 1;
const int Condition::DOMAIN = 2;
const int Condition::GROUP = 3;
const int Condition::ARG_L = 4;
const int Condition::ARG_V = 5;
const int Condition::EQUALS = 6;
const int Condition::IN_DOMAIN = 7;
const int Condition::ARG_FIND = 8;
const int Condition::ARG_EQUALS = 9;
const int Condition::CONJUNCTION = 10;
const int Condition::DISJUNCTION = 11;
const int Condition::PRE = 12;
const int Condition::POST = 13;
const int Condition::SIMPLE = 14;

Condition::Condition() {
	pr_c.clear();
	ps_c.clear();
	action_label = FILLER;
	label = FILLER;
}

void Condition::addCondition(int COND_TYPE_, int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_) {
	cond_struct.LOGICAL = TRUE;
	cond_struct.ARG_1_TYPE = ARG_1_TYPE_;
	cond_struct.arg_1 = arg_1_;
	cond_struct.OPERATOR = OPERATOR_;
	cond_struct.ARG_2_TYPE = ARG_2_TYPE_;
	cond_struct.arg_2 = arg_2_;
	cond_struct.condition_label = FILLER;
	if (COND_TYPE_ == PRE) {
		pr_c.push_back(cond_struct);
	} else if (COND_TYPE_ == POST) {
		ps_c.push_back(cond_struct);
	} else {
		std::cout<<"Error: Invalid condition type\n";
	}
}

void Condition::addCondition(int COND_TYPE_, int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_, bool LOGICAL_) {
	cond_struct.LOGICAL = LOGICAL_;
	cond_struct.ARG_1_TYPE = ARG_1_TYPE_;
	cond_struct.arg_1 = arg_1_;
	cond_struct.OPERATOR = OPERATOR_;
	cond_struct.ARG_2_TYPE = ARG_2_TYPE_;
	cond_struct.arg_2 = arg_2_;
	cond_struct.condition_label = FILLER;
	if (COND_TYPE_ == PRE) {
		pr_c.push_back(cond_struct);
	} else if (COND_TYPE_ == POST) {
		ps_c.push_back(cond_struct);
	} else {
		std::cout<<"Error: Invalid condition type\n";
	}
}

void Condition::addCondition(int COND_TYPE_, int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_, bool LOGICAL_, std::string condition_label_) {
	cond_struct.LOGICAL = LOGICAL_;
	cond_struct.ARG_1_TYPE = ARG_1_TYPE_;
	cond_struct.arg_1 = arg_1_;
	cond_struct.OPERATOR = OPERATOR_;
	cond_struct.ARG_2_TYPE = ARG_2_TYPE_;
	cond_struct.arg_2 = arg_2_;
	cond_struct.condition_label = condition_label_;
	if (COND_TYPE_ == PRE) {
		pr_c.push_back(cond_struct);
	} else if (COND_TYPE_ == POST) {
		ps_c.push_back(cond_struct);
	} else {
		std::cout<<"Error: Invalid condition type\n";
	}
}

void Condition::setCondJunctType(int COND_TYPE_, int LOGICAL_OPERATOR) {
	if (COND_TYPE_ == PRE) {
		pre_cond_junct = LOGICAL_OPERATOR;
	} else if (COND_TYPE_ == POST) {
		post_cond_junct = LOGICAL_OPERATOR;
	} else {
		std::cout<<"Error: Invalid condition type\n";
	}
}

/*
void Condition::addPostCondition(int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_) {
	cond_struct.LOGICAL = TRUE;
	cond_struct.ARG_1_TYPE = ARG_1_TYPE_;
	cond_struct.arg_1 = arg_1_;
	cond_struct.OPERATOR = OPERATOR_;
	cond_struct.ARG_2_TYPE = ARG_2_TYPE_;
	cond_struct.arg_2 = arg_2_;
	cond_struct.condition_label = FILLER;
	ps_c.push_back(cond_struct);
}

void Condition::addPostCondition(int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_, bool LOGICAL_) {
	cond_struct.LOGICAL = LOGICAL_;
	cond_struct.ARG_1_TYPE = ARG_1_TYPE_;
	cond_struct.arg_1 = arg_1_;
	cond_struct.OPERATOR = OPERATOR_;
	cond_struct.ARG_2_TYPE = ARG_2_TYPE_;
	cond_struct.arg_2 = arg_2_;
	cond_struct.condition_label = FILLER;
	ps_c.push_back(cond_struct);
}

void Condition::addPostCondition(int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_, bool LOGICAL_, std::string condition_label_) {
	cond_struct.LOGICAL = LOGICAL_;
	cond_struct.ARG_1_TYPE = ARG_1_TYPE_;
	cond_struct.arg_1 = arg_1_;
	cond_struct.OPERATOR = OPERATOR_;
	cond_struct.ARG_2_TYPE = ARG_2_TYPE_;
	cond_struct.arg_2 = arg_2_;
	cond_struct.condition_label = condition_label_;
	ps_c.push_back(cond_struct);
}

void Condition::setPostCondJunctType(int LOGICAL_OPERATOR) {
	post_cond_junct = LOGICAL_OPERATOR;
}
*/

void Condition::setActionLabel(const std::string& action_label_) {
	action_label = action_label_;
}

std::string Condition::getActionLabel() {
	return action_label;
}

void Condition::setLabel(const std::string& label_) {
	label = label_;
}

std::string Condition::getLabel() {
	return label;
}

bool Condition::subEvaluate(const State* state, const sub_condition& cond) {
	bool sub_eval = false;

	std::string temp_var;
	std::string arg_dimension_label;
	switch (cond.OPERATOR) {
		case EQUALS: 
			if (cond.ARG_2_TYPE == VAR){
				sub_eval = state->getVar(cond.arg_1) == cond.arg_2;
			} else {
				std::cout<<"Error: Condition Syntax error for operator EQUALS\n";
			}
			break;
		case IN_DOMAIN:
			if (cond.ARG_2_TYPE == DOMAIN) {
				sub_eval = false;
				bool found;
				std::vector<std::string> in_domains;
				std::string dom_var;
				switch (cond.ARG_1_TYPE) {
					case LABEL:
						dom_var = state->getVar(cond.arg_1);
						found = state->getDomains(dom_var, in_domains);
						if (found) {
							for (int ii=0; ii<in_domains.size(); ii++) {
								if (in_domains[ii] == cond.arg_2) {
									sub_eval = true;	
									break;
								}
							}
						}
						break;
					case GROUP: 
						{
							std::vector<std::string> group_dim_labels;
							state->getGroupDimLabels(cond.arg_1, group_dim_labels);
							sub_eval = true;
							for (int ii=0; ii<group_dim_labels.size(); ++ii) {
								bool sub_sub_eval = false;
								dom_var = state->getVar(group_dim_labels[ii]);
								found = state->getDomains(dom_var, in_domains);
								if (found) {
									for (int ii=0; ii<in_domains.size(); ii++) {
										if (in_domains[ii] == cond.arg_2) {
											sub_sub_eval = true;	
											break;
										}
									}
								}		
								if (!sub_sub_eval) {
									sub_eval = false;
									break;
								}
							}
						}
						break;
					default:
						std::cout<<"Error: Condition Syntax error for operator IN_DOMAIN\n";
				}
			} else {
				std::cout<<"Error: Condition Syntax error for operator IN_DOMAIN\n";
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
						arg_V_i.first = true;
						arg_V_i.second = state->getVar(cond.arg_1);
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
					if (arg_V_i.first) {
						//if (cond.ARG_2_TYPE == VAR){
						switch (cond.ARG_2_TYPE){
							case VAR:
								sub_eval = arg_V_i.second == cond.arg_2;
								break;
							case LABEL:
								sub_eval = arg_V_i.second == state->getVar(cond.arg_2);
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
		bool eval = false;
		bool pre_eval;
		arg_L.clear();
		arg_L.resize(0);
		arg_V.clear();
		arg_V.resize(0);
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
			arg_V_i.second = "no_arg";
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
						goto postcondition;
					}
					break;
				case DISJUNCTION:
					pre_eval = pre_eval || pre_eval_i;
					if (pre_eval) {
						goto postcondition;
					} else {
						break;
					}
					break;
			}	
		}
postcondition:
		if (pre_eval) {
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
						if (arg_V_i.first) {
							excl_dim_labels.push_back(arg_V_i.second);
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
							goto returncondition;
						}
						break;
					case DISJUNCTION:
						post_eval = post_eval || post_eval_i;
						if (post_eval) {
							goto returncondition;
						} else {
							break;
						}
						break;
				}	
			}
returncondition:
			eq_eval = pre_state->exclEquals(post_state, excl_dim_labels);
			// pre_eval: Are the preconditions satisfied?
			// post_eval: Are the post conditions satisfied?
			// eq_eval: Are the other unmentioned dimensions still equal between states?
			// All of these must be true for the condition to be satisfied
			eval = pre_eval && post_eval && eq_eval;
			return eval;
		} else {
			return false;
		}
	}

	void Condition::sub_print(const std::vector<sub_condition>& p_c) const {
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
				case IN_DOMAIN:
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
				case DOMAIN:
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
					goto returncondition;
				}
				break;
			case DISJUNCTION:
				eval = eval || eval_i;
				if (eval) {
					goto returncondition;
				} else {
					break;
				}
				break;
		}	
	}
returncondition:
	return eval;
} 


