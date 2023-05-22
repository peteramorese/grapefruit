#include "TaskPlanner.h"

#include <Eigen/Dense>

#include <iostream>
#include <spot/misc/version.hh>

using namespace TP;

int main() {

    Eigen::IOFormat OctaveFmt(3, 0, ", ", ";\n", "", "", "[", "]");

    Stats::Distributions::FixedInverseWishart<2> iwish;
    std::cout << iwish.Psi.format(OctaveFmt) << std::endl;

    return 0;
}
