#pragma once
#include<string>
#include<vector>
#include<iostream>
#include<functional>
#include "lexSet.h"

// Declare some struct list types for different graphs:

struct WL {
	float weight;
	std::string label;
};

struct WLI {
	float weight;
	std::string label;
	int i;
};

struct WIV {
	float weight;
	int i;
	std::vector<int> v;
};

struct IVFlexLexS {
	IVFlexLexS(float mu, unsigned int S) : v(S), lex_set(mu, S) {}
	IVFlexLexS(float mu, float fill_val, unsigned int S) : v(S), lex_set(mu, fill_val, S) {}
	int i;
	std::vector<int> v;
	FlexLexSetS lex_set;
};

template<class T>
class Graph {
	private:
		//std::vector<T> node_list;
		bool ordered;
		bool isEmpty(); 
	public:
		struct node {
			int nodeind; // Node index or name
			node* adjptr; // Pointer to connected element
			T* dataptr;
		};
	protected:
		std::vector<node*> tails;
		std::vector<node*> heads;
		template<typename pLAM> void print(pLAM printLambda);
	public:
		Graph(bool ordered = true); 
		bool isOrdered() const;
		bool isEmpty(node* head) const; 
		int size() const; 
		T* getNodeDataptr(unsigned int ind_) const;
		void getConnectedNodes(unsigned int ind_, std::vector<int>& node_list);
		void getConnectedData(unsigned int ind_, std::vector<T*>& data_list);
		const std::vector<node*>* getHeads() const; // DO NOT USE UNLESS YOU NEED RAW ACCESS
		virtual bool connect(const std::pair<unsigned int, T*>& src, const std::pair<unsigned int, T*>& dst);
		virtual bool connect(unsigned int src_ind, const std::pair<unsigned int, T*>& dst);
		
		template<typename LAM> bool hop(unsigned int ind, LAM lambda);
		//template<typename LAM> bool hopS(unsigned int ind, LAM lambda);
		bool hopS(unsigned int ind, std::function<bool(node*, node*)> lambda) const;
		bool hopF(unsigned int ind, std::function<bool(node*, node*)> lambda);
		virtual void remove(unsigned int ind_);
		virtual void print();
		void updateData(unsigned int ind_from, unsigned int ind_to, T* dataptr_);
		static int augmentedStateFunc(int i, int j, int n, int m);
		static void augmentedStateMap(unsigned int ind_product, int n, int m, std::pair<unsigned int, unsigned int>& ret_indices);
		void clear();
		virtual ~Graph(); 
		//virtual void compose(const Edge &mult_graph, Edge& product_graph);

};

template<class T>
class Automaton : public Graph<T> {
	private:
		unsigned int max_accepting_state_index;
		unsigned int max_init_state_index;
	protected:
		//std::vector<T> node_data_list;
		std::vector<bool> is_accepting;
		std::vector<unsigned int> accepting_states;
		std::vector<unsigned int> init_states;
		std::vector<std::string> alphabet;
		void addWord(const std::string&);
		void addAcceptingState(unsigned int accepting_state);
		bool isAutomatonValid();
		bool inAlphabet(std::string);
		bool checkAlphabet(const Automaton* arg_dfa);
		//template<typename pLAM> void print(pLAM printLambda);
		std::vector<T*> node_data_list;
	public:
		Automaton();
		void setAcceptingStates(const std::vector<unsigned int>& accepting_states_);
		bool isAccepting(unsigned int ind) const;
		const std::vector<unsigned int>* getAcceptingStates() const;
		void setInitStates(const std::vector<unsigned int>& init_states_);
		const std::vector<unsigned int>* getInitStates() const;
		void setAlphabet(const std::vector<std::string>& alphabet_);
		const std::vector<std::string>* getAlphabet() const;
};

class NFA : public Automaton<std::string> {
	private:
	public:
};

class DFA : public Automaton<std::string> {
	private:
		bool check_det;
	public:
		DFA();
		void toggleCheckDeterminism(bool check_det_);
		int getInitState() const;
		bool connectDFA(unsigned int ind_from, unsigned int ind_to, const std::string& label_);
		bool readFileSingle(const std::string& filename);
		void print();
		//void operator*(const DFA& arg_dfa);
		//static bool syncProduct( const DFA* arg_dfa, const DFA* arg_dfa2, DFA* product);
		//static bool readFileMultiple(const std::string& filename, std::array<DFA, int>& dfa_arr);
		~DFA();
};

class DFA_EVAL {
	private:
		const DFA* dfaptr;
		//Graph<std::string>::node* curr_node;
		int curr_node;
		bool init, accepting;
		friend class DFA;
		//friend class Graph<std::string>;
	public:
		DFA_EVAL(const DFA* dfaptr_);
		const std::vector<std::string>* getAlphabetEVAL() const;
		bool eval(const std::string& letter);
		int getCurrNode() const;
		void set(int set_node);
		void reset();
		bool isCurrAccepting() const;
};
