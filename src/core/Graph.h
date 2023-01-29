#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <list>

#include "tools/Logging.h"

template <class T>
class RandomAccessList {
	public:
		void push_back(const T& value) {
			m_list.push_back(value);
			m_access_ptrs.push_back(&m_list.back());
		}

		T& operator[](uint32_t i) {return *m_access_ptrs[i];}
		const T& operator[](uint32_t i) const {return *m_access_ptrs[i];}

		void clear() {m_list.clear(); m_access_ptrs.clear();}
		void resize(std::size_t count) {m_list.resize(count); m_access_ptrs.resize(count);}
		std::size_t size() {return m_list.size();}

		std::list<T>::iterator begin() {return m_list.begin();}
		std::list<T>::iterator end() {return m_list.end();}
		std::list<T>::const_iterator begin() const {return m_list.cbegin();}
		std::list<T>::const_iterator end() const {return m_list.cend();}
	private:
		std::list<T> m_list;
		std::vector<T*> m_access_ptrs;
};

namespace AugmentedStateIndex {
	static uint64_t wrap(const std::vector<uint32_t>& inds, const std::vector<uint32_t>& graph_sizes) {
		uint64_t ret_ind = 0;
		uint64_t prod_size = 1;
		ASSERT(inds.size() == graph_sizes.size(), "Parameters need to have same number of elements");
		for (uint8_t i=0; i<inds.size(); ++i) {
			uint64_t temp = 1;
			for (uint8_t j=0; j<i; ++j) {
				temp *= graph_sizes[j];	
			}
			ret_ind += temp*inds[i];
			prod_size *= graph_sizes[i];
		}
		ASSERT(ret_ind < prod_size, "Indices are out of bounds (ret_ind: "<<ret_ind<<" prod_size: "<<prod_size);
		return ret_ind;
	}

	static std::vector<uint32_t> unwrap(const std::vector<int>& graph_sizes, uint64_t ind_prod) {
		std::vector<uint32_t> ret_inds(graph_sizes.size());
		uint64_t prod_size = 1;
		uint64_t temp_mod = ind_prod;
		for (int i=0; i<graph_sizes.size(); ++i) {
			prod_size *= graph_sizes[i];
			uint64_t temp = 1;
			for (uint8_t j=0; j<(graph_sizes.size() - i - 1); ++j) {
				temp *= graph_sizes[j];
			}
			ret_inds[graph_sizes.size() - i - 1] = (temp_mod - ind_prod % temp)/temp;
			temp_mod = ind_prod % temp;
		}
		ASSERT(ind_prod < prod_size, "Prod index out of bounds");
		return ret_inds;
	}
}


template<class T>
class Graph {
	public:
		typedef std::string(*EdgeToStrFunction)(const T&);
	public:
		Graph(bool directed = true, bool reversible = false, EdgeToStrFunction edgeToStr = nullptr)
			: m_directed(directed)
			, m_reversible(reversible)
			, m_edgeToStr(edgeToStr)
		{}
		
		virtual ~Graph() {}

		bool isDirected() const {return m_directed;}

		bool isReversible() const {return m_reversible;}

		std::size_t size() const {return m_graph.size();}

		const std::vector<T>& getOutgoingEdges(uint32_t ind) {
			return m_graph[ind].forward.edges;
		}

		const std::vector<uint32_t>& getChildren(uint32_t ind) {
			return m_graph[ind].forward.nodes;
		}
		
		const std::vector<T>& getIncomingEdges(uint32_t ind) {
			return m_graph[ind].backward.edges;
		}

		const std::vector<uint32_t>& getParents(uint32_t ind) {
			return m_graph[ind].backward.nodes;
		}
		
		virtual bool connect(uint32_t src, uint32_t dst, const T& edge) {
			if (src >= size() || dst >= size()) {
				uint32_t max_ind = (src > dst) ? src : dst;
				m_graph.resize(max_ind);
			}
			m_graph[src].forward.pushConnect(dst, edge);
			if (m_reversible) m_graph[dst].backward.pushConnect(src, edge);
			return true;
		}

		virtual void print() const {
			LOG("Printing graph");
			uint32_t node_ind = 0;
			for (const auto& list : m_graph) {
				PRINT_NAMED("Node " << node_ind++, "is connected to:");
				for (uint32_t i=0; i < list.forward.size(); ++i) {
					std::string edge_str = (m_edgeToStr) ? m_edgeToStr(list.forward.edges[i]) : list.forward.edges[i];
					PRINT_NAMED("    - child node " << list.forward.nodes[i], "with edge: " << list.forward.edges[i]);
				}
			}
		}

		virtual void printReverse() const {
			LOG("Printing reversed graph");
			uint32_t node_ind = 0;
			for (const auto& list : m_graph) {
				PRINT_NAMED("Node " << node_ind++, "is connected to:");
				for (uint32_t i=0; i < list.backward.size(); ++i) {
					std::string edge_str = (m_edgeToStr) ? m_edgeToStr(list.backward.edges[i]) : list.backward.edges[i];
					PRINT_NAMED("    - parent node " << list.backward.nodes[i], "with edge: " << edge_str);
				}
			}
		}

		virtual void clear() {m_graph.clear();}
	
	private:
		struct AdjacencyList {
			std::vector<T> edges;
			std::vector<uint32_t> nodes;
			void pushConnect(uint32_t dst_node, const T& edge) {
				edges.push_back(edge);
				nodes.push_back(dst_node);
			}
			template<typename ... Args>
			constexpr void emplaceConnect(uint32_t dst_node, Args&& ... args) {
				edges.emplace_back(std::forward<Args>(args)...);
				nodes.push_back(dst_node);
			}
			std::size_t size() const {return edges.size();}
		};
		struct BidirectionalConnectionList {
			AdjacencyList forward;
			AdjacencyList backward;
		};
	private:
		RandomAccessList<BidirectionalConnectionList> m_graph;
		bool m_directed, m_reversible;
		EdgeToStrFunction m_edgeToStr;
};

