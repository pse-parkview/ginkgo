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

#ifndef GKO_PUBLIC_CORE_MATRIX_BATCH_CSR_HPP_
#define GKO_PUBLIC_CORE_MATRIX_BATCH_CSR_HPP_


#include <ginkgo/core/base/array.hpp>
#include <ginkgo/core/base/lin_op.hpp>
#include <ginkgo/core/base/math.hpp>
#include <ginkgo/core/matrix/csr.hpp>


namespace gko {
namespace matrix {


template <typename ValueType>
class BatchDense;

template <typename ValueType, typename IndexType>
class BatchCsr;


/**
 * BatchCsr is a matrix format which stores only the nonzero coefficients by
 * compressing each row of the matrix (compressed sparse row format).
 *
 * The nonzero elements are stored in a 1D array row-wise, and accompanied
 * with a row pointer array which stores the starting index of each row.
 * An additional column index array is used to identify the column of each
 * nonzero element.
 *
 * The BatchCsr LinOp supports different operations:
 *
 * ```cpp
 * matrix::BatchCsr *A, *B, *C;      // matrices
 * matrix::Dense *b, *x;        // vectors tall-and-skinny matrices
 * matrix::Dense *alpha, *beta; // scalars of dimension 1x1
 * matrix::Identity *I;         // identity matrix
 *
 * // Applying to Dense matrices computes an SpMV/SpMM product
 * A->apply(b, x)              // x = A*b
 * A->apply(alpha, b, beta, x) // x = alpha*A*b + beta*x
 *
 * // Applying to BatchCsr matrices computes a SpGEMM product of two sparse
 * matrices A->apply(B, C)              // C = A*B A->apply(alpha, B, beta, C)
 * // C = alpha*A*B + beta*C
 *
 * // Applying to an Identity matrix computes a SpGEAM sparse matrix addition
 * A->apply(alpha, I, beta, B) // B = alpha*A + beta*B
 * ```
 * Both the SpGEMM and SpGEAM operation require the input matrices to be sorted
 * by column index, otherwise the algorithms will produce incorrect results.
 *
 * @tparam ValueType  precision of matrix elements
 * @tparam IndexType  precision of matrix indexes
 *
 * @ingroup batch_csr
 * @ingroup mat_formats
 * @ingroup LinOp
 */
template <typename ValueType = default_precision, typename IndexType = int32>
class BatchCsr
    : public EnableLinOp<BatchCsr<ValueType, IndexType>>,
      public EnableCreateMethod<BatchCsr<ValueType, IndexType>>,
      public ConvertibleTo<BatchCsr<next_precision<ValueType>, IndexType>>,
      public BatchReadableFromMatrixData<ValueType, IndexType>,
      public BatchWritableToMatrixData<ValueType, IndexType>,
      public Transposable {
    friend class EnableCreateMethod<BatchCsr>;
    friend class EnablePolymorphicObject<BatchCsr, LinOp>;
    friend class BatchCsr<to_complex<ValueType>, IndexType>;

public:
    using BatchReadableFromMatrixData<ValueType, IndexType>::read;

    using value_type = ValueType;
    using index_type = IndexType;
    using transposed_type = BatchCsr<ValueType, IndexType>;
    using unbatch_type = Csr<ValueType, IndexType>;
    using mat_data = matrix_data<ValueType, IndexType>;
    using absolute_type = remove_complex<BatchCsr>;

    void convert_to(BatchCsr<ValueType, IndexType> *result) const override
    {
        bool same_executor = this->get_executor() == result->get_executor();
        result->values_ = this->values_;
        result->col_idxs_ = this->col_idxs_;
        result->row_ptrs_ = this->row_ptrs_;
        result->set_batch_sizes(this->get_batch_sizes());
    }

    void move_to(BatchCsr<ValueType, IndexType> *result) override
    {
        bool same_executor = this->get_executor() == result->get_executor();
        EnableLinOp<BatchCsr>::move_to(result);
    }
    friend class BatchCsr<next_precision<ValueType>, IndexType>;

    void convert_to(
        BatchCsr<next_precision<ValueType>, IndexType> *result) const override;

    void move_to(
        BatchCsr<next_precision<ValueType>, IndexType> *result) override;

    void read(const std::vector<mat_data> &data) override;

    void write(std::vector<mat_data> &data) const override;

    std::unique_ptr<LinOp> transpose() const override;

    std::unique_ptr<LinOp> conj_transpose() const override;

    std::vector<std::unique_ptr<unbatch_type>> unbatch() const
    {
        auto exec = this->get_executor();
        auto unbatch_mats = std::vector<std::unique_ptr<unbatch_type>>{};
        size_type num_nnz =
            this->get_num_stored_elements() / this->get_num_batches();
        size_type offset = 0;
        for (size_type b = 0; b < this->get_num_batches(); ++b) {
            auto mat =
                unbatch_type::create(exec, this->get_batch_sizes()[b], num_nnz);
            exec->copy_from(exec.get(), num_nnz,
                            this->get_const_values() + offset,
                            mat->get_values());
            exec->copy_from(exec.get(), num_nnz, this->get_const_col_idxs(),
                            mat->get_col_idxs());
            exec->copy_from(exec.get(), this->get_batch_sizes()[b][0] + 1,
                            this->get_const_row_ptrs(), mat->get_row_ptrs());
            unbatch_mats.emplace_back(std::move(mat));
            offset += num_nnz;
        }
        return unbatch_mats;
    }

    /**
     * Sorts all (value, col_idx) pairs in each row by column index
     */
    void sort_by_column_index();

    /*
     * Tests if all row entry pairs (value, col_idx) are sorted by column index
     *
     * @returns True if all row entry pairs (value, col_idx) are sorted by
     *          column index
     */
    bool is_sorted_by_column_index() const;

    /**
     * Returns the values of the matrix.
     *
     * @return the values of the matrix.
     */
    value_type *get_values() noexcept { return values_.get_data(); }

    /**
     * @copydoc BatchCsr::get_values()
     *
     * @note This is the constant version of the function, which can be
     *       significantly more memory efficient than the non-constant version,
     *       so always prefer this version.
     */
    const value_type *get_const_values() const noexcept
    {
        return values_.get_const_data();
    }

    /**
     * Returns the column indexes of the matrix.
     *
     * @return the column indexes of the matrix.
     */
    index_type *get_col_idxs() noexcept { return col_idxs_.get_data(); }

    /**
     * @copydoc BatchCsr::get_col_idxs()
     *
     * @note This is the constant version of the function, which can be
     *       significantly more memory efficient than the non-constant version,
     *       so always prefer this version.
     */
    const index_type *get_const_col_idxs() const noexcept
    {
        return col_idxs_.get_const_data();
    }

    /**
     * Returns the row pointers of the matrix.
     *
     * @return the row pointers of the matrix.
     */
    index_type *get_row_ptrs() noexcept { return row_ptrs_.get_data(); }

    /**
     * @copydoc BatchCsr::get_row_ptrs()
     *
     * @note This is the constant version of the function, which can be
     *       significantly more memory efficient than the non-constant version,
     *       so always prefer this version.
     */
    const index_type *get_const_row_ptrs() const noexcept
    {
        return row_ptrs_.get_const_data();
    }

    /**
     * Returns the number of elements explicitly stored in the matrix.
     *
     * @return the number of elements explicitly stored in the matrix
     */
    size_type get_num_stored_elements() const noexcept
    {
        return values_.get_num_elems();
    }

    size_type get_num_batches() const { return batch_sizes_.size(); }

    std::vector<dim<2>> get_batch_sizes() const { return batch_sizes_; }

    void set_batch_sizes(const std::vector<dim<2>> sizes)
    {
        batch_sizes_ = sizes;
    }

private:
    inline dim<2> compute_cumulative_size(const std::vector<gko::dim<2>> &sizes)
    {
        auto cumul_size = dim<2>{};
        for (auto i = 0; i < sizes.size(); ++i) {
            cumul_size[0] += (sizes[i])[0];
            cumul_size[1] += (sizes[i])[1];
        }
        return cumul_size;
    }

protected:
    /**
     * Creates an uninitialized BatchCsr matrix of the specified size.
     *
     * @param exec  Executor associated to the matrix
     * @param size  size of the matrix
     * @param num_nonzeros  number of nonzeros
     * @param strategy  the strategy of BatchCsr
     */
    BatchCsr(std::shared_ptr<const Executor> exec,
             const size_type num_batches = {}, const dim<2> &size = dim<2>{},
             size_type num_nonzeros = {})
        : EnableLinOp<BatchCsr>(
              exec,
              compute_cumulative_size(std::vector<dim<2>>(num_batches, size))),
          batch_sizes_(std::vector<dim<2>>(num_batches, size)),
          values_(exec, num_nonzeros * num_batches),
          col_idxs_(exec, num_nonzeros),
          row_ptrs_(exec, (size[0]) + 1)
    {}

    /**
     * Creates a BatchCsr matrix from already allocated (and initialized) row
     * pointer, column index and value arrays.
     *
     * @tparam ValuesArray  type of `values` array
     * @tparam ColIdxsArray  type of `col_idxs` array
     * @tparam RowPtrsArray  type of `row_ptrs` array
     *
     * @param exec  Executor associated to the matrix
     * @param size  size of the matrix
     * @param values  array of matrix values
     * @param col_idxs  array of column indexes
     * @param row_ptrs  array of row pointers
     *
     * @note If one of `row_ptrs`, `col_idxs` or `values` is not an rvalue, not
     *       an array of IndexType, IndexType and ValueType, respectively, or
     *       is on the wrong executor, an internal copy of that array will be
     *       created, and the original array data will not be used in the
     *       matrix.
     */
    template <typename ValuesArray, typename ColIdxsArray,
              typename RowPtrsArray>
    BatchCsr(std::shared_ptr<const Executor> exec, const size_type num_batches,
             const dim<2> &size, ValuesArray &&values, ColIdxsArray &&col_idxs,
             RowPtrsArray &&row_ptrs)
        : EnableLinOp<BatchCsr>(
              exec,
              compute_cumulative_size(std::vector<dim<2>>(num_batches, size))),
          batch_sizes_(std::vector<dim<2>>(num_batches, size)),
          values_{exec, std::forward<ValuesArray>(values)},
          col_idxs_{exec, std::forward<ColIdxsArray>(col_idxs)},
          row_ptrs_{exec, std::forward<RowPtrsArray>(row_ptrs)}
    {
        GKO_ASSERT_EQ(values_.get_num_elems(),
                      col_idxs_.get_num_elems() * num_batches);
        GKO_ASSERT_EQ(this->get_batch_sizes()[0][0] + 1,
                      row_ptrs_.get_num_elems());
    }

    virtual void validate_application_parameters(const LinOp *b,
                                                 const LinOp *x) const override
    {
        auto batch_this = as<BatchCsr<ValueType, IndexType>>(this);
        auto batch_x = as<BatchDense<ValueType>>(x);
        auto batch_b = as<BatchDense<ValueType>>(b);
        GKO_ASSERT_CONFORMANT(batch_this, batch_b);
        GKO_ASSERT_EQUAL_ROWS(batch_this, batch_x);
        GKO_ASSERT_EQUAL_COLS(batch_b, batch_x);
        GKO_ASSERT_BATCH_CONFORMANT(batch_this, batch_b);
        GKO_ASSERT_BATCH_EQUAL_ROWS(batch_this, batch_x);
        GKO_ASSERT_BATCH_EQUAL_COLS(batch_b, batch_x);
    }

    virtual void validate_application_parameters(const LinOp *alpha,
                                                 const LinOp *b,
                                                 const LinOp *beta,
                                                 const LinOp *x) const override
    {
        this->validate_application_parameters(b, x);
        GKO_ASSERT_BATCH_EQUAL_DIMENSIONS(
            as<BatchDense<ValueType>>(alpha),
            std::vector<dim<2>>(get_num_batches(), dim<2>(1, 1)));
        GKO_ASSERT_BATCH_EQUAL_DIMENSIONS(
            as<BatchDense<ValueType>>(beta),
            std::vector<dim<2>>(get_num_batches(), dim<2>(1, 1)));
    }

    void apply_impl(const LinOp *b, LinOp *x) const override;

    void apply_impl(const LinOp *alpha, const LinOp *b, const LinOp *beta,
                    LinOp *x) const override;

private:
    std::vector<dim<2>> batch_sizes_;
    Array<value_type> values_;
    Array<index_type> col_idxs_;
    Array<index_type> row_ptrs_;
};


}  // namespace matrix
}  // namespace gko


#endif  // GKO_PUBLIC_CORE_MATRIX_BATCH_CSR_HPP_