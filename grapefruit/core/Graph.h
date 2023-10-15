#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <iostream>

#include "tools/Logging.h"
#include "tools/Containers.h"

namespace GF {

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



template<class EDGE_T, typename NATIVE_NODE_T = Node, bool REVERSIBLE = true, bool DIRECTED = true>
class Graph {
	public:
	 	typedef EDGE_T edge_t; // used for dependent types
	 	typedef NATIVE_NODE_T node_t; // used for dependent types

	public:
		Graph() {}
		virtual ~Graph() {}

		constexpr bool isDirected() {return DIRECTED;}
		constexpr bool isReversible() {return REVERSIBLE;}

		inline std::size_t size() const {return m_graph.size();}

		std::vector<NATIVE_NODE_T> nodes() const {
			std::vector<NATIVE_NODE_T> nodes;
			nodes.reserve(m_graph.size());
			for (Node n=0; n<m_graph.size(); ++n) {
				if (!m_graph[n].forward.empty() || !m_graph[n].backward.empty()) 
					nodes.push_back(n);
			}
			return nodes;
		}

		inline const std::vector<EDGE_T>& outgoingEdges(NATIVE_NODE_T node) const {
			return m_graph[node].forward.edges;
		}

		inline const std::vector<NATIVE_NODE_T>& children(NATIVE_NODE_T node) const {
			return m_graph[node].forward.nodes;
		}
		
		inline const std::vector<EDGE_T>& incomingEdges(NATIVE_NODE_T node) const {
			static_assert(REVERSIBLE, "Graph must be reversible");
			return m_graph[node].backward.edges;
		}

		inline const std::vector<NATIVE_NODE_T>& parents(NATIVE_NODE_T node) const {
			static_assert(REVERSIBLE, "Graph must be reversible");
			return m_graph[node].backward.nodes;
		}
		
		virtual bool connect(NATIVE_NODE_T src, NATIVE_NODE_T dst, const EDGE_T& edge) {
			if (src >= size() || dst >= size()) {
				NATIVE_NODE_T max_ind = (src > dst) ? src : dst;
				m_graph.resize(max_ind + 1);
			}
			m_graph[src];
			m_graph[src].forward.pushConnect(dst, edge);

			if constexpr (REVERSIBLE) m_graph[dst].backward.pushConnect(src, edge);
			return true;
		}

		virtual bool disconnect(NATIVE_NODE_T src, NATIVE_NODE_T dst) {
			ASSERT(src < size() && dst < size(), "Disconnection nodes not found");
			bool found = m_graph[src].forward.disconnect(dst);
			if (!found) return false;

			if constexpr (REVERSIBLE) {
				found = m_graph[dst].backward.disconnect(src);
				ASSERT(found, "Disconnection found in forward, but not backward");
			}
			return true;
		}

		virtual bool disconnect(NATIVE_NODE_T src, NATIVE_NODE_T dst, const EDGE_T& edge) {
			ASSERT(src < size() && dst < size(), "Disconnection nodes not found");
			bool found = m_graph[src].forward.disconnect(dst, edge);
			if (!found) return false;

			if constexpr (REVERSIBLE) {
				found = m_graph[dst].backward.disconnect(src);
				ASSERT(found, "Disconnection found in forward, but not backward");
			}
			return true;
		}

		template <typename LAM>
		void disconnectIf(NATIVE_NODE_T src, LAM removeConnection) {
			auto removeConnectionWrapper = [&, this](NATIVE_NODE_T dst, const EDGE_T& edge) {
				bool remove = removeConnection(dst, edge);
				if constexpr (REVERSIBLE) {
					if (remove) {
						m_graph[dst].backward.disconnect(src, edge);
					}
				}
				return remove;
			};
			m_graph[src].forward.disconnectIf(removeConnectionWrapper);
		}

