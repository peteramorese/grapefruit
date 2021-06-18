#pragma once
#include<string>
#include<vector>
#include<iostream>
class Edge {
	private:
		struct edgelist {
			//edgelist(int Nobj_) : Nobjs(Nobj_) {};

			int nodeind; // Node index or name
			float weight; // Weight of connecting edge, representing resource function
			std::string label;
			edgelist* adjptr;
		};
		bool checking, ordered;
		unsigned int ind, ind_checkout;
		edgelist* head;
		edgelist* prev;
		std::vector<edgelist*> prevs;
		std::vector<edgelist*> heads;
		bool isEmpty(); 
	public:
		Edge(bool ordered); 
		bool isOrdered() const;
		bool isListEmpty(edgelist* head) const; 
		void append(unsigned int nodeind_, float weight_, std::string label); 
		void checkout(int ind_checkout); 
		void newlist();
		int returnListCount() const; 
		const std::vector<edgelist*> getHeads() const;
		void returnListNodes(unsigned int ind_, std::vector<int>& node_list) const;
		void returnListLabels(unsigned int ind_, std::vector<std::string>& label_list) const;
		void returnListWeights(unsigned int ind_, std::vector<float>& weights_list) const;
		void connect(unsigned int ind_from, unsigned int ind_to, float weight_, std::string label_);
		void print() const;
		static int augmentedStateFunc(int i, int j, int n, int m);
		void compose(const Edge &mult_graph, Edge& product_graph);
		static void augmentedStateMap(unsigned int ind_product, int n, int m, std::pair<unsigned int, unsigned int>& ret_indices);
		~Edge(); 

};


