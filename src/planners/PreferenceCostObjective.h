#pragma once

namespace TP {
namespace Planner {

    // Collection of inerhited costs for each task
    template <typename INHERITED_COST_T>
    struct PreferenceCostSet {
        PreferenceCostSet(uint32_t size) : pcs(size) {}
        Containers::SizedArray<INHERITED_COST_T> pcs;
    };
    

    /*
    Preference cost objective converts the inherited product edge into an objective cost. PreferenceCostObjective is an instantiated plugin to allow
    the user monitor/interact with the function
    */
    template <class INHERITED_EDGE_T, class OBJ_COST_T>
    struct OrderedPreferenceCostObjective {
        //typedef OBJ_COST_T

        //
        OBJ_COST_T operator(WideNode node, INHERITED_EDGE_T&& edge) {

        }
    };
    
} // namespace Planner
} // namespace TP
