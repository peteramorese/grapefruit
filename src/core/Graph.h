#pragma once
#include <string>
#include <vector>
#include <iostream>

#include "tools/Logging.h"
#include "tools/Containers.h"

namespace TP {

namespace AugmentedNodeIndex {
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

using Node = uint32_t;

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

		const std::vector<T>& getOutgoingEdges(Node node) {
			return m_graph[node].forward.edges;
		}

		const std::vector<Node>& getChildren(Node node) {
			return m_graph[node].forward.nodes;
		}
		
		const std::vector<T>& getIncomingEdges(Node node) {
			return m_graph[node].backward.edges;
		}

		const std::vector<Node>& getParents(Node node) {
			return m_graph[node].backward.nodes;
		}
		
		virtual bool connect(Node src, Node dst, const T& edge) {
			if (src >= size() || dst >= size()) {
				Node max_ind = (src > dst) ? src : dst;
				m_graph.resize(max_ind + 1);
			}
			m_graph[src];
			m_graph[src].forward.pushConnect(dst, edge);

			if (m_reversible) m_graph[dst].backward.pushConnect(src, edge);
			return true;
		}

		virtual void print() const {
			if (!m_edgeToStr) {
				ERROR("No 'edgeToStr' function provided, cannot print");
				return;
			}
			LOG("Printing graph (size: " << size() << ")");
			Node node = 0;
			for (const auto& list : m_graph) {
				for (uint32_t i=0; i < list.forward.size(); ++i) {
					if (i == 0) PRINT_NAMED("Node " << node, "is connected to:");
					PRINT_NAMED("    - child node " << list.forward.nodes[i], "with edge: " << m_edgeToStr(list.forward.edges[i]));
				}
				++node;
			}
		}

		virtual void printReverse() const {
			if (!m_edgeToStr) {
				ERROR("No 'edgeToStr' function provided, cannot print");
				return;
			}
			LOG("Printing reversed graph (size: " << size() << ")");
			Node node = 0;
			for (const auto& list : m_graph) {
				for (uint32_t i=0; i < list.backward.size(); ++i) {
					if (i == 0) PRINT_NAMED("Node " << node, "is connected to:");
					PRINT_NAMED("    - parent node " << list.backward.nodes[i], "with edge: " << m_edgeToStr(list.backward.edges[i]));
				}
				++node;
			}
		}

		virtual void clear() {m_graph.clear();}
	
	protected:
		struct AdjacencyList {
			std::vector<T> edges;
			std::vector<Node> nodes;
			void pushConnect(Node dst_node, const T& edge) {
				edges.push_back(edge);
				nodes.push_back(dst_node);
			}
			template<typename ... Args>
			constexpr void emplaceConnect(Node dst_node, Args&& ... args) {
				edges.emplace_back(std::forward<Args>(args)...);
				nodes.push_back(dst_node);
			}
			std::size_t size() const {return edges.size();}
		};
		struct BidirectionalConnectionList {
			AdjacencyList forward;
			AdjacencyList backward;
		};

	protected:
		Containers::RandomAccessList<BidirectionalConnectionList> m_graph;
		EdgeToStrFunction m_edgeToStr;

	private:
		bool m_directed, m_reversible;
};

}