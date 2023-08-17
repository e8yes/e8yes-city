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

#include "procedural/probing/flow/update.hpp"
#include "procedural/probing/flow/topology.hpp"
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <cassert>
#include <cmath>
#include <limits>
#include <vector>

namespace e8 {
namespace procedural {
namespace {

float const kSqrtFlowPerLane = 6.2f;
float const kFlowUpdateRate = 0.1f;

unsigned SuitableLaneCount(float flow) {
  return std::max(1.f, std::round(std::sqrt(flow) / kSqrtFlowPerLane));
}

float UpdatedFlow(float simulated_flow, float current_flow) {
  return kFlowUpdateRate * simulated_flow +
         (1 - kFlowUpdateRate) * current_flow;
}

} // namespace

FlowStatistics::FlowStatistics(std::vector<float> const &flows)
    : min(std::numeric_limits<float>::infinity()), max(0) {
  float sum = 0;
  for (auto flow : flows) {
    min = std::min(min, flow);
    max = std::max(max, flow);
    sum += flow;
  }
  mean = sum / flows.size();

  float sum_var = 0;
  for (auto flow : flows) {
    sum_var += (flow - mean) * (flow - mean);
  }
  std = std::sqrt(sum_var / flows.size());
}

FlowStatistics UpdateFlow(TopologyFlow const &simulated_flow,
                          TopologyFlow *current_flow) {
  assert(boost::num_vertices(simulated_flow) ==
         boost::num_vertices(*current_flow));
  assert(boost::num_edges(simulated_flow) == boost::num_edges(*current_flow));

  std::vector<float> edge_flow_values;
  edge_flow_values.reserve(boost::num_edges(*current_flow));

  for (auto [current, end] = boost::edges(*current_flow); current != end;
       ++current) {
    auto [simulated_edge, existence] =
        boost::edge(current->m_source, current->m_target, simulated_flow);
    assert(existence);
    float simulated_flow_value =
        boost::get(&FlowProperties::flow, simulated_flow, simulated_edge);
    FlowProperties current_properties =
        boost::get(boost::edge_bundle_t(), *current_flow, *current);

    current_properties.flow =
        UpdatedFlow(simulated_flow_value, current_properties.flow);
    current_properties.lane_count = SuitableLaneCount(current_properties.flow);
    boost::put(boost::edge_bundle_t(), *current_flow, *current,
               current_properties);

    edge_flow_values.push_back(current_properties.flow);
  }

  return FlowStatistics(edge_flow_values);
}

} // namespace procedural
} // namespace e8
