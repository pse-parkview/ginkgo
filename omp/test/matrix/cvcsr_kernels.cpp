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

#include <ginkgo/core/matrix/cvcsr.hpp>


#include <random>


#include <gtest/gtest.h>


#include <ginkgo/core/base/exception.hpp>
#include <ginkgo/core/base/exception_helpers.hpp>
#include <ginkgo/core/base/executor.hpp>
#include <ginkgo/core/matrix/csr.hpp>
#include <ginkgo/core/matrix/dense.hpp>
#include <ginkgo/core/matrix/diagonal.hpp>


#include "core/matrix/cvcsr_kernels.hpp"
#include "core/test/utils.hpp"
#include "core/test/utils/unsort_matrix.hpp"


namespace {


class Cvcsr : public ::testing::Test {
protected:
    using Mtx = gko::matrix::Cvcsr<>;
    using Vec = gko::matrix::Dense<>;
    using ComplexVec = gko::matrix::Dense<std::complex<double>>;

    Cvcsr() : mtx_size(532, 231), rand_engine(42) {}

    void SetUp()
    {
        ref = gko::ReferenceExecutor::create();
        omp = gko::OmpExecutor::create();
    }

    void TearDown()
    {
        if (omp != nullptr) {
            ASSERT_NO_THROW(omp->synchronize());
        }
    }

    template <typename MtxType = Vec>
    std::unique_ptr<MtxType> gen_mtx(int num_rows, int num_cols,
                                     int min_nnz_row)
    {
        return gko::test::generate_random_matrix<MtxType>(
            num_rows, num_cols,
            std::uniform_int_distribution<>(min_nnz_row, num_cols),
            std::normal_distribution<>(-1.0, 1.0), rand_engine, ref);
    }

    void set_up_apply_data(int num_vectors = 1)
    {
        mtx = Mtx::create(ref);
        mtx->copy_from(gen_mtx(mtx_size[0], mtx_size[1], 1));
        expected = gen_mtx(mtx_size[0], num_vectors, 1);
        y = gen_mtx(mtx_size[1], num_vectors, 1);
        alpha = gko::initialize<Vec>({2.0}, ref);
        beta = gko::initialize<Vec>({-1.0}, ref);
        dmtx = Mtx::create(omp);
        dmtx->copy_from(mtx.get());
        dresult = Vec::create(omp);
        dresult->copy_from(expected.get());
        dy = Vec::create(omp);
        dy->copy_from(y.get());
        dalpha = Vec::create(omp);
        dalpha->copy_from(alpha.get());
        dbeta = Vec::create(omp);
        dbeta->copy_from(beta.get());
    }

    struct matrix_pair {
        std::unique_ptr<Mtx> ref;
        std::unique_ptr<Mtx> omp;
    };


    std::shared_ptr<gko::ReferenceExecutor> ref;
    std::shared_ptr<const gko::OmpExecutor> omp;

    const gko::dim<2> mtx_size;
    std::ranlux48 rand_engine;

    std::unique_ptr<Mtx> mtx;
    std::unique_ptr<Vec> expected;
    std::unique_ptr<Vec> y;
    std::unique_ptr<Vec> alpha;
    std::unique_ptr<Vec> beta;

