// e8City
// Copyright (C) 2023 e8yes
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#define BOOST_TEST_MAIN
#include "procedural/probing/topology/definition.hpp"
#include "procedural/probing/topology/optimize_efficiency.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/test/unit_test.hpp>

namespace e8 {
namespace procedural {
namespace {

BOOST_AUTO_TEST_CASE(WhenTopologyIsMeshGrid_ThenCheckEdgeCountIsLess) {
  Topology topology = testing::CreateMeshTopology(/*side=*/5, /*scale=*/1e3f,
                                                  /*population=*/4e3);
  std::default_random_engine random_engine(13);
  OptimizeEfficiencyResult result =
      OptimizeEfficiency(topology, /*iteration_count=*/10000, &random_engine);
  BOOST_CHECK_CLOSE(147, result.score, 10);
  BOOST_CHECK_LT(boost::num_edges(result.topology), boost::num_edges(topology));
}

} // namespace
} // namespace procedural
} // namespace e8
