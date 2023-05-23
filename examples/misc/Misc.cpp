#include "TaskPlanner.h"

#include <Eigen/Dense>

#include <iostream>
#include <spot/misc/version.hh>

using namespace TP;

int main() {

    Eigen::IOFormat OctaveFmt(3, 0, ", ", ";\n", "", "", "[", "]");

    //Stats::Distributions::FixedInverseWishart<2> iwish;

    //iwish.nu = 4.0f;

    //iwish.Psi(0, 0) = 0.3f;
    //iwish.Psi(1, 1) = 0.4f;
    ////iwish.Psi(2, 2) = 0.5f;
    //iwish.Psi(0, 1) = 0.11f;
    //iwish.Psi(1, 0) = 0.11f;
    ////iwish.Psi(0, 2) = 0.22f;
    ////iwish.Psi(2, 0) = 0.22f;
    ////iwish.Psi(1, 2) = 0.12f;
    ////iwish.Psi(2, 1) = 0.12f;

    //std::cout << iwish.Psi.format(OctaveFmt) << std::endl;
    //std::cout << TP::Stats::var(iwish).format(OctaveFmt) << std::endl;

    //Stats::Distributions::MinimalFixedInverseWishart<2> iwish_minimal = Stats::wishartToMinimalWishart(iwish);

    ////std::cout << iwish_minimal.Psi_minimal.format(OctaveFmt) << std::endl;
    //std::cout << TP::Stats::var(iwish_minimal).format(OctaveFmt) << std::endl;

    
    //Stats::Distributions::FixedInverseWishart<2> iwish_back = Stats::minimalWishartToWishart(iwish_minimal);
    //LOG("back to b4");
    //std::cout << iwish_back.Psi.format(OctaveFmt) << std::endl;

    Stats::Distributions::FixedNormalInverseWishart<2> niw;
    niw.nu = 4.0f;
    niw.mu(0) = 5.0f;
    niw.mu(1) = 3.0f;
    niw.Lambda(0, 0) = 0.3f;
    niw.Lambda(1, 1) = 0.4f;
    niw.Lambda(0, 1) = 0.11f;
    niw.Lambda(1, 0) = 0.11f;
    niw.kappa = 2.0f;
    auto mvn = Stats::MomentMatch::niw2mvn(niw);
    std::cout<< mvn.mu << std::endl;
    std::cout<< mvn.Sigma << std::endl;


    return 0;
}
