#pragma once
#include<string>
#include<vector>
#include<iostream>
#include<functional>

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

template<class T>
class Graph {
	private:
		//std::vector<T> node_list;
		bool ordered;
		bool isEmpty(); 
	protected:
		struct node {
			int nodeind; // Node index or name
			node* adjptr; // Pointer to connected element
			T* dataptr;
		};
		std::vector<node*> tails;
		std::vector<node*> heads;
		template<typename pLAM> void print(pLAM printLambda);
	public:
		Graph(bool ordered = true); 
		bool isOrdered() const;
		bool isEmpty(node* head) const; 
		int size() const; 
		void getConnectedNodes(unsigned int ind_, std::vector<int>& node_list);
		void getConnectedData(unsigned int ind_, std::vector<T*>& data_list);
		const std::vector<node*>* getHeads() const; // DO NOT USE UNLESS YOU NEED RAW ACCESS
		virtual bool connect(const std::pair<unsigned int, T*>& src, const std::pair<unsigned int, T*>& dst);
		virtual bool connect(unsigned int src_ind, const std::pair<unsigned int, T*>& dst);
		
		template<typename LAM> bool hop(unsigned int ind, LAM lambda);
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
		const std::vector<unsigned int>* getAcceptingStates() const;
		void setInitStates(const std::vector<unsigned int>& init_states_);
		const std::vector<unsigned int>* getInitStates() const;
		void setAlphabet(const std::vector<std::string>& alphabet_);
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
		int getInitState();
		bool connectDFA(unsigned int ind_from, unsigned int ind_to, const std::string& label_);
		bool readFileSingle(const std::string& filename);
		void print();
		//void operator*(const DFA& arg_dfa);
		//static bool syncProduct( const DFA* arg_dfa, const DFA* arg_dfa2, DFA* product);
		//static bool readFileMultiple(const std::string& filename, std::array<DFA, int>& dfa_arr);
		~DFA();
};
