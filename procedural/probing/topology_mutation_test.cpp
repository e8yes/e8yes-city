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
#include "procedural/probing/topology_definition.hpp"
#include "procedural/probing/topology_mutation.hpp"
#include "procedural/probing/topology_objective.hpp"
#include <boost/test/unit_test.hpp>
#include <random>

namespace e8 {
namespace procedural {
namespace {

Topology CreateGridTopology(unsigned side, float scale, float population) {
  Topology topology(side * side);

  for (unsigned x = 0; x < side; ++x) {
    for (unsigned y = 0; y < side; ++y) {
      float importance =
          static_cast<float>(2 * side - x - y) / ((side + 1) * side * side);

      topology[x + y * side] = VertexProperties(
          /*location=*/Eigen::Vector3f(scale * x, scale * y, 0.0),
          /*local_population=*/importance * population, /*area_population=*/0,
          /*importance=*/importance);
    }
  }

  for (unsigned x = 1; x < side - 1; ++x) {
    for (unsigned y = 1; y < side - 1; ++y) {
      Topology::vertex_descriptor u = boost::vertex(x + y * side, topology);
      Topology::vertex_descriptor v0 =
          boost::vertex(x - 1 + y * side, topology);
      Topology::vertex_descriptor v1 =
          boost::vertex(x + (y + 1) * side, topology);
      Topology::vertex_descriptor v2 =
          boost::vertex(x + 1 + y * side, topology);
      Topology::vertex_descriptor v3 =
          boost::vertex(x + (y - 1) * side, topology);

      if (!boost::edge(u, v0, topology).second) {
        boost::add_edge(u, v0, topology);
      }
      if (!boost::edge(u, v1, topology).second) {
        boost::add_edge(u, v1, topology);
      }
      if (!boost::edge(u, v2, topology).second) {
        boost::add_edge(u, v2, topology);
      }
      if (!boost::edge(u, v3, topology).second) {
        boost::add_edge(u, v3, topology);
      }
    }
  }

  return topology;
}

BOOST_AUTO_TEST_CASE(WhenAtOriginalState_ThenCheckActiveAndDeletedEdges) {
  Topology topology =
      CreateGridTopology(/*side=*/5, /*scale=*/1e3f, /*population=*/4e3);
  CostMap cost_map = CreateCostMapForTopology(topology);

  std::default_random_engine random_engine;
  EdgeSetState edge_set_state(cost_map, &random_engine);

  std::vector<Edge> active_edges = edge_set_state.ActiveEdges();
  std::vector<Edge> deleted_edges = edge_set_state.DeletedEdges();

  auto [current, end] = boost::adjacent_vertices(6, cost_map);
  for (; current != end; ++current) {
    std::cout << *current << std::endl;
  }

  BOOST_CHECK_EQUAL(active_edges.size(), 24);
  BOOST_CHECK_EQUAL(deleted_edges.size(), 0);
}

} // namespace
} // namespace procedural
} // namespace e8
