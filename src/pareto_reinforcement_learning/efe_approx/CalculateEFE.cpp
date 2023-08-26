#include "Benchmark.h"

using namespace PRL;

int main(int argc, char* argv[]) {

	GF::ArgParser parser(argc, argv);

	auto fake_plan_file = parser.parse<std::string>("filepath", 'f', "Fake plan file");
	auto data_file = parser.parse<std::string>("date-filepath", 'd', "File to append data to");
	auto n_samples = parser.parse<uint32_t>("n-efe-samples", 'n', 3000, "Number of EFE samples");

    parser.enableHelp();

    GF::Deserializer dszr(fake_plan_file.get());
    FakePlan fake_plan(n_samples.get());
    fake_plan.deserialize(dszr);

    float efe = fake_plan.calculateEFE();
    //std::printf("efe: %.10f\n", efe);

    GF::Serializer szr(data_file.get(), std::ios::app);
    YAML::Emitter& out = szr.get();
    out << YAML::Key << "EFE" << YAML::Value << efe;
    szr.done();
    return 0;
}