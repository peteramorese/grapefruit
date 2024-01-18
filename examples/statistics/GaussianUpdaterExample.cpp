#include "Grapefruit.h"

using namespace GF::Stats;

int main(int argc, char** argv) {

    GF::ArgParser parser(argc, argv);

    GF::Argument<uint32_t> n_samples = parser.parse<uint32_t>("n-samples", 30, "Number of samples");
    GF::Argument<uint32_t> print_interval = parser.parse<uint32_t>("print-interval", 1, "Print after an interval");
	GF::Argument<std::string> sample_filepath = parser.parse<std::string>("sample-filepath", "Filepath to write samples to for visualization");
    bool mv = parser.parse<void>("mv", "Multivariate test").has();

    parser.enableHelp();

    if (!mv) {
        GaussianUpdater upd(0.0f, 0.0f, 3, 1.0f);
    
        Distributions::Normal true_dist(15.0f, 4.0f);


        for (uint32_t i = 0; i < n_samples.get(); ++i) {
            float sample = GF::RNG::nrand(true_dist);
            upd.update(sample);
            if (i % print_interval.get() == 0) {
                Distributions::Normal estimate = upd.getEstimateNormal();
                LOG("Estimated normal distribution: (mean: " << E(estimate) << " variance: " << var(estimate) << ")");
            }
        }
    } else {
        MultivariateGaussianUpdater<3> mvupd;
        Eigen::Matrix<float, 3, 1> mu;
        mu(0) = 1.0f;
        mu(1) = 2.0f;
        mu(2) = 3.0f;
        Eigen::Matrix<float, 6, 1> Sigma_minimal;
        Sigma_minimal(0) = 0.2f;
        Sigma_minimal(1) = 0.02f;
        Sigma_minimal(2) = 0.03f;
        Sigma_minimal(3) = 0.3f;
        Sigma_minimal(4) = 0.04f;
        Sigma_minimal(5) = 0.4f;
        Distributions::FixedMultivariateNormal<3> mv_true_dist;
        mv_true_dist.mu = mu;
        mv_true_dist.setSigmaFromUniqueElementVector(Sigma_minimal);

        std::vector<Eigen::Matrix<float, 3, 1>> sample_set(n_samples.get());
        Distributions::FixedMultivariateNormalSampler sampler(mv_true_dist);
        for (uint32_t i = 0; i < n_samples.get(); ++i) {
            auto sample = GF::RNG::mvnrand(sampler);
            sample_set[i] = sample;
            mvupd.update(sample);
            if (i % print_interval.get() == 0) {
                auto estimate = mvupd.getEstimateNormal();
                LOG("Estimated normal distribution: \n-mean: \n" << E(estimate) << "\n-variance: \n" << var(estimate) << "\n");
            }
        }

        PRINT_NAMED("True normal distribution", "\n-mean: \n" << E(mv_true_dist) << "\n-variance: \n" << var(mv_true_dist) << "\n");


        if (sample_filepath.has()) {
            LOG("writing...");
            GF::Serializer szr(sample_filepath.get());
            YAML::Emitter& out = szr.get();
            for (uint32_t s = 0; s < n_samples.get(); ++s) {
                out << YAML::Key << "Sample " + std::to_string(s) << YAML::Value << YAML::BeginSeq;
                for (uint32_t i = 0; i < 3; ++i) {
                    out << sample_set[s](i);
                }
                out << YAML::EndSeq;
            }
            szr.done();
        }
    }

  
    return 0;
}
