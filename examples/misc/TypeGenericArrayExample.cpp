
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include "tools/Logging.h"
#include "tools/Containers.h"

using namespace TP::Containers;

template<typename T>
void printTGA(const std::string& name, const T& tga) {
    std::stringstream stream;
    stream << "elements: ";
    auto printTGA_ = [&]<typename E>(const E& element) {
        stream << element << " ";
        return true;
    };

    tga.forEach(printTGA_);
    PRINT_NAMED(name, stream.str());
}

int main() {
    TypeGenericArray<double, int, float> A(0.0, 1, 3.0f);
    TypeGenericArray<double, int, float> B(0.0, 1, 3.0f);
    TypeGenericArray<std::string, float, float, int> C("I am C", 6.0f, 3.0f, 10);
    TypeGenericArray<double, int, float> D(1.0, 3, 3.2f);
    TypeGenericArray<double, int, float> E(0.9, 2, 3.3f);
    TypeGenericArray<double, int, float> F(0.9, 1, 3.4f);

    // Iterate thru each element
    printTGA("A", A);
    printTGA("B", B);
    printTGA("C", C);
    printTGA("D", D);
    printTGA("E", E);
    printTGA("F", F);

    // Compare
    PRINT_NAMED("A == B", std::boolalpha << (A == B));
    PRINT_NAMED("A == D", std::boolalpha << (A == D));

    PRINT_NAMED("B < D", std::boolalpha << (B < D));
    PRINT_NAMED("B > D", std::boolalpha << (B > D));
    PRINT_NAMED("D < E", std::boolalpha << (D < E));

    PRINT_NAMED("E lexicographicLess D", std::boolalpha << E.lexicographicLess(D));
    PRINT_NAMED("E lexicographicGreater F", std::boolalpha << E.lexicographicGreater(F));

    PRINT_NAMED("D dominates F", std::boolalpha << D.dominates(F));
    PRINT_NAMED("A dominates E", std::boolalpha << A.dominates(E));

    SizedArray<int> test(4);
    std::vector<int> test_v = {1, 4, 2, 0};
    for (int i=0; i<4; ++i) test[i] = test_v[i];
    //LOG("vector sz: "<< sizeof(test_v) << " sa sz: " << sizeof(test));

    //auto v = test.toVector();
    std::sort(test.begin(), test.end());
    for (int i=0; i<4; ++i) LOG(test[i]);

    return 0;
}