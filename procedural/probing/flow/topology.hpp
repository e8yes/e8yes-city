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

#pragma once

#include <boost/graph/adjacency_list.hpp>

namespace e8 {
namespace procedural {

// Specifies what to measure between 2 connected probes.
struct FlowProperties {
  FlowProperties();
  FlowProperties(float flow, unsigned lane_count);

  float LaneFlow() const;

  float flow;
  unsigned lane_count;
};

// For measuring the flows over the topology.
using TopologyFlow = boost::adjacency_list<
    /*OutEdgeListS=*/boost::vecS, /*VertexListS=*/boost::vecS,
    /*DirectedS=*/boost::bidirectionalS, /*VertexProperty=*/boost::no_property,
    /*EdgeProperty=*/FlowProperties>;

namespace testing {

// Creates a flow topology of the following structure:
// 0 1 2
//   |
// 3-4-5
//   |
// 6 7 8
TopologyFlow CreateGridTopologyFlow(float flow_value);

} // namespace testing
} // namespace procedural
} // namespace e8
