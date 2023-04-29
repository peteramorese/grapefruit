#pragma once

#include "statistics/Normal.h"
#include "statistics/NormalGamma.h"
#include "statistics/StatTools.h"

namespace TP {
namespace Stats {

class GaussianUpdater {
    public:
        GaussianUpdater(float mu_0, float kappa_0, uint32_t alpha_0, float beta_0)
            : m_ng(mu_0, kappa_0, static_cast<float>(alpha_0), beta_0)
            , m_sample_set()
        {}

        inline void update(float sample) {
            m_sample_set.add(sample);
        }

        Distributions::Normal getEstimateNormal() const {
            Distributions::NormalGamma posterior = m_ng.posterior(m_sample_set);
            return Distributions::Normal(
                E(posterior.meanMarginal()),
                1.0f / E(posterior.precisionMarginal()));
        }

    private:
        Distributions::NormalGamma m_ng;
        SampleSet m_sample_set;
};

}
}