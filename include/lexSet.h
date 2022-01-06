#pragma once
#include<vector>
#include<iostream>

class LexSet {
	protected:
		std::vector<float> lex_set;
		bool inf_set;
		const unsigned int S;
	public:
		LexSet(unsigned int S_);
		LexSet(float fill_val, unsigned int S_);
		LexSet(const std::vector<float>* fill_set, unsigned int S_);
		unsigned int size() const;
		void setInf();
		float retMaxVal() const;
		virtual void operator+=(const LexSet& arg_set);
		virtual void operator+=(const std::vector<float>& arg_vec);
		virtual void operator=(const LexSet& arg_set);
		virtual void operator=(const std::vector<float>& arg_vec);
		virtual bool operator==(const LexSet& arg_set);
		virtual bool operator<(const LexSet& arg_set);
		virtual bool operator<=(const LexSet& arg_set);
		virtual bool operator>(const LexSet& arg_set);
		virtual bool operator>=(const LexSet& arg_set);
		void print() const;
};

class FlexLexSetS : public LexSet {
	private:
		float mu;
		void overflow();
	public:
		FlexLexSetS(float mu_, unsigned int S_);
		FlexLexSetS(float mu_, float fill_val, unsigned int S_);
		FlexLexSetS(float mu_, const std::vector<float>* fill_set, unsigned int S_);
		void operator+=(const FlexLexSetS& arg_set);
		void operator+=(const std::vector<float>& arg_vec);
		void operator=(const FlexLexSetS& arg_set);
		void operator=(const std::vector<float>& arg_vec);

};

class REQLex : public LexSet {
	private:
		float mu;
	public:
		REQLex(float mu_, unsigned int S_);
		REQLex(float mu_, float fill_val, unsigned int S_);
		REQLex(float mu_, const std::vector<float>* fill_set, unsigned int S_);
		void operator=(const REQLex& arg_set);
		void operator=(const std::vector<float>& arg_vec);
		bool operator==(const REQLex& arg_set);
		bool operator<(const REQLex& arg_set);
		bool operator<=(const REQLex& arg_set);
		bool operator>(const REQLex& arg_set);
		bool operator>=(const REQLex& arg_set);
};