		template <typename LAM>
		void rdisconnectIf(NATIVE_NODE_T dst, LAM removeConnection) {
			static_assert(REVERSIBLE, "Graph must be reversible to use r methods");
			auto removeConnectionWrapper = [&, this](NATIVE_NODE_T src, const EDGE_T& edge) {
				bool remove = removeConnection(src, edge);
				if (remove) {
					m_graph[src].forward.disconnect(dst, edge);
				}
				return remove;
			};
			m_graph[dst].backward.disconnectIf(removeConnectionWrapper);
		}

		void reverse() {
			static_assert(REVERSIBLE, "Cannot reverse a graph that is not reversible");
			for (auto& adj_list : m_graph) {
				swap(adj_list.forward, adj_list.backward);
			}
		}

		template <typename LAM>
		void print(LAM edgeToStr) const {
			LOG("Printing graph (" << nodes().size() << " nodes)");
			NATIVE_NODE_T node = 0;
			for (const auto& list : m_graph) {
				for (uint32_t i=0; i < list.forward.size(); ++i) {
					if (i == 0) PRINT_NAMED("Node " << node, "is connected to:");
					PRINT_NAMED("    - child node " << list.forward.nodes[i], "with edge: " << edgeToStr(list.forward.edges[i]));
				}
				++node;
			}
		}

		template <typename LAM>
		void rprint(LAM edgeToStr) const {
			static_assert(REVERSIBLE, "Graph must be reversible to print reverse");
			LOG("Printing reversed graph (" << nodes().size() << " nodes)");
			NATIVE_NODE_T node = 0;
			for (const auto& list : m_graph) {
				for (uint32_t i=0; i < list.backward.size(); ++i) {
					if (i == 0) PRINT_NAMED("Node " << node, "is connected to:");
					PRINT_NAMED("    - parent node " << list.backward.nodes[i], "with edge: " << edgeToStr(list.backward.edges[i]));
				}
				++node;
			}
		}

		virtual void clear() {m_graph.clear();}
	
	protected:
		struct AdjacencyList {
			std::size_t pushConnect(NATIVE_NODE_T dst_node, const EDGE_T& edge) {
				edges.push_back(edge);
				nodes.push_back(dst_node);
				return nodes.size();
			}
			template <typename ... Args>
			constexpr std::size_t emplaceConnect(NATIVE_NODE_T dst_node, Args&& ... args) {
				edges.emplace_back(std::forward<Args>(args)...);
				nodes.push_back(dst_node);
				return nodes.size();
			}
			std::size_t disconnect(NATIVE_NODE_T dst_node) {
				auto removeConnection = [dst_node] (NATIVE_NODE_T n, const EDGE_T& e) -> bool {
					return n == dst_node;
				};
				return disconnectIf(removeConnection);
			}
			std::size_t disconnect(NATIVE_NODE_T dst_node, const EDGE_T& edge) {
				auto removeConnection = [dst_node, &edge] (NATIVE_NODE_T n, const EDGE_T& e) -> bool {
					return n == dst_node && e == edge;
				};
				return disconnectIf(removeConnection);
			}
			template <typename LAM>
			std::size_t disconnectIf(LAM removeConnection) {
				std::size_t n_removed;
				auto e_it = edges.begin();
				for (auto n_it = nodes.begin(); n_it != nodes.end();) {
					if (removeConnection(*n_it, *e_it)) {
						nodes.erase(n_it);
						edges.erase(e_it);
						++n_removed;
					} else {
						++n_it;
						++e_it;
					}
				}
				return n_removed;
			}

			inline std::size_t size() const {return edges.size();}

			inline bool empty() const {return edges.empty();}

			friend void swap(AdjacencyList& lhs, AdjacencyList& rhs) {
				std::swap(lhs.nodes, rhs.nodes);
				std::swap(lhs.edges, rhs.edges);
			}

			std::vector<NATIVE_NODE_T> nodes;
			std::vector<EDGE_T> edges;
		};
		
		struct BidirectionalConnectionList {
			AdjacencyList forward;
			AdjacencyList backward;
		};

