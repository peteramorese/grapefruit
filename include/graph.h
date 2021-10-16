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
	public:
		Graph(bool ordered = true); 
		bool isOrdered() const;
		bool isEmpty(node* head) const; 
		int size() const; 
		void getConnectedNodes(unsigned int ind_, std::vector<int>& node_list);
		void getConnectedData(unsigned int ind_, std::vector<T*>& data_list);
		virtual bool connect(const std::pair<unsigned int, T*>& src, const std::pair<unsigned int, T*>& dst);
		virtual bool connect(unsigned int src_ind, const std::pair<unsigned int, T*>& dst);
		
		template<typename LAM> bool hop(unsigned int ind, LAM lambda);
		virtual void remove(unsigned int ind_);
		virtual void print();
		void updateData(unsigned int ind_from, unsigned int ind_to, T* dataptr_);
		static int augmentedStateFunc(int i, int j, int n, int m);
		static void augmentedStateMap(unsigned int ind_product, int n, int m, std::pair<unsigned int, unsigned int>& ret_indices);
		void clear();
		virtual ~Graph(); 
		//virtual void compose(const Edge &mult_graph, Edge& product_graph);

};


