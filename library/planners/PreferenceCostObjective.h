#pragma once

#include "tools/Containers.h"
#include "tools/Misc.h"

#define REQ_TOLERANCE 0.000000000000001

namespace TP {
namespace Planner {

    template <typename COLLAPSED_COST_T>
    struct PreferenceCostObjective {
        typedef COLLAPSED_COST_T collapsed_cost_t;
    };

    // Collection of inerhited costs for each task
    template <typename COLLAPSED_COST_T>
    struct PreferenceCostSet : public PreferenceCostObjective<COLLAPSED_COST_T>{
        protected:
            // Need default constructed value to represent 'zero'
            PreferenceCostSet() : m_pcs(0) {}
            PreferenceCostSet(uint32_t size) : m_pcs(size) {}
            PreferenceCostSet(const PreferenceCostSet&) = default;


            // For general cases, represents the conversion from the member pcs into the quantitative comparison
            virtual COLLAPSED_COST_T preferenceFunction() const = 0;

            operator COLLAPSED_COST_T() const {return preferenceFunction();}

        protected:
            Containers::SizedArray<COLLAPSED_COST_T> m_pcs;

        public:
            // Used for comparing paths during graph search
            inline bool empty() const {return (m_pcs.size() == 0) ? true : false;}
            bool operator<(const PreferenceCostSet& other) const {return (other.preferenceFunction() > REQ_TOLERANCE) ? preferenceFunction() < other.preferenceFunction() - REQ_TOLERANCE : false;}
            bool operator==(const PreferenceCostSet& other) const {return diff(preferenceFunction(), other.preferenceFunction()) < REQ_TOLERANCE;}
            void operator+=(const PreferenceCostSet& other) {
                if (!empty() && !other.empty()) {
                    for (uint32_t i=0; i<m_pcs.size(); ++i) m_pcs[i] += other.m_pcs[i];
                } else if (empty()) {
                    operator=(other);
                } 
            }
            void operator=(const PreferenceCostSet& other) {m_pcs = other.m_pcs;}

            //friend PreferenceCostSet operator+<COLLAPSED_COST_T>(const PreferenceCostSet& lhs, const PreferenceCostSet& rhs);
    };

    //// Sized Array non-member operators
    //template <typename COLLAPSED_COST_T>
    //static PreferenceCostSet<COLLAPSED_COST_T> operator+(const PreferenceCostSet<COLLAPSED_COST_T>& lhs, const PreferenceCostSet<COLLAPSED_COST_T>& rhs) {
    //    if (!lhs.empty() && !rhs.empty()) {
    //        return lhs + rhs;
    //    } else if (lhs.empty()) {
    //        return rhs;
    //    } else if (rhs.empty()) {
    //        return lhs;
    //    } else {
    //        return PreferenceCostSet{};
    //    }
    //}
    

    /*
    Preference cost objective stores the partial trajectory collapsed cost objective

    The implementation must captures conversion from the inherited edge and node to the underlying cost structure (ctor())
    as well as the less than operator< for quantitatively comparing two paths
    
    Here is an example of an objective that uses a PCS:

    template <class SYMBOLIC_GRAPH_T, class COLLAPSED_COST_T>
    struct ExamplePreferenceCostObjective : public PreferenceCostSet<COLLAPSED_COST_T> {

        // The constructor converts from node and edge to the difference pcs
        ExamplePreferenceCostObjective(const SYMBOLIC_GRAPH_T& sym_graph, const SYMBOLIC_GRAPH_T::node_t& node, SYMBOLIC_GRAPH_T::edge_t&& edge) 
            : PreferenceCostSet<COLLAPSED_COST_T>(sym_graph.rank() - 1) 
        {
            ...
        }

        // Convert
        virtual COLLAPSED_COST_T preferenceFunction() const override {
            ...
        }
    };
    */
    
} // namespace Planner
} // namespace TP
