#include "Benchmark.h"

using namespace PRL;

int main(int argc, char* argv[]) {

	GF::ArgParser parser(argc, argv);

	auto fake_plan_file = parser.parse<std::string>("filepath", 'f', "Fake plan file");
	auto n_samples = parser.parse<uint32_t>("n-efe-samples", 'n', 3000, "Number of EFE samples");

    GF::Deserializer dszr(fake_plan_file.get());
    FakePlan fake_plan(n_samples.get());
    LOG("EFE: " << fake_plan.calculateEFE());
    return 0;
}