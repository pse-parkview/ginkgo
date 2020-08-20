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

#include "core/solver/idr_kernels.hpp"


#include <algorithm>


#include <omp.h>


#include <ginkgo/core/base/array.hpp>
#include <ginkgo/core/base/exception_helpers.hpp>
#include <ginkgo/core/base/math.hpp>


namespace gko {
namespace kernels {
namespace omp {
/**
 * @brief The IDR solver namespace.
 *
 * @ingroup idr
 */
namespace idr {


namespace {


template <typename ValueType>
void solve_lower_triangular(const matrix::Dense<ValueType> *m,
                            const matrix::Dense<ValueType> *f,
                            matrix::Dense<ValueType> *c)
{
    const auto nrhs = m->get_size()[1] / m->get_size()[0];
#pragma omp parallel for
    for (size_type i = 0; i < f->get_size()[1]; i++) {
        for (size_type row = 0; row < m->get_size()[0]; row++) {
            auto temp = f->at(row, i);
            for (size_type col = 0; col < row; col++) {
                temp -= m->at(row, col * nrhs + i) * c->at(col, i);
            }
            c->at(row, i) = temp / m->at(row, row * nrhs + i);
        }
    }
}


template <typename ValueType>
void update_g_and_u(size_type k, const matrix::Dense<ValueType> *p,
                    const matrix::Dense<ValueType> *m,
                    matrix::Dense<ValueType> *g, matrix::Dense<ValueType> *u)
{
    const auto nrhs = m->get_size()[1] / m->get_size()[0];
#pragma omp parallel for
    for (size_type i = 0; i < nrhs; i++) {
        for (size_type j = 0; j < k; j++) {
            auto alpha = zero<ValueType>();
            for (size_type ind = 0; ind < p->get_size()[1]; ind++) {
                alpha += p->at(j, ind) * g->at(ind, k * nrhs + i);
            }
            alpha /= m->at(j, j * nrhs + i);

            for (size_type row = 0; row < g->get_size()[0]; row++) {
                g->at(row, k * nrhs + i) -= alpha * g->at(row, j * nrhs + i);
                u->at(row, k * nrhs + i) -= alpha * u->at(row, j * nrhs + i);
            }
        }
    }
}


}  // namespace


template <typename ValueType>
void initialize(std::shared_ptr<const OmpExecutor> exec,
                matrix::Dense<ValueType> *m,
                Array<stopping_status> *stop_status)
{
    const auto nrhs = m->get_size()[1] / m->get_size()[0];

#pragma omp parallel for
    for (size_type i = 0; i < nrhs; i++) {
        stop_status->get_data()[i].reset();
    }

#pragma omp parallel for
    for (size_type row = 0; row < m->get_size()[0]; row++) {
        for (size_type col = 0; col < m->get_size()[1]; col++) {
            m->at(row, col) =
                (row == col / nrhs) ? one<ValueType>() : zero<ValueType>();
        }
    }
}

GKO_INSTANTIATE_FOR_EACH_VALUE_TYPE(GKO_DECLARE_IDR_INITIALIZE_KERNEL);


template <typename ValueType>
void step_1(std::shared_ptr<const OmpExecutor> exec, const size_type k,
            const matrix::Dense<ValueType> *m,
            const matrix::Dense<ValueType> *f,
            const matrix::Dense<ValueType> *residual,
            const matrix::Dense<ValueType> *g, matrix::Dense<ValueType> *c,
            matrix::Dense<ValueType> *v,
            const Array<stopping_status> *stop_status)
{
    const auto m_size = m->get_size();
    const auto nrhs = f->get_size()[1];

    for (size_type i = 0; i < nrhs; i++) {
        if (stop_status->get_const_data()[0].has_stopped()) {
            continue;
        }
    }

    // Compute c = M \ f
    solve_lower_triangular(m, f, c);

    for (size_type i = 0; i < nrhs; i++) {
        // v = residual - c_k * g_k - ... - c_s * g_s
#pragma omp parallel for
        for (size_type row = 0; row < v->get_size()[0]; row++) {
            auto temp = residual->at(row, i);
            for (size_type j = k; j < m->get_size()[0]; j++) {
                temp -= c->at(j, i) * g->at(row, j * nrhs + i);
            }
            v->at(row, i) = temp;
        }
    }
}

GKO_INSTANTIATE_FOR_EACH_VALUE_TYPE(GKO_DECLARE_IDR_STEP_1_KERNEL);


template <typename ValueType>
void step_2(std::shared_ptr<const OmpExecutor> exec, const size_type k,
            const matrix::Dense<ValueType> *omega,
            const matrix::Dense<ValueType> *preconditioned_vector,
            const matrix::Dense<ValueType> *c, matrix::Dense<ValueType> *u,
            const Array<stopping_status> *stop_status)
{
    const auto nrhs = omega->get_size()[1];
    for (size_type i = 0; i < nrhs; i++) {
        if (stop_status->get_const_data()[0].has_stopped()) {
            continue;
        }

#pragma omp parallel for
        for (size_type row = 0; row < u->get_size()[0]; row++) {
            auto temp = omega->at(0, i) * preconditioned_vector->at(row, i);
            for (size_type j = k; j < c->get_size()[0]; j++) {
                temp += c->at(j, i) * u->at(row, j * nrhs + i);
            }
            u->at(row, k * nrhs + i) = temp;
        }
    }
}

GKO_INSTANTIATE_FOR_EACH_VALUE_TYPE(GKO_DECLARE_IDR_STEP_2_KERNEL);


template <typename ValueType>
void step_3(std::shared_ptr<const OmpExecutor> exec, const size_type k,
            const matrix::Dense<ValueType> *p, matrix::Dense<ValueType> *g,
            matrix::Dense<ValueType> *u, matrix::Dense<ValueType> *m,
            matrix::Dense<ValueType> *f, matrix::Dense<ValueType> *residual,
            matrix::Dense<ValueType> *x,
            const Array<stopping_status> *stop_status)
{
    const auto nrhs = x->get_size()[1];

    for (size_type i = 0; i < nrhs; i++) {
        if (stop_status->get_const_data()[0].has_stopped()) {
            continue;
        }
    }

    update_g_and_u(k, p, m, g, u);

    for (size_type i = 0; i < nrhs; i++) {
#pragma omp parallel for
        for (size_type j = k; j < m->get_size()[0]; j++) {
            auto temp = zero<ValueType>();
            for (size_type ind = 0; ind < p->get_size()[1]; ind++) {
                temp += p->at(j, ind) * g->at(ind, k * nrhs + i);
            }
            m->at(j, k * nrhs + i) = temp;
        }

        auto beta = f->at(k, i) / m->at(k, k * nrhs + i);

#pragma omp parallel for
        for (size_type row = 0; row < g->get_size()[0]; row++) {
            residual->at(row, i) -= beta * g->at(row, k * nrhs + i);
            x->at(row, i) += beta * u->at(row, k * nrhs + i);
        }

        if (k + 1 < f->get_size()[0]) {
            f->at(k, i) = zero<ValueType>();
#pragma omp parallel for
            for (size_type j = k + 1; j < f->get_size()[0]; j++) {
                f->at(j, i) -= beta * m->at(j, k * nrhs + i);
            }
        }
    }
}

GKO_INSTANTIATE_FOR_EACH_VALUE_TYPE(GKO_DECLARE_IDR_STEP_3_KERNEL);


template <typename ValueType>
void compute_omega(
    std::shared_ptr<const OmpExecutor> exec,
    const remove_complex<ValueType> kappa, const matrix::Dense<ValueType> *tht,
    const matrix::Dense<remove_complex<ValueType>> *residual_norm,
    matrix::Dense<ValueType> *omega, const Array<stopping_status> *stop_status)
{
#pragma omp parallel for
    for (size_type i = 0; i < omega->get_size()[1]; i++) {
        if (stop_status->get_const_data()[0].has_stopped()) {
            continue;
        }

        auto thr = omega->at(0, i);
        auto normt = sqrt(real(tht->at(0, i)));
        omega->at(0, i) /= tht->at(0, i);
        auto absrho = abs(thr / (normt * residual_norm->at(0, i)));

        if (absrho < kappa) {
            omega->at(0, i) *= kappa / absrho;
        }
    }
}

GKO_INSTANTIATE_FOR_EACH_VALUE_TYPE(GKO_DECLARE_IDR_COMPUTE_OMEGA_KERNEL);


}  // namespace idr
}  // namespace omp
}  // namespace kernels
}  // namespace gko