#pragma once

#include "tools/Containers.h"
#include "tools/Debug.h"

namespace TP {
namespace Algorithms {

    class Combinatorics {
        public:

            /*
            Computes all permutations amongst options specified by n_options_arr 
            */
            static uint32_t permutationsSize(const Containers::SizedArray<uint32_t>& n_options_arr) {
                uint32_t sz = 1;
                for (uint32_t i=0; i<n_options_arr.size(); ++i) sz *= n_options_arr[i];
                return sz;
            }

            template <typename LAM>
            static void permutations(const Containers::SizedArray<uint32_t>& n_options_arr, LAM onPermutation) {
                Containers::SizedArray<uint32_t> option_indices(n_options_arr.size(), 0);
                while (option_indices.back() < n_options_arr.back()) {
                    onPermutation(option_indices);
                    option_indices[0]++;
                    for (uint32_t i=0; i < n_options_arr.size(); ++i) {
                        if (option_indices[i] >= n_options_arr[i] && (i < n_options_arr.size() - 1)) {
                            option_indices[i] = 0;
                            ++option_indices[i+1];
                        }
                    }
                }
            }

    };

}
}