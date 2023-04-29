#include "statistics/GaussianUpdater.h"
#include "statistics/Random.h"
#include "tools/ArgParser.h"

using namespace TP::Stats;

int main(int argc, char** argv) {

    TP::ArgParser parser(argc, argv);

    uint32_t n_samples = parser.parse<uint32_t>("n-samples", 30, "Number of samples");
    uint32_t print_interval = parser.parse<uint32_t>("print-interval", 1, "Print after an interval");

    if (parser.enableHelp()) return 0;

    GaussianUpdater upd(0.0f, 0.0f, 3, 1.0f);
  
    Distributions::Normal true_dist(15.0f, 4.0f);


    for (uint32_t i = 0; i < n_samples; ++i) {
        float sample = TP::RNG::nrand(true_dist);
        upd.update(sample);
        if (i % print_interval == 0) {
            Distributions::Normal estimate = upd.getEstimateNormal();
            LOG("Estimated normal distribution: (mean: " << E(estimate) << " variance: " << var(estimate) << ")");
        }
    }
  
    return 0;
}
