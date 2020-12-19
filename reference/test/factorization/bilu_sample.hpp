/*******************************<GINKGO LICENSE>******************************
Copyright (c) 2017-2020, the Ginkgo authors
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


#ifndef GKO_CORE_TEST_BILU_HPP_
#define GKO_CORE_TEST_BILU_HPP_


#include <ginkgo/core/base/composition.hpp>
#include <ginkgo/core/matrix/fbcsr.hpp>


#include "core/components/fixed_block.hpp"


namespace gko {
namespace testing {


template <typename ValueType, typename IndexType>
class Bilu0Sample {
public:
    using value_type = ValueType;
    using index_type = IndexType;
    using Fbcsr = gko::matrix::Fbcsr<value_type, index_type>;


    const size_type nrows = 9;
    const size_type ncols = 9;
    const size_type nnz = 63;
    const size_type nbrows = 3;
    const size_type nbcols = 3;
    const size_type nbnz = 7;
    const int bs = 3;
    const std::shared_ptr<const gko::Executor> exec;


    Bilu0Sample(std::shared_ptr<const gko::ReferenceExecutor> rexec)
        : exec(rexec)
    {}

    /**
     * @return The sample matrix in FBCSR format
     */
    std::unique_ptr<Fbcsr> generate_fbcsr() const
    {
        std::unique_ptr<Fbcsr> mtx =
            Fbcsr::create(exec,
                          gko::dim<2>{static_cast<size_type>(nrows),
                                      static_cast<size_type>(ncols)},
                          nnz, bs);

        value_type *const v = mtx->get_values();
        index_type *const c = mtx->get_col_idxs();
        index_type *const r = mtx->get_row_ptrs();
        r[0] = 0;
        r[1] = 3;
        r[2] = 5;
        r[3] = 7;
        c[0] = 0;
        c[1] = 1;
        c[2] = 2;
        c[3] = 1;
        c[4] = 2;
        c[5] = 0;
        c[6] = 2;

        gko::blockutils::DenseBlocksView<value_type, index_type> vb(v, bs, bs);
        vb(0, 0, 0) = 2.0;
        vb(0, 0, 1) = -1.0;
        vb(0, 0, 2) = 0.0;
        vb(0, 1, 0) = -1.0;
        vb(0, 1, 1) = 2.0;
        vb(0, 1, 2) = -1.0;
        vb(0, 2, 0) = 0.0;
        vb(0, 2, 1) = -1.0;
        vb(0, 2, 2) = 2.0;
        for (int i = 0; i < bs; i++)
            for (int j = 0; j < bs; j++) {
                vb(1, i, j) = 0.0;
                vb(2, i, j) = 0.0;
                vb(4, i, j) = 0.0;
            }
        vb(1, 0, 0) = 1.0;
        vb(1, 1, 1) = 2.0;
        vb(1, 2, 2) = 1.0;
        vb(2, 0, 0) = 2.0;
        vb(2, 1, 1) = 1.0;
        vb(2, 2, 2) = 2.0;
        vb(3, 0, 0) = 4.0;
        vb(3, 0, 1) = 1.0;
        vb(3, 0, 2) = 1.0;
        vb(3, 1, 1) = 2.0;
        vb(3, 1, 1) = 4.0;
        vb(3, 1, 2) = 1.0;
        vb(3, 2, 0) = 0.0;
        vb(3, 2, 1) = 2.0;
        vb(3, 2, 2) = 5.0;
        vb(4, 0, 0) = 2.0;
        vb(4, 0, 1) = 1.0;
        vb(4, 1, 1) = 1.0;
        vb(4, 2, 2) = 1.0;
        vb(5, 0, 0) = 2.0;
        vb(5, 0, 1) = -2.0;
        vb(5, 0, 2) = 2.0;
        vb(5, 1, 0) = 3.0;
        vb(5, 1, 1) = 4.0;
        vb(5, 1, 2) = 5.0;
        vb(5, 2, 0) = vb(5, 2, 1) = vb(5, 2, 2) = 1.0;
        vb(6, 0, 0) = 5.0;
        vb(6, 0, 1) = 2.0;
        vb(6, 0, 2) = -1.0;
        vb(6, 1, 0) = 0.5;
        vb(6, 1, 1) = 10.0;
        vb(6, 1, 2) = 1.0;
        vb(6, 2, 0) = -0.5;
        vb(6, 2, 1) = 1.0;
        vb(6, 2, 2) = 5.0;

        return mtx;
    }

    /**
     * @return Sample matrix in CSR format
     *
     * Keeps explicit zeros.
     */
    std::unique_ptr<Composition<ValueType>> generate_factors() const
    {
        const std::shared_ptr<Fbcsr> fb_l =
            Fbcsr::create(exec, gko::dim<2>{nrows, ncols}, 4 * 9, bs);
        const std::shared_ptr<Fbcsr> fb_u =
            Fbcsr::create(exec, gko::dim<2>{nrows, ncols}, 6 * 9, bs);
        const std::unique_ptr<const Fbcsr> A = generate_fbcsr();

        blockutils::DenseBlocksView<value_type, index_type> vl(
            fb_l->get_values(), bs, bs);
        blockutils::DenseBlocksView<value_type, index_type> vu(
            fb_u->get_values(), bs, bs);
        blockutils::DenseBlocksView<const value_type, index_type> vA(
            A->get_values(), bs, bs);

        for (int i = 0; i < bs; i++) {
            for (int j = 0; j < bs; j++) {
                vu(0, i, j) = vA(0, i, j);
                vu(1, i, j) = vA(1, i, j);
                vu(2, i, j) = vA(2, i, j);
                vu(3, i, j) = vA(3, i, j);
                vu(4, i, j) = vA(4, i, j);

                vl(0, i, j) = 0.0;
                vl(1, i, j) = 0.0;
                vl(3, i, j) = 0.0;
            }
            vl(0, i, i) = 1.0;
            vl(1, i, i) = 1.0;
            vl(3, i, i) = 1.0;

            vl(2, 0, 0) = 1.0;
            vl(2, 0, 1) = 0.0;
            vl(2, 0, 2) = 1.0;
            vl(2, 1, 0) = 5.5;
            vl(2, 1, 1) = 8.0;
            vl(2, 1, 2) = 6.5;
            vl(2, 2, 0) = 1.5;
            vl(2, 2, 1) = 2.0;
            vl(2, 2, 2) = 1.5;

            vu(5, 0, 0) = 3.0;
            vu(5, 0, 1) = 2.0;
            vu(5, 0, 2) = -3.0;
            vu(5, 1, 0) = -10.5;
            vu(5, 1, 1) = 2.0;
            vu(5, 1, 2) = -12.0;
            vu(5, 2, 0) = -3.5;
            vu(5, 2, 1) = -1.0;
            vu(5, 2, 2) = 2.0;
        }

        index_type *const l_r = fb_l->get_row_ptrs();
        l_r[0] = 0;
        l_r[1] = 1;
        l_r[2] = 2;
        l_r[3] = 4;
        index_type *const l_c = fb_l->get_col_idxs();
        l_c[0] = 0;
        l_c[1] = 1;
        l_c[2] = 0;
        l_c[3] = 2;

        index_type *const u_r = fb_u->get_row_ptrs();
        u_r[0] = 0;
        u_r[1] = 3;
        u_r[2] = 5;
        u_r[3] = 6;
        index_type *const u_c = fb_u->get_col_idxs();
        u_c[0] = 0;
        u_c[1] = 1;
        u_c[2] = 2;
        u_c[3] = 1;
        u_c[4] = 2;
        u_c[5] = 2;

        return Composition<ValueType>::create(std::move(fb_l), std::move(fb_u));
    }
};

}  // namespace testing
}  // namespace gko

#endif