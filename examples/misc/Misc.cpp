#include "TaskPlanner.h"

#include <Eigen/Dense>

#include <iostream>
#include <spot/misc/version.hh>

using namespace TP;

int main() {

    ParetoFront<Containers::FixedArray<2, float>> pf;
    Containers::FixedArray<2, float> pt0;
    pt0[0] = 1.0f;
    pt0[1] = 5.0f;
    Containers::FixedArray<2, float> pt1;
    pt1[0] = 2.0f;
    pt1[1] = 3.0f;
    Containers::FixedArray<2, float> pt2;
    pt2[0] = 4.0f;
    pt2[1] = 2.5f;
    Containers::FixedArray<2, float> pt3;
    pt3[0] = 5.0f;
    pt3[1] = 0.5f;

    pf.push_back(pt0);
    pf.push_back(pt1);
    pf.push_back(pt2);
    pf.push_back(pt3);

    Containers::FixedArray<2, float> sample;
    sample[0] = 5.0f;
    sample[1] = 4.1f;
    LOG("Regret: " << pf.regret(sample));
    
    return 0;
}
