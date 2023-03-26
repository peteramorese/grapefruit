#include "orderedPlanner.h"


int main() {
    OrderedPlanner::Result res;
    OrderedPlanner::Plan plan = {{}, {}};
    if (res.addParetoPoint(5.0f, 5.0f, plan)) {std::cout<<"add pt 1 worked"<<std::endl;};
    if (res.addParetoPoint(6.0f, 4.0f, plan)) {std::cout<<"add pt 2 worked"<<std::endl;};
    if (res.addParetoPoint(7.0f, 3.5f, plan)) {std::cout<<"add pt 3 worked"<<std::endl;};
    if (res.addParetoPoint(3.0f, 5.5f, plan)) {std::cout<<"add pt 4 worked"<<std::endl;};
    res.printParetoFront();
    if (res.addParetoPoint(5.5f, 3.0f, plan)) {std::cout<<"add pt 5 worked"<<std::endl;};
    res.printParetoFront();
    if (res.addParetoPoint(3.5f, 5.25f, plan)) {std::cout<<"add pt 6 worked"<<std::endl;};
    res.printParetoFront();
    if (!res.addParetoPoint(3.5f, 6.1f, plan)) {std::cout<<"pt 7 was not added (correct)"<<std::endl;};
    res.printParetoFront();
    return 0;
}