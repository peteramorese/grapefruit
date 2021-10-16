#pragma once
#include<string>
#include<vector>
#include<iostream>
#include<functional>
//const static auto printLAMB = [](node* dst){std::cout<<dst->nodeind;};
template<class T>
class Graph {
	private:
		std::vector<T> node_list;
		bool checking, ordered;
		unsigned int ind, ind_checkout;
		bool isEmpty(); 
	protected:
		struct node {
			int nodeind; // Node index or name
			node* adjptr; // Pointer to connected element
			T* dataptr;
		};
		node* head;
		node* prev;
		std::vector<node*> tails;
		std::vector<node*> heads;


		
		void deleteList(node* head);
	public:
		Graph(bool ordered = true); 
		bool isOrdered() const;
		bool isEmpty(node* head) const; 
		//void append(unsigned int nodeind_, float weight_, std::string label); 
		//void checkout(int ind_checkout); 
		//void newlist();
		int size() const; 
		//const std::vector<node*>* getHeads() const;
		//void returnListNodes(unsigned int ind_, std::vector<int>& node_list) const;
		//void returnListLabels(unsigned int ind_, std::vector<std::string>& label_list) const;
		//void returnListWeights(unsigned int ind_, std::vector<float>& weights_list) const;
		virtual bool connect(std::pair<unsigned int, T*> src, std::pair<unsigned int, T*> dst);
		
		template<typename LAM> bool hop(unsigned int ind, LAM lambda);
		virtual void remove(unsigned int ind_);
		virtual void print();
		//float getWeight(unsigned int ind_from, unsigned int ind_to) const;
		//void updateWeight(unsigned int ind_from, unsigned int ind_to, float weight_);
		static int augmentedStateFunc(int i, int j, int n, int m);
		//virtual void compose(const Edge &mult_graph, Edge& product_graph);
		static void augmentedStateMap(unsigned int ind_product, int n, int m, std::pair<unsigned int, unsigned int>& ret_indices);
		void clear();
		virtual ~Graph(); 

};

//class PrioQueue : public Edge {
//	private:
//		node* prio_node;
//
//	public:
//		bool connect(unsigned int ind_from, unsigned int ind_to, float weight_, std::string label_);
//}



