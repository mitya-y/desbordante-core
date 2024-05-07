#include "algorithms/md/hymd/preprocessing/similarity_measure/levenshtein_distance.h"

#include <algorithm>
#include <numeric>
#include <vector>

unsigned LevenshteinDistance(std::string_view l, std::string_view r) noexcept {
    std::size_t r_size = r.size();
    std::size_t l_size = l.size();
    std::vector<unsigned> v0(r_size + 1);
    std::vector<unsigned> v1(r_size + 1);

    std::iota(v0.begin(), v0.end(), 0);

    auto compute_arrays = [&](std::vector<unsigned>& v0, std::vector<unsigned>& v1, unsigned i) {
        v1[0] = i + 1;
        char const li = l[i];

        for (unsigned j = 0; j != r_size;) {
            unsigned const insert_cost = v1[j] + 1;
            unsigned const substition_cost = v0[j] + (li != r[j]);
            ++j;
            unsigned const deletion_cost = v0[j] + 1;

            v1[j] = std::min({deletion_cost, insert_cost, substition_cost});
        }
    };

    auto loop_to_l_size = [&l_size, &v0, &v1, &compute_arrays]() {
        for (unsigned i = 0; i != l_size; ++i) {
            compute_arrays(v0, v1, i);
            ++i;
            compute_arrays(v1, v0, i);
        }
    };

    if (l_size & 1) {
        --l_size;
        loop_to_l_size();
        compute_arrays(v0, v1, l_size);
        return v1[r_size];
    } else {
        loop_to_l_size();
        return v0[r_size];
    }
}