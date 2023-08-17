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

#include "procedural/probing/flow/time_cost.hpp"
#include "procedural/probing/flow/topology.hpp"
#include "procedural/probing/probe/probe.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <cassert>
#include <cmath>
#include <eigen3/Eigen/Core>
#include <vector>

namespace e8 {
namespace procedural {
namespace {

float const kMinSpeedMetersPerSecond = 8.33f;
float const kMaxSpeedMetersPerSecond = 38.9f;
constexpr float const kP50Flow = 122.f;
constexpr float const kP10Flow = 68.3f;
constexpr float const kLog9 = 2.197224577f;
float const kFlowPerSecond = 1.f;
float const kFlowFraction = .04f;

float EstimateSpeed(float flow) {
  constexpr float scale = kLog9 / (kP10Flow - kP50Flow);
  float interpolant = 1.0f / (1.0 + std::exp(scale * (kP50Flow - flow)));

  return kMinSpeedMetersPerSecond +
         interpolant * (kMaxSpeedMetersPerSecond - kMinSpeedMetersPerSecond);
}

float TravelTimeCost(unsigned u, unsigned v, TopologyFlow const &flow,
                     std::vector<PopulationProbe> const &probes) {
  assert(u != v);
  auto [edge, existence] = boost::edge(u, v, flow);
  assert(existence);

  FlowProperties property = boost::get(boost::edge_bundle_t(), flow, edge);
  float speed = EstimateSpeed(property.LaneFlow());

  assert(probes[v].location != probes[u].location);
  float distance = (probes[v].location - probes[u].location).norm();

  return distance / speed;
}

float WaitTimeCost(unsigned v, TopologyFlow const &flow) {
  if (boost::in_degree(v, flow) < 3) {
    return 0;
  }

  float total_flow = 0.0f;
  for (auto [current, end] = boost::in_edges(v, flow); current != end;
       ++current) {
    total_flow += boost::get(&FlowProperties::flow, flow, *current);
  }
  return total_flow * kFlowFraction / kFlowPerSecond;
}

} // namespace

FlowCostMap CreateFlowCostMapFrom(TopologyFlow const &flow,
                                  std::vector<PopulationProbe> const &probes) {
  assert(boost::num_vertices(flow) == probes.size());

  FlowCostMap flow_cost(boost::num_vertices(flow));
  for (auto [current, end] = boost::edges(flow); current != end; ++current) {
    float travel_time_cost =
        TravelTimeCost(current->m_source, current->m_target, flow, probes);
    float wait_time_cost = WaitTimeCost(current->m_target, flow);

    auto [edge, successful] =
        boost::add_edge(current->m_source, current->m_target,
                        travel_time_cost + wait_time_cost, flow_cost);
    assert(successful);
  }

  return flow_cost;
}

} // namespace procedural
} // namespace e8