	protected:
		Containers::RandomAccessList<BidirectionalConnectionList> m_graph;
		std::size_t m_size;
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
		inline bool contains(const NODE_T& node) const {return m_node_to_ind.contains(node);}
		inline std::size_t size() const {return m_ind_to_node.size();}
		void clear() {m_ind_to_node.clear(); m_node_to_ind.clear();}
	protected:
		std::vector<NODE_T> m_ind_to_node;
		std::unordered_map<NODE_T, NATIVE_NODE_T> m_node_to_ind;
};


template<class NODE_T, class EDGE_T, typename NATIVE_NODE_T = Node, bool REVERSIBLE = true, bool DIRECTED = true>
class NodeGenericGraph : public Graph<EDGE_T, NATIVE_NODE_T, REVERSIBLE, DIRECTED> {
	public:
	 	typedef NODE_T node_t;
		//typedef std::string(*NodeToStrFunction)(const NODE_T&);
	public:
		NodeGenericGraph() {}
		
		virtual ~NodeGenericGraph() {}

		inline std::vector<NODE_T> getChildrenGenericNodes(NATIVE_NODE_T node) {
			const std::vector<NATIVE_NODE_T>& children = this->children(node);
			std::vector<NODE_T> nodes(children.size());
			for (uint32_t i=0; i < children.size(); ++i) nodes[i] = m_node_container[children[i]];
			return nodes;
		}

		inline const std::vector<NATIVE_NODE_T>& getParentsGenericNodes(NATIVE_NODE_T node) {
			const std::vector<NATIVE_NODE_T>& parents = this->parents(node);
			std::vector<NODE_T> nodes(parents.size());
			for (uint32_t i=0; i < parents.size(); ++i) nodes[i] = m_node_container[parents[i]];
			return nodes;
		}

		inline std::vector<NODE_T> getChildrenGenericNodes(const NODE_T& node) {
			return getChildrenGenericNodes(m_node_container[node]);
		}

		inline const std::vector<NATIVE_NODE_T>& getParentsGenericNodes(const NODE_T& node) {
			return getParentsGenericNodes(m_node_container[node]);
		}

		bool connect(const NODE_T& src_node, const NODE_T& dst_node, const EDGE_T& edge) {
			return Graph<EDGE_T>::connect(m_node_container.tryInsert(src_node).first, m_node_container.tryInsert(dst_node).first, edge);
		}

		inline const BijectiveGenericNodeContainer<NODE_T, NATIVE_NODE_T>& getGenericNodeContainer() const {return m_node_container;}

		template <typename NODE_LAM, typename EDGE_LAM>
		void print(EDGE_LAM edgeToStr) const {
			LOG("Printing graph (size: " << this->size() << ")");
			NATIVE_NODE_T node = 0;
			for (const auto& list : this->m_graph) {
				for (uint32_t i=0; i < list.forward.size(); ++i) {
					if (i == 0) PRINT_NAMED("Node " << node << " (" << static_cast<std::string>(m_node_container[node]) << ")", "is connected to:");
					PRINT_NAMED("    - child node " << list.forward.nodes[i] << " (" << static_cast<std::string>(m_node_container[list.forward.nodes[i]]) << ")", "with edge: " << edgeToStr(list.forward.edges[i]));
				}
				++node;
			}
		}

		template <typename LAM>
		void rprint(LAM edgeToStr) const {
			static_assert(REVERSIBLE, "Graph must be reversible to print reverse");
			LOG("Printing reversed graph (size: " << this->size() << ")");
			NATIVE_NODE_T node = 0;
			for (const auto& list : this->m_graph) {
				for (uint32_t i=0; i < list.backward.size(); ++i) {
					if (i == 0) PRINT_NAMED("Node " << node << " (" << static_cast<std::string>(m_node_container[node]) << ")", "is connected to:");
					PRINT_NAMED("    - parent node " << list.backward.nodes[i] << " (" << static_cast<std::string>(m_node_container[list.backward.nodes[i]]) << ")", "with edge: " << edgeToStr(list.backward.edges[i]));
				}
				++node;
			}
		}

		virtual void clear() override {Graph<EDGE_T>::clear(); m_node_container.clear();}
	
	protected:
		BijectiveGenericNodeContainer<NODE_T, NATIVE_NODE_T> m_node_container;
};
}