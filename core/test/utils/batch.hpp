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


#ifndef GKO_CORE_TEST_UTILS_BATCH_HPP_
#define GKO_CORE_TEST_UTILS_BATCH_HPP_


#include <ginkgo/core/matrix/batch_csr.hpp>
#include <ginkgo/core/matrix/batch_dense.hpp>


namespace gko {
namespace test {


/**
 * Generate a batch of 1D Poisson matrices on the reference executor.
 *
 * @param exec  The reference executor.
 * @param nrows  The size (number of rows) of the generated matrix
 * @param nbatch  The number of Poisson matrices in the batch
 */
template <typename ValueType>
std::shared_ptr<matrix::BatchCsr<ValueType, int>> create_poisson1d_batch(
    std::shared_ptr<const ReferenceExecutor> exec, const int nrows,
    const size_type nbatch)
{
    const int nnz = 3 * (nrows - 2) + 4;
    gko::Array<int> row_ptrs(exec, nrows + 1);
    {
        int *const ra = row_ptrs.get_data();
        ra[0] = 0;
        ra[1] = 2;
        for (int i = 2; i < nrows; i++) {
            ra[i] = ra[i - 1] + 3;
        }
        ra[nrows] = ra[nrows - 1] + 2;
        GKO_ASSERT(ra[nrows] == nnz);
    }
    gko::Array<int> col_idxs(exec, nnz);
    {
        int *const ca = col_idxs.get_data();
        ca[0] = 0;
        ca[1] = 1;
        for (int i = 1; i < nrows - 1; i++) {
            const int rstart = row_ptrs.get_const_data()[i];
            ca[rstart] = i - 1;
            ca[rstart + 1] = i;
            ca[rstart + 2] = i + 1;
        }
        const int lrstart = row_ptrs.get_const_data()[nrows - 1];
        ca[lrstart] = nrows - 2;
        ca[lrstart + 1] = nrows - 1;
    }
    gko::Array<ValueType> vals(exec, nnz * nbatch);
    for (int i = 0; i < nbatch; i++) {
        ValueType *const va = vals.get_data() + i * nnz;
        va[0] = 2.0;
        va[1] = -1.0;
        for (int i = 1; i < nrows - 1; i++) {
            const int rstart = row_ptrs.get_const_data()[i];
            va[rstart] = -1.0;
            va[rstart + 1] = 2.0;
            va[rstart + 2] = -1.0;
        }
        const int lrstart = row_ptrs.get_const_data()[nrows - 1];
        va[lrstart] = -1.0;
        va[lrstart + 1] = 2.0;
    }
    using Mtx = matrix::BatchCsr<ValueType, int>;
    return Mtx::create(exec, nbatch, gko::dim<2>{(size_t)nrows, (size_t)nrows},
                       std::move(vals), std::move(col_idxs),
                       std::move(row_ptrs));
}


}  // namespace test
}  // namespace gko


#endif
