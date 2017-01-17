/******************************************************************************
** Copyright (c) 2015, Intel Corporation                                     **
** All rights reserved.                                                      **
**                                                                           **
** Redistribution and use in source and binary forms, with or without        **
** modification, are permitted provided that the following conditions        **
** are met:                                                                  **
** 1. Redistributions of source code must retain the above copyright         **
**    notice, this list of conditions and the following disclaimer.          **
** 2. Redistributions in binary form must reproduce the above copyright      **
**    notice, this list of conditions and the following disclaimer in the    **
**    documentation and/or other materials provided with the distribution.   **
** 3. Neither the name of the copyright holder nor the names of its          **
**    contributors may be used to endorse or promote products derived        **
**    from this software without specific prior written permission.          **
**                                                                           **
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       **
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         **
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR     **
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT      **
** HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    **
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED  **
** TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR    **
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    **
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      **
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        **
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* ******************************************************************************/
/* Narayanan Sundaram (Intel Corp.), Michael Anderson (Intel Corp.)
 * ******************************************************************************/

#include <iostream>
#include "catch.hpp"
#include "generator.hpp"
#include <algorithm>
#include "test_utils.hpp"

TEST_CASE("vector", "vector")
{
  SECTION(" SpVec basic tests", "SpVec basic tests") {
      int tiles_per_dim;
      int (*partition_fn)(int,int,int,int,int);
      GMDP::get_fn_and_tiles(1, GMDP::get_global_nrank(), &partition_fn, &tiles_per_dim);
      GMDP::SpVec<GMDP::DenseSegment<int> > myvec(1000, tiles_per_dim, GMDP::vector_partition_fn);
      REQUIRE(myvec.getNNZ() == 0);

      myvec.set(1, 1);
      myvec.set(10, 1);
      myvec.set(200, 1);
      myvec.set(300, 1);

      REQUIRE(myvec.getNNZ() == 4);

      myvec.setAll(2);
      REQUIRE(myvec.getNNZ() == 1000);

      GMDP::Clear(&myvec);
      REQUIRE(myvec.getNNZ() == 0);
  }

  SECTION(" DenseSegment basic tests", "DenseSegment basic tests") {
      GMDP::DenseSegment<int> v1(1000);
      v1.set(1, 1);
      v1.set(10, 1);
      v1.set(200, 1);
      v1.set(300, 1);
      REQUIRE(v1.compute_nnz() == 4);
   }

  SECTION("DenseSegment send/recv tests", "DenseSegment send/recv tests") {
      if(GMDP::get_global_nrank() % 2 == 0)
      {
        GMDP::DenseSegment<int> v1(1000);
        std::vector<MPI_Request> requests;

        if(GMDP::get_global_myrank() % 2 == 1)
        {
          REQUIRE(v1.compute_nnz() == 0);
          v1.recv_nnz(GMDP::get_global_myrank(),
                      GMDP::get_global_myrank() - 1,
                      &requests);
          v1.recv_segment(GMDP::get_global_myrank(),
                          GMDP::get_global_myrank() - 1,
                          &requests);
        }
        else
        {
          v1.set(1, 1);
          v1.set(10, 1);
          v1.set(200, 1);
          v1.set(300, 1);
          REQUIRE(v1.compute_nnz() == 4);
          v1.compress();
          v1.send_nnz(GMDP::get_global_myrank(),
                      GMDP::get_global_myrank() + 1,
                      &requests);
          v1.send_segment(GMDP::get_global_myrank(),
                          GMDP::get_global_myrank() + 1,
                          &requests);

        }

        MPI_Waitall(requests.size(), requests.data(), MPI_STATUS_IGNORE);
        requests.clear();
        v1.decompress();
        REQUIRE(v1.compute_nnz() == 4);
      }
   }
}

