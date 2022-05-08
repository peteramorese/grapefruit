#pragma once
#include<string>
#include<vector>
#include<iostream>
class Edge {
	private:
		
		bool checking, ordered;
		unsigned int ind, ind_checkout;
		bool isEmpty(); 
	protected:
		struct node {
			//node(int Nobj_) : Nobjs(Nobj_) {};

			int nodeind; // Node index or name
			float weight; // Weight of connecting edge, representing resource function
			std::string label;
			node* adjptr;
		};
		node* head;
		node* prev;
		std::vector<node*> prevs;
		std::vector<node*> heads;

		
		void deleteList(node* head);
	public:
		Edge(bool ordered); 
		bool isOrdered() const;
		bool isListEmpty(node* head) const; 
		void append(unsigned int nodeind_, float weight_, std::string label); 
		void checkout(int ind_checkout); 
		void newlist();
		int size() const; 
		const std::vector<node*> getHeads() const;
		void returnListNodes(unsigned int ind_, std::vector<int>& node_list) const;
		void returnListLabels(unsigned int ind_, std::vector<std::string>& label_list) const;
		void returnListWeights(unsigned int ind_, std::vector<float>& weights_list) const;
		virtual bool connect(unsigned int ind_from, unsigned int ind_to, float weight_, std::string label_);
		virtual bool remove(unsigned int ind_);
		virtual void print() const;
		float getWeight(unsigned int ind_from, unsigned int ind_to) const;
		void updateWeight(unsigned int ind_from, unsigned int ind_to, float weight_);
		static int augmentedStateFunc(int i, int j, int n, int m);
		virtual void compose(const Edge &mult_graph, Edge& product_graph);
		static void augmentedStateMap(unsigned int ind_product, int n, int m, std::pair<unsigned int, unsigned int>& ret_indices);
		void clear();
		virtual ~Edge(); 

};

//class PrioQueue : public Edge {
//	private:
//		node* prio_node;
//
//	public:
//		bool connect(unsigned int ind_from, unsigned int ind_to, float weight_, std::string label_);
//}



