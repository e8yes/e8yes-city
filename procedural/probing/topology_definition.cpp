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

#include "procedural/probing/topology_definition.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <eigen3/Eigen/Core>

namespace e8 {
namespace procedural {
namespace testing {

Topology CreateGridTopology(unsigned side, float scale, float population) {
  Topology topology(side * side);

  for (unsigned x = 0; x < side; ++x) {
    for (unsigned y = 0; y < side; ++y) {
      float importance =
          static_cast<float>(2 * side - x - y) / ((side + 1) * side * side);

      topology[x + y * side] = VertexProperties(
          /*location=*/Eigen::Vector3f(scale * x, scale * y, 0.0),
          /*local_population=*/importance * population,
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

} // namespace testing
} // namespace procedural
} // namespace e8
