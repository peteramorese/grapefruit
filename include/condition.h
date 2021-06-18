#pragma once
#include<vector>
#include "state.h"
#include<string>
#include<unordered_map>

class Condition {
	protected:
		struct sub_condition {
			bool LOGICAL;
			int ARG_1_TYPE;
			std::string arg_1;
			int OPERATOR;
			int ARG_2_TYPE;
			std::string arg_2;
			std::string condition_label;
		} cond_struct;
		std::vector<sub_condition> pr_c;
		std::vector<sub_condition> ps_c;
		int pre_cond_junct;
		int post_cond_junct;
		std::vector<std::pair<bool, std::string>> arg_L;
		std::unordered_map<std::string, int> arg_L_labels;
		std::vector<std::pair<bool, std::string>> arg_V;
		std::unordered_map<std::string, int> arg_V_labels;
		std::pair<bool, std::string> arg_L_i;
		std::pair<bool, std::string> arg_V_i;
		void sub_print(const std::vector<sub_condition>& p_c) const;
		std::string action_label;
		std::string label;
	public:	
		static const bool TRUE;
		static const bool NEGATE;
		static const std::string FILLER;
		static const int NONE;
		static const int LABEL;
		static const int VAR;	
		static const int DOMAIN;	
		static const int GROUP;
		static const int ARG_L;	
		static const int ARG_V;	
		static const int EQUALS;	
		static const int IN_DOMAIN;	
		static const int ARG_FIND;	
		static const int ARG_EQUALS;	
		static const int CONJUNCTION;	
		static const int DISJUNCTION;
		static const int PRE;
		static const int POST;
		static const int SIMPLE;

		Condition();
		virtual void addCondition(int COND_TYPE_, int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_);
		virtual void addCondition(int COND_TYPE_, int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_, bool LOGICAL_);
		virtual void addCondition(int COND_TYPE_, int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_, bool LOGICAL_, std::string condition_label_);
		virtual void setCondJunctType(int COND_TYPE_, int LOGICAL_OPERATOR);
		//void addPostCondition(int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_);
		//void addPostCondition(int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_, bool LOGICAL_);
		//void addPostCondition(int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_, bool LOGICAL_, std::string condition_label_);
		//void setPostCondJunctType(int LOGICAL_OPERATOR);
		void setActionLabel(const std::string& action_label_);
		std::string getActionLabel();
		void setLabel(const std::string& label_);
		std::string getLabel();
		bool subEvaluate(const State* state, const sub_condition& cond);
		bool evaluate(const State* pre_state, const State* post_state);
		void print() const;
};


class SimpleCondition : public Condition {
	private:
		std::vector<sub_condition> s_c;
		int simple_cond_junct;
	public:
		void addCondition(int COND_TYPE_, int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_);
		void addCondition(int COND_TYPE_, int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_, bool LOGICAL_);
		void addCondition(int COND_TYPE_, int ARG_1_TYPE_, std::string arg_1_, int OPERATOR_, int ARG_2_TYPE_, std::string arg_2_, bool LOGICAL_, std::string condition_label_);
		void setCondJunctType(int COND_TYPE_, int LOGICAL_OPERATOR);
		bool evaluate(const State* state);
};

