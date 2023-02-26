#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <iostream>

#include "tools/Logging.h"
#include "tools/Containers.h"

namespace TP {

using Node = uint32_t;
using WideNode = uint64_t;

namespace AugmentedNodeIndex {
	static WideNode wrap(const Containers::SizedArray<Node>& inds, const Containers::SizedArray<std::size_t>& graph_sizes) {
		WideNode ret_ind = 0;
		WideNode prod_size = 1;
		ASSERT(inds.size() == graph_sizes.size(), "Parameters need to have same number of elements");
		for (uint8_t i=0; i<inds.size(); ++i) {
			WideNode temp = 1;
			for (uint8_t j=0; j<i; ++j) {
				temp *= graph_sizes[j];	
			}
			ret_ind += temp*inds[i];
			prod_size *= graph_sizes[i];
		}
		ASSERT(ret_ind < prod_size, "Indices are out of bounds (ret_ind: "<<ret_ind<<" prod_size: "<<prod_size);
		return ret_ind;
	}

	static const Containers::SizedArray<Node> unwrap(WideNode ind_prod, const Containers::SizedArray<std::size_t>& graph_sizes) {
		Containers::SizedArray<Node> ret_inds(graph_sizes.size());
		WideNode prod_size = 1;
		WideNode temp_mod = ind_prod;
		for (int i=0; i<graph_sizes.size(); ++i) {
			prod_size *= graph_sizes[i];
			WideNode temp = 1;
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



template<class EDGE_T, typename NATIVE_NODE_T = Node>
class Graph {
	public:
	 	typedef EDGE_T edge_t; // used for dependent types
	 	typedef NATIVE_NODE_T node_t; // used for dependent types
		
		typedef std::string(*EdgeToStrFunction)(const EDGE_T&);
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

		inline const std::vector<EDGE_T>& getOutgoingEdges(NATIVE_NODE_T node) const {
			return m_graph[node].forward.edges;
		}

		inline const std::vector<NATIVE_NODE_T>& getChildren(NATIVE_NODE_T node) const {
			return m_graph[node].forward.nodes;
		}
		
		inline const std::vector<EDGE_T>& getIncomingEdges(NATIVE_NODE_T node) const {
			return m_graph[node].backward.edges;
		}

		inline const std::vector<NATIVE_NODE_T>& getParents(NATIVE_NODE_T node) const {
			return m_graph[node].backward.nodes;
		}
		
		virtual bool connect(NATIVE_NODE_T src, NATIVE_NODE_T dst, const EDGE_T& edge) {
			if (src >= size() || dst >= size()) {
				NATIVE_NODE_T max_ind = (src > dst) ? src : dst;
				m_graph.resize(max_ind + 1);
			}
			m_graph[src];
			m_graph[src].forward.pushConnect(dst, edge);

			if (m_reversible) m_graph[dst].backward.pushConnect(src, edge);
			return true;
		}

		virtual bool disconnect(NATIVE_NODE_T src, NATIVE_NODE_T dst) {
			ASSERT(src < size() && dst < size(), "Disconnection nodes not found");
			bool found = m_graph[src].forward.disconnect(dst);
			if (!found) return false;
			found = m_graph[dst].backward.disconnect(src);
			ASSERT(found, "Disconnection found in forward, but not backward");
			return true;
		}

		virtual bool disconnect(NATIVE_NODE_T src, NATIVE_NODE_T dst, const EDGE_T& edge) {
			ASSERT(src < size() && dst < size(), "Disconnection nodes not found");
			bool found = m_graph[src].forward.disconnect(dst, edge);
			if (!found) return false;
			found = m_graph[dst].backward.disconnect(src, edge);
			ASSERT(found, "Disconnection found in forward, but not backward");
			return true;
		}

		template <typename LAM>
		void disconnectIf(NATIVE_NODE_T src, LAM removeConnection) {
			auto removeConnectionWrapper = [&, this](NATIVE_NODE_T dst, const EDGE_T& edge) {
				bool remove = removeConnection(dst, edge);
				if (remove) LOG("FOUND ONE TO REMOVE");
				LOG("HELO");
				if (m_reversible && remove) {
					LOG("   backwards disconnecting " << src);
					m_graph[dst].backward.disconnect(src, edge);
				}
				return remove;
			};
			m_graph[src].forward.disconnectIf(removeConnectionWrapper);
		}

		virtual void print() const {
			if (!m_edgeToStr) { // Allow program to continue even if print function is not provided
				ERROR("No 'edgeToStr' function provided, cannot print");
				return;
			}
			LOG("Printing graph (size: " << size() << ")");
			NATIVE_NODE_T node = 0;
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
			NATIVE_NODE_T node = 0;
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
			std::vector<EDGE_T> edges;
			std::vector<NATIVE_NODE_T> nodes;
			void pushConnect(NATIVE_NODE_T dst_node, const EDGE_T& edge) {
				edges.push_back(edge);
				nodes.push_back(dst_node);
			}
			template <typename ... Args>
			constexpr void emplaceConnect(NATIVE_NODE_T dst_node, Args&& ... args) {
				edges.emplace_back(std::forward<Args>(args)...);
				nodes.push_back(dst_node);
			}
			bool disconnect(NATIVE_NODE_T dst_node) {
				bool found = false;
				auto e_it = edges.begin();
				for (auto n_it = nodes.begin(); n_it != nodes.end();) {
					if (*n_it == dst_node) {
						nodes.erase(n_it);
						edges.erase(e_it);
						found = true;
					} else {
						n_it++;
						e_it++;
					}
				}
				return found;
			}
			bool disconnect(NATIVE_NODE_T dst_node, const EDGE_T& edge) {
				bool found = false;
				auto e_it = edges.begin();
				for (auto n_it = nodes.begin(); n_it != nodes.end();) {
					if (*n_it == dst_node && *e_it == edge) {
						nodes.erase(n_it);
						edges.erase(e_it);
						found = true;
					} else {
						n_it++;
						e_it++;
					}
				}
				return found;
			}
			template <typename LAM>
			bool disconnectIf(LAM removeConnection) {
				bool found = false;
				auto e_it = edges.begin();
				for (auto n_it = nodes.begin(); n_it != nodes.end();) {
					if (removeConnection(*n_it, *e_it)) {
						nodes.erase(n_it);
						edges.erase(e_it);
						found = true;
					} else {
						n_it++;
						e_it++;
					}
				}
				return found;
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

		bool m_directed, m_reversible;
};


// Node Type Generic Graph:

template<class NODE_T, typename NATIVE_NODE_T, typename = typename std::enable_if<!std::is_same<NODE_T, NATIVE_NODE_T>::value>::type>
class BijectiveGenericNodeContainer {
	public:
		inline NODE_T& operator[](NATIVE_NODE_T node_ind) {return m_ind_to_node[node_ind];}
		inline const NODE_T& operator[](NATIVE_NODE_T node_ind) const {return m_ind_to_node[node_ind];}
		inline NATIVE_NODE_T operator[](const NODE_T& node) const {return m_node_to_ind.at(node);}
		std::pair<NATIVE_NODE_T, bool> tryInsert(const NODE_T& node) {
			if (!m_node_to_ind.contains(node)) {
				m_ind_to_node.push_back(node);
				NATIVE_NODE_T ind = m_ind_to_node.size() - 1;
				m_node_to_ind[node] = ind;
				return {ind, true};
			} else {
				return {m_node_to_ind.at(node), false};
			}
		}
		inline std::size_t size() const {return m_ind_to_node.size();}
		void clear() {m_ind_to_node.clear(); m_node_to_ind.clear();}
	protected:
		std::vector<NODE_T> m_ind_to_node;
		std::unordered_map<NODE_T, NATIVE_NODE_T> m_node_to_ind;
};


template<class NODE_T, class EDGE_T, typename NATIVE_NODE_T = Node>
class NodeGenericGraph : public Graph<EDGE_T, NATIVE_NODE_T> {
	public:
	 	typedef NODE_T node_t;
		typedef std::string(*NodeToStrFunction)(const NODE_T&);
	public:
		NodeGenericGraph(bool directed = true, bool reversible = false, Graph<EDGE_T>::EdgeToStrFunction edgeToStr = nullptr, NodeToStrFunction nodeToStr = nullptr)
			: Graph<EDGE_T>(directed, reversible, edgeToStr)
		{}
		
		virtual ~NodeGenericGraph() {}

		inline std::vector<NODE_T> getChildrenGenericNodes(NATIVE_NODE_T node) {
			std::vector<NODE_T> nodes(this->m_graph[node].forward.nodes.size());
			for (uint32_t i=0; i < nodes.size(); ++i) nodes[i] = m_node_container[this->m_graph[node].forward.nodes[i]];
			return nodes;
		}

		inline const std::vector<NATIVE_NODE_T>& getParentsGenericNodes(NATIVE_NODE_T node) {
			std::vector<NODE_T> nodes(this->m_graph[node].backward.nodes.size());
			for (uint32_t i=0; i < nodes.size(); ++i) nodes[i] = m_node_container[this->m_graph[node].backward.nodes[i]];
			return nodes;
		}

		inline std::vector<NODE_T> getChildrenGenericNodes(const NODE_T& node) {
			NATIVE_NODE_T node_native = m_node_container[node];
			std::vector<NODE_T> nodes(this->m_graph[node_native].forward.nodes.size());
			for (uint32_t i=0; i < nodes.size(); ++i) nodes[i] = m_node_container[this->m_graph[node_native].forward.nodes[i]];
			return nodes;
		}

		inline const std::vector<NATIVE_NODE_T>& getParentsGenericNodes(const NODE_T& node) {
			NATIVE_NODE_T node_native = m_node_container[node];
			std::vector<NODE_T> nodes(this->m_graph[node_native].backward.nodes.size());
			for (uint32_t i=0; i < nodes.size(); ++i) nodes[i] = m_node_container[this->m_graph[node_native].backward.nodes[i]];
			return nodes;
		}

		bool connect(const NODE_T& src_node, const NODE_T& dst_node, const EDGE_T& edge) {
			return Graph<EDGE_T>::connect(m_node_container.tryInsert(src_node).first, m_node_container.tryInsert(dst_node).first, edge);
		}

		inline const BijectiveGenericNodeContainer<NODE_T, NATIVE_NODE_T>& getGenericNodeContainer() const {return m_node_container;}

		virtual void print() const override {
			if (!this->m_edgeToStr) { // Allow program to continue even if print function is not provided
				ERROR("No 'edgeToStr' function provided, cannot print");
				return;
			}
			LOG("Printing graph (size: " << this->size() << ")");
			NATIVE_NODE_T node = 0;
			for (const auto& list : this->m_graph) {
				for (uint32_t i=0; i < list.forward.size(); ++i) {
					if (i == 0) {
						if (m_nodeToStr) {
							PRINT_NAMED("Node " << node << " (" << m_nodeToStr(m_node_container[node]) << ")", "is connected to:");
						} else {
							PRINT_NAMED("Node " << node, "is connected to:");
						}
					}
					PRINT_NAMED("    - child node " << list.forward.nodes[i], "with edge: " << this->m_edgeToStr(list.forward.edges[i]));
				}
				++node;
			}
		}

		virtual void clear() override {Graph<EDGE_T>::clear(); m_node_container.clear();}
	
	protected:
		BijectiveGenericNodeContainer<NODE_T, NATIVE_NODE_T> m_node_container;
		NodeToStrFunction m_nodeToStr;
};
}