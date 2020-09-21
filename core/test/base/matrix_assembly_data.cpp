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

#include <ginkgo/core/base/matrix_assembly_data.hpp>


#include <gtest/gtest.h>


namespace {


TEST(MatrixAssemblyData, InitializesANullMatrix)
{
    gko::matrix_assembly_data<double, int> m;

    ASSERT_EQ(m.size, gko::dim<2>(0, 0));
    ASSERT_EQ(m.nonzeros.size(), 0);
}


TEST(MatrixAssemblyData, InitializesWithZeros)
{
    gko::matrix_assembly_data<double, int> m(gko::dim<2>{3, 5});

    ASSERT_EQ(m.size, gko::dim<2>(3, 5));
    ASSERT_EQ(m.nonzeros.size(), 0);
}


TEST(MatrixAssemblyData, InsertsValuesWithoutAdding)
{
    gko::matrix_assembly_data<double, int> m(gko::dim<2>{3, 5});

    m.insert_value(0, 0, 1.3);
    m.insert_value(2, 3, 2.2);
    m.insert_value(1, 4, 1.1);
    m.insert_value(1, 2, 3.6);

    ASSERT_EQ(m.size, gko::dim<2>(3, 5));
    ASSERT_EQ(m.nonzeros.size(), 4);
    ASSERT_EQ(m.nonzeros.at({0, 0}), 1.3);
    ASSERT_EQ(m.nonzeros.at({2, 3}), 2.2);
    ASSERT_EQ(m.nonzeros.at({1, 4}), 1.1);
    ASSERT_EQ(m.nonzeros.at({1, 2}), 3.6);
}


TEST(MatrixAssemblyData, InsertsValuesWithAdding)
{
    gko::matrix_assembly_data<double, int> m(gko::dim<2>{3, 5});

    m.insert_value(0, 0, 1.3, true);
    m.insert_value(2, 3, 2.2, true);
    m.insert_value(1, 4, 1.1, true);
    m.insert_value(1, 2, 3.6, true);
    m.insert_value(1, 4, 9.1, true);
    m.insert_value(2, 3, 1.3, true);

    ASSERT_EQ(m.size, gko::dim<2>(3, 5));
    ASSERT_EQ(m.nonzeros.size(), 4);
    ASSERT_EQ(m.nonzeros.at({0, 0}), 1.3);
    ASSERT_EQ(m.nonzeros.at({2, 3}), 3.5);
    ASSERT_EQ(m.nonzeros.at({1, 4}), 10.2);
    ASSERT_EQ(m.nonzeros.at({1, 2}), 3.6);
}


}  // namespace
