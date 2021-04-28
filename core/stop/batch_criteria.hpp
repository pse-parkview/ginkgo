/*******************************<GINKGO LICENSE>******************************
Copyright (c) 2017-2021, the Ginkgo authors
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************<GINKGO LICENSE>*******************************/


#ifndef GKO_CORE_STOP_BATCH_CRITERIA_HPP_
#define GKO_CORE_STOP_BATCH_CRITERIA_HPP_

namespace stop {

namespace detail {


constexpr uint32 power(const uint32 x, const int N)
{
    uint32 ans = 1;
    for (int i = 0; i < N; i++) {
        ans *= x;
    }
    return ans;
}


}  // namespace detail


// TODO: Remove this
enum class tolerance {

    absolute,
    relative

};

/**
 * A stopping criterion for batch solvers that comprises a
 * maximum iteration count as well as relative residual tolerance or absolute
 * residual tolerance - which one to consider can be chosen.
 *
 * At most 32 right-hand-side vectors are supported.
 */
template <typename ValueType>
class AbsAndRelResidualMaxIter {
public:
    using real_type = remove_complex<ValueType>;
    // using tolerance = ::gko::stop::batch::ToleranceType;
    using bitset_type = uint32;
    static constexpr int max_nrhs = 32;

    /**
     * Set up the stopping criterion and convergence variable.
     *
     * @param num_rhs  The number of right-hand-sides in the linear systems.
     * @param max_iters  Maximum number of iterations allowed.
     * @param converged_bitset  A bit-set representing the state of convergence
     *                          of each RHS: 1 for converged and 0 otherwise.
     *                          It is initialized appropriately here, and must
     *                          be passed to the \ref check_converged function.
     * @param rhs_b_norms  The reference RHS norms.
     */
    GKO_ATTRIBUTES GKO_INLINE AbsAndRelResidualMaxIter(
        const int num_rhs, const int max_iters, const real_type abs_res_tol,
        const real_type rel_res_tol, const tolerance type_of_tol,
        bitset_type &converge_bitset, const real_type *const rhs_b_norms)
        : nrhs{num_rhs},
          rel_tol{rel_res_tol},
          abs_tol{abs_res_tol},
          tol_type{type_of_tol},
          max_its{max_iters},
          rhs_norms{rhs_b_norms}
    {
        if (nrhs > 32) {
            printf("Batch stopping criterion: Too many right hand sides!\n");
        }
        converge_bitset = 0 - (1 << num_rhs);
    }

    /**
     * Checks whether the different right hand sides have converged.
     *
     * @param iter  The current iteration count.
     * @param residual_norms  (Optional) current residual norm of each RHS.
     * @param residual  Current residual vectors. Unused if residual_norms
     *                  are provided.
     * @param converged  Bits representing converged (1) or not (0) for each
     *                   RHS. The 'right-most' bit corresponds to the first RHS.
     *
     * @return  True if all RHS have converged, false otherwise.
     */
    GKO_ATTRIBUTES GKO_INLINE bool check_converged(
        const int iter, const real_type *const residual_norms,
        const gko::batch_dense::BatchEntry<const ValueType> &residual,
        bitset_type &converged) const;

private:
    int nrhs;
    int max_its;
    const real_type rel_tol;
    const real_type abs_tol;
    const tolerance tol_type;
    const real_type *const rhs_norms;
    static constexpr uint32 all_true = detail::power(2, 32) - 1;

    GKO_ATTRIBUTES GKO_INLINE void check_norms(const real_type *const res_norms,
                                               bitset_type &converged) const;
};

}  // namespace stop

#endif