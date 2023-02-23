#pragma once

#include "tools/Containers.h"

namespace TP {
namespace Planner {

    // Collection of inerhited costs for each task
    template <typename INHERITED_COST_T>
    struct PreferenceCostSet {
        protected:
            // Need default constructed value to represent 'zero'
            PreferenceCostSet() : m_pcs(0) {}
            PreferenceCostSet(uint32_t size) : m_pcs(size) {}
            PreferenceCostSet(const PreferenceCostSet&) = default;

            inline bool empty() const {return (m_pcs.size() == 0) ? true : false}

            // For general cases, represents the conversion from the member pcs into the quantitative comparison
            virtual INHERITED_COST_T preferenceFunction() const = 0;

        protected:
            Containers::SizedArray<INHERITED_COST_T> m_pcs;

        private:
            // Used for comparing paths during graph search
            bool operator<(const PreferenceCostSet& other) const {return preferenceFunction() < otherpreferenceFunction();}

            friend PreferenceCostSet operator+<INHERITED_COST_T>(const PreferenceCostSet& lhs, const PreferenceCostSet& rhs);
    };

    // Sized Array non-member operators
    template <typename INHERITED_COST_T>
    static PreferenceCostSet<INHERITED_COST_T> operator+(const PreferenceCostSet<INHERITED_COST_T>& lhs, const PreferenceCostSet<INHERITED_COST_T>& rhs) {
        if (!lhs.empty() && !rhs.empty()) {
            return lhs + rhs;
        } else if (lhs.empty()) {
            return rhs;
        } else if (rhs.empty()) {
            return lhs;
        } else {
            return PreferenceCostSet{};
        }
    }
    

    /*
    Preference cost objective stores the partial trajectory collapsed cost objective

    The implementation must captures conversion from the inherited edge and node to the underlying cost structure (ctor())
    as well as the less than operator< for quantitatively comparing two paths
    
    Here is an example of an objective that uses a PCS:

    template <class SYMBOLIC_GRAPH_T, class INHERITED_COST_T>
    struct ExamplePreferenceCostObjective : public PreferenceCostSet<INHERITED_COST_T> {

        // The constructor converts from node and edge to the difference pcs
        ExamplePreferenceCostObjective(uint32_t size, const SYMBOLIC_GRAPH_T::node_t& node, SYMBOLIC_GRAPH_T::edge_t&& edge) 
            : PreferenceCostSet<INHERITED_COST_T>(size) 
        {
            ...
        }

        // Convert
        virtual INHERITED_COST_T preferenceFunction() const override {
            ...
        }
    };
    */
    
} // namespace Planner
} // namespace TP
