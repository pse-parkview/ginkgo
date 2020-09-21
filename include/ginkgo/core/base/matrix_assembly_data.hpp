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

#ifndef GKO_CORE_BASE_MATRIX_ASSEMBLY_DATA_HPP_
#define GKO_CORE_BASE_MATRIX_ASSEMBLY_DATA_HPP_


#include <algorithm>
#include <numeric>
#include <tuple>
#include <unordered_map>


#include <ginkgo/core/base/dim.hpp>
#include <ginkgo/core/base/math.hpp>
#include <ginkgo/core/base/types.hpp>
#include <ginkgo/core/base/utils.hpp>


namespace gko {


namespace detail {


/**
 * Type used to store indices used as keys in the unordered_map storing
 * the nonzeros.
 */
template <typename IndexType = int32>
struct symbolic_nonzero {
    using index_type = IndexType;
    symbolic_nonzero() = default;

    symbolic_nonzero(index_type r, index_type c) : row(r), column(c) {}

#define GKO_DEFINE_DEFAULT_COMPARE_OPERATOR(_op)           \
    bool operator _op(const symbolic_nonzero &other) const \
    {                                                      \
        return std::tie(this->row, this->column)           \
            _op std::tie(other.row, other.column);         \
    }

    GKO_DEFINE_DEFAULT_COMPARE_OPERATOR(==);
    GKO_DEFINE_DEFAULT_COMPARE_OPERATOR(!=);
    GKO_DEFINE_DEFAULT_COMPARE_OPERATOR(<);
    GKO_DEFINE_DEFAULT_COMPARE_OPERATOR(>);
    GKO_DEFINE_DEFAULT_COMPARE_OPERATOR(<=);
    GKO_DEFINE_DEFAULT_COMPARE_OPERATOR(>=);

#undef GKO_DEFINE_DEFAULT_COMPARE_OPERATOR

    index_type row;
    index_type column;
};


template <typename IndexType>
struct symbolic_nonzero_hash {
    symbolic_nonzero_hash(const size_type num_cols_) : num_cols{num_cols_} {}

    std::size_t operator()(const symbolic_nonzero<IndexType> &nnz)
    {
        return nnz.row * num_cols + nnz.column;
    }

    const size_type num_cols;
};


}  // namespace detail


/**
 * This structure is used as an intermediate data type to store a sparse matrix.
 *
 * The matrix is stored as a sequence of nonzero elements, where each element is
 * a triple of the form (row_index, column_index, value).
 *
 * @note All Ginkgo functions returning such a structure will return the
 *       nonzeros sorted in row-major order.
 * @note All Ginkgo functions that take this structure as input expect that the
 *       nonzeros are sorted in row-major order.
 * @note This structure is not optimized for usual access patterns and it can
 *       only exist on the CPU. Thus, it should only be used for utility
 *       functions which do not have to be optimized for performance.
 *
 * @tparam ValueType  type of matrix values stored in the structure
 * @tparam IndexType  type of matrix indexes stored in the structure
 */
template <typename ValueType = default_precision, typename IndexType = int32>
struct matrix_assembly_data {
    using value_type = ValueType;
    using index_type = IndexType;

    matrix_assembly_data(const dim<2> size_ = dim<2>{})
        : size{size_},
          nonzeros{0, detail::symbolic_nonzero_hash<index_type>(size[1])}
    {}

    void insert_value(index_type row, index_type col, value_type val,
                      bool add = false)
    {
        auto ind = detail::symbolic_nonzero<index_type>(row, col);
        auto inserted = nonzeros.insert({ind, val});

        if (add && !inserted.second) {
            nonzeros.at(ind) += val;
        }
    }

    /**
     * Size of the matrix.
     */
    const dim<2> size;

    /**
     * An unordered map storing the non-zeros of the matrix.
     *
     * The keys of the elements in the map are the row index and the column
     * index of a matrix element, and its value is the value at that
     * position.
     */
    std::unordered_map<
        detail::symbolic_nonzero<index_type>, value_type,
        std::function<size_type(detail::symbolic_nonzero<index_type>)>>
        nonzeros;
};


}  // namespace gko


#endif  // GKO_CORE_BASE_MATRIX_ASSEMBLY_DATA_HPP_
