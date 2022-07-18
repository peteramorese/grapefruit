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
		LexSet(unsigned int S_, float fill_val);
		LexSet(unsigned int S_, const std::vector<float>& fill_set);
		unsigned int size() const;
		void fill(float value);
		void setInf();
		bool isInf() const;
		float getMaxVal() const;
		void addToMax(float v);
		virtual void operator+=(const LexSet& arg_set);
		virtual void operator+=(const std::vector<float>& arg_vec);
		virtual void operator=(const LexSet& arg_set);
		virtual void operator=(const std::vector<float>& arg_vec);
		virtual bool operator==(const LexSet& arg_set) const;
		virtual bool operator<(const LexSet& arg_set) const;
		virtual bool operator<=(const LexSet& arg_set) const;
		virtual bool operator>(const LexSet& arg_set) const;
		virtual bool operator>=(const LexSet& arg_set) const;
		virtual float operator[](unsigned ind) const;
		void print() const;
		virtual ~LexSet();
};

class FlexLexSetS : public LexSet {
	private:
		float mu;
		void overflow();
	public:
		FlexLexSetS(unsigned int S_, float mu_);
		FlexLexSetS(unsigned int S_, float mu_, float fill_val);
		FlexLexSetS(unsigned int S_, float mu_, const std::vector<float>& fill_set);
		void operator+=(const FlexLexSetS& arg_set);
		void operator+=(const std::vector<float>& arg_vec);
		void operator=(const FlexLexSetS& arg_set);
		void operator=(const std::vector<float>& arg_vec);

};

class DetourLex : public LexSet {
	private: 
		const float mu;
	public:
		DetourLex(unsigned int S_, float mu_);
		DetourLex(unsigned int S_, float mu_, float fill_val);
		DetourLex(unsigned int S_, float mu_, const std::vector<float>& fill_set);
		void operator+=(const DetourLex& arg_set);
		void operator+=(const std::vector<float>& arg_vec);
		void operator=(const DetourLex& arg_set);
		void operator=(const std::vector<float>& arg_vec);
		bool withinBounds(const DetourLex& arg_set) const;
		void addHeuristic(const std::vector<float>& h_vals);
};

