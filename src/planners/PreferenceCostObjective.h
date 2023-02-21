#pragma once

namespace TP {
namespace Planner {

    // Collection of inerhited costs for each task
    template <typename INHERITED_COST_T>
    using PreferenceCostSet = Containers::SizedArray<INHERITED_COST_T>;

    /*
    Preference cost objective converts the inherited product edge into an objective cost. PreferenceCostObjective is an instantiated plugin to allow
    the user monitor/interact with the function
    */
    template <typename OBJECTIVE_COST_T>
    struct OrderedPreferenceCostObjective {
        typedef OBJECTIVE_COST_T

        OBJECTIVE_COST_T operator()
    };
    
} // namespace Planner
} // namespace TP