    std::unique_ptr<Mtx> dmtx;
    std::unique_ptr<Vec> dresult;
    std::unique_ptr<Vec> dy;
    std::unique_ptr<Vec> dalpha;
    std::unique_ptr<Vec> dbeta;
};


TEST_F(Cvcsr, SimpleApplyIsEquivalentToRef)
GKO_NOT_IMPLEMENTED;
//{
// TODO (script:cvcsr): change the code imported from matrix/coo if needed
//    set_up_apply_data();
//
//    mtx->apply(y.get(), expected.get());
//    dmtx->apply(dy.get(), dresult.get());
//
//    GKO_ASSERT_MTX_NEAR(dresult, expected, 1e-14);
//}


TEST_F(Cvcsr, AdvancedApplyIsEquivalentToRef)
GKO_NOT_IMPLEMENTED;
//{
// TODO (script:cvcsr): change the code imported from matrix/coo if needed
//    set_up_apply_data();
//
//    mtx->apply(alpha.get(), y.get(), beta.get(), expected.get());
//    dmtx->apply(dalpha.get(), dy.get(), dbeta.get(), dresult.get());
//
//    GKO_ASSERT_MTX_NEAR(dresult, expected, 1e-14);
//}


TEST_F(Cvcsr, SimpleApplyAddIsEquivalentToRef)
GKO_NOT_IMPLEMENTED;
//{
// TODO (script:cvcsr): change the code imported from matrix/coo if needed
//    set_up_apply_data();
//
//    mtx->apply2(y.get(), expected.get());
//    dmtx->apply2(dy.get(), dresult.get());
//
//    GKO_ASSERT_MTX_NEAR(dresult, expected, 1e-14);
//}


TEST_F(Cvcsr, AdvancedApplyAddIsEquivalentToRef)
GKO_NOT_IMPLEMENTED;
//{
// TODO (script:cvcsr): change the code imported from matrix/coo if needed
//    set_up_apply_data();
//
//    mtx->apply2(alpha.get(), y.get(), expected.get());
//    dmtx->apply2(dalpha.get(), dy.get(), dresult.get());
//
//    GKO_ASSERT_MTX_NEAR(dresult, expected, 1e-14);
//}


TEST_F(Cvcsr, SimpleApplyToDenseMatrixIsEquivalentToRef)
GKO_NOT_IMPLEMENTED;
//{
// TODO (script:cvcsr): change the code imported from matrix/coo if needed
//    set_up_apply_data(3);
//
//    mtx->apply(y.get(), expected.get());
//    dmtx->apply(dy.get(), dresult.get());
//
//    GKO_ASSERT_MTX_NEAR(dresult, expected, 1e-14);
//}


TEST_F(Cvcsr, AdvancedApplyToDenseMatrixIsEquivalentToRef)
GKO_NOT_IMPLEMENTED;
//{
// TODO (script:cvcsr): change the code imported from matrix/coo if needed
//    set_up_apply_data(3);
//
//    mtx->apply(alpha.get(), y.get(), beta.get(), expected.get());
//    dmtx->apply(dalpha.get(), dy.get(), dbeta.get(), dresult.get());
//
//    GKO_ASSERT_MTX_NEAR(dresult, expected, 1e-14);
//}


TEST_F(Cvcsr, SimpleApplyAddToDenseMatrixIsEquivalentToRef)
GKO_NOT_IMPLEMENTED;
//{
// TODO (script:cvcsr): change the code imported from matrix/coo if needed
//    set_up_apply_data(3);
//
//    mtx->apply2(y.get(), expected.get());
//    dmtx->apply2(dy.get(), dresult.get());
//
//    GKO_ASSERT_MTX_NEAR(dresult, expected, 1e-14);
//}


TEST_F(Cvcsr, SimpleApplyAddToDenseMatrixIsEquivalentToRefUnsorted)
GKO_NOT_IMPLEMENTED;
//{
// TODO (script:cvcsr): change the code imported from matrix/coo if needed
//    set_up_apply_data(3);
//    auto pair = gen_unsorted_mtx();
//
//    pair.ref->apply2(y.get(), expected.get());
//    pair.omp->apply2(dy.get(), dresult.get());
//
//    GKO_ASSERT_MTX_NEAR(dresult, expected, 1e-14);
//}


TEST_F(Cvcsr, AdvancedApplyAddToDenseMatrixIsEquivalentToRef)
GKO_NOT_IMPLEMENTED;
//{
// TODO (script:cvcsr): change the code imported from matrix/coo if needed
//    set_up_apply_data(3);
//
//    mtx->apply2(alpha.get(), y.get(), expected.get());
//    dmtx->apply2(dalpha.get(), dy.get(), dresult.get());
//
//    GKO_ASSERT_MTX_NEAR(dresult, expected, 1e-14);
//}


TEST_F(Cvcsr, ApplyToComplexIsEquivalentToRef)
GKO_NOT_IMPLEMENTED;
//{
// TODO (script:cvcsr): change the code imported from matrix/coo if needed
//    set_up_apply_data();
//    auto complex_b = gen_mtx<ComplexVec>(231, 3, 1);
//    auto dcomplex_b = ComplexVec::create(omp);
//    dcomplex_b->copy_from(complex_b.get());
//    auto complex_x = gen_mtx<ComplexVec>(532, 3, 1);
//    auto dcomplex_x = ComplexVec::create(omp);
//    dcomplex_x->copy_from(complex_x.get());
//
//    mtx->apply(complex_b.get(), complex_x.get());
//    dmtx->apply(dcomplex_b.get(), dcomplex_x.get());
//
//    GKO_ASSERT_MTX_NEAR(dcomplex_x, complex_x, 1e-14);
//}


TEST_F(Cvcsr, AdvancedApplyToComplexIsEquivalentToRef)
GKO_NOT_IMPLEMENTED;
//{
// TODO (script:cvcsr): change the code imported from matrix/coo if needed
//    set_up_apply_data();
//    auto complex_b = gen_mtx<ComplexVec>(231, 3, 1);
//    auto dcomplex_b = ComplexVec::create(omp);
//    dcomplex_b->copy_from(complex_b.get());
//    auto complex_x = gen_mtx<ComplexVec>(532, 3, 1);
//    auto dcomplex_x = ComplexVec::create(omp);
//    dcomplex_x->copy_from(complex_x.get());
//
//    mtx->apply(alpha.get(), complex_b.get(), beta.get(), complex_x.get());
//    dmtx->apply(dalpha.get(), dcomplex_b.get(), dbeta.get(),
//    dcomplex_x.get());
//
//    GKO_ASSERT_MTX_NEAR(dcomplex_x, complex_x, 1e-14);
//}


TEST_F(Cvcsr, ApplyAddToComplexIsEquivalentToRef)
GKO_NOT_IMPLEMENTED;
//{
// TODO (script:cvcsr): change the code imported from matrix/coo if needed
//    set_up_apply_data();
//    auto complex_b = gen_mtx<ComplexVec>(231, 3, 1);
//    auto dcomplex_b = ComplexVec::create(omp);
//    dcomplex_b->copy_from(complex_b.get());
//    auto complex_x = gen_mtx<ComplexVec>(532, 3, 1);
//    auto dcomplex_x = ComplexVec::create(omp);
//    dcomplex_x->copy_from(complex_x.get());
//
//    mtx->apply2(alpha.get(), complex_b.get(), complex_x.get());
//    dmtx->apply2(dalpha.get(), dcomplex_b.get(), dcomplex_x.get());
//
//    GKO_ASSERT_MTX_NEAR(dcomplex_x, complex_x, 1e-14);
//}


TEST_F(Cvcsr, ConvertToCsrIsEquivalentToRef)
GKO_NOT_IMPLEMENTED;
//{
// TODO (script:cvcsr): change the code imported from matrix/coo if needed
//    set_up_apply_data();
//    auto csr_mtx = gko::matrix::Csr<>::create(ref);
//    auto dcsr_mtx = gko::matrix::Csr<>::create(omp);
//
//    mtx->convert_to(csr_mtx.get());
//    dmtx->convert_to(dcsr_mtx.get());
//
//    GKO_ASSERT_MTX_NEAR(csr_mtx.get(), dcsr_mtx.get(), 1e-14);
//}


TEST_F(Cvcsr, ExtractDiagonalIsEquivalentToRef)
GKO_NOT_IMPLEMENTED;
//{
// TODO (script:cvcsr): change the code imported from matrix/coo if needed
//    set_up_apply_data();
//
//    auto diag = mtx->extract_diagonal();
//    auto ddiag = dmtx->extract_diagonal();
//
//    GKO_ASSERT_MTX_NEAR(diag.get(), ddiag.get(), 0);
//}


TEST_F(Cvcsr, InplaceAbsoluteMatrixIsEquivalentToRef)
GKO_NOT_IMPLEMENTED;
//{
// TODO (script:cvcsr): change the code imported from matrix/coo if needed
//    set_up_apply_data();
//
//    mtx->compute_absolute_inplace();
//    dmtx->compute_absolute_inplace();
//
//    GKO_ASSERT_MTX_NEAR(mtx, dmtx, 1e-14);
//}


TEST_F(Cvcsr, OutplaceAbsoluteMatrixIsEquivalentToRef)
GKO_NOT_IMPLEMENTED;
//{
// TODO (script:cvcsr): change the code imported from matrix/coo if needed
//    set_up_apply_data();
//
//    auto abs_mtx = mtx->compute_absolute();
//    auto dabs_mtx = dmtx->compute_absolute();
//
//    GKO_ASSERT_MTX_NEAR(abs_mtx, dabs_mtx, 1e-14);
//}


}  // namespace