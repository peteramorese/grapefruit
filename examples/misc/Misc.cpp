#include "tools/Containers.h"

#include "statistics/Normal.h"

#include "tools/Logging.h"

#include <Eigen/Dense>

#include <iostream>
#include <spot/misc/version.hh>

int main()
{
  std::cout << "Hello world!\nThis is Spot " << spot::version() << ".\n";
  TP::Stats::Distributions::Normal p;
  p.mu = 5.0f;
  LOG("expected val: " << TP::Stats::E(p));
  return 0;
}

//using namespace TP;
//
//template <typename T>
//void print(const Containers::RandomAccessList<T>& l, const std::string& name) {
//    for (uint32_t i = 0; i < l.size(); ++i) LOG(name << "[" << i <<"]: " << l[i]);
//}
//
//int main() {
//
//    Containers::RandomAccessList<int> l;    
//
//    l.push_back(1);
//    l.push_back(5);
//    l.push_back(3);
//    l.push_back(2);
//    l.push_back(7);
//    l.push_back(3);
//
//    print(l, "l");
//
//    Containers::RandomAccessList<int> l2 = l;    
//
//    print(l2, "l2");
//
//    return 0;
//}