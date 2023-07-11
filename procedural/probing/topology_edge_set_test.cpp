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

#define BOOST_TEST_MAIN
#include "procedural/probing/topology_definition.hpp"
#include "procedural/probing/topology_edge_set.hpp"
#include "procedural/probing/topology_objective.hpp"
#include <boost/test/unit_test.hpp>
#include <random>

namespace e8 {
namespace procedural {
namespace {

BOOST_AUTO_TEST_CASE(WhenAtOriginalState_ThenCheckActiveAndDeletedEdges) {
  Topology topology = testing::CreateGridTopology(/*side=*/5, /*scale=*/1e3f,
                                                  /*population=*/4e3);
  CostMap cost_map = CreateCostMapForTopology(topology);

  std::default_random_engine random_engine;
  EdgeSetState edge_set_state = CreateEdgeSetStateFor(cost_map, &random_engine);

  std::vector<Edge> active_edges = edge_set_state.ActiveEdges();
  std::vector<Edge> deleted_edges = edge_set_state.DeletedEdges();

  BOOST_CHECK_EQUAL(active_edges.size(), 24);
  BOOST_CHECK_EQUAL(deleted_edges.size(), 0);
}

} // namespace
} // namespace procedural
} // namespace e8