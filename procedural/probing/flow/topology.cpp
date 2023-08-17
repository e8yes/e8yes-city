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

#include "procedural/probing/flow/topology.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <cassert>

namespace e8 {
namespace procedural {

FlowProperties::FlowProperties() : flow(0), lane_count(1) {}

FlowProperties::FlowProperties(float flow, unsigned lane_count)
    : flow(flow), lane_count(lane_count) {}

float FlowProperties::LaneFlow() const {
  assert(lane_count > 0);
  return flow / lane_count;
}

namespace testing {

TopologyFlow CreateGridTopologyFlow(float flow_value) {
  TopologyFlow flows(9);

  boost::add_edge(1, 4, FlowProperties(flow_value, /*lane_count=*/1), flows);
  boost::add_edge(4, 1, FlowProperties(flow_value, /*lane_count=*/1), flows);

  boost::add_edge(3, 4, FlowProperties(flow_value, /*lane_count=*/1), flows);
  boost::add_edge(4, 3, FlowProperties(flow_value, /*lane_count=*/1), flows);

  boost::add_edge(4, 5, FlowProperties(flow_value, /*lane_count=*/1), flows);
  boost::add_edge(5, 4, FlowProperties(flow_value, /*lane_count=*/1), flows);

  boost::add_edge(4, 7, FlowProperties(flow_value, /*lane_count=*/1), flows);
  boost::add_edge(7, 4, FlowProperties(flow_value, /*lane_count=*/1), flows);

  return flows;
}

} // namespace testing
} // namespace procedural
} // namespace e8
