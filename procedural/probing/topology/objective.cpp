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

#include "procedural/probing/topology/objective.hpp"
#include "procedural/probing/topology/definition.hpp"
#include "procedural/probing/topology/sampler.hpp"
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <cassert>
#include <cmath>
#include <eigen3/Eigen/Core>
#include <numbers>
#include <random>
#include <vector>

namespace e8 {
namespace procedural {
namespace {

float const kPedestrianCrossingTimeSeconds = 10.0f;
float const kUTurnWaitTimeSeconds = 60.0f;
float const k3WayIntersectionWaitTimeSeconds = 50.0f;
float const k4WayIntersectionWaitTimeSeconds = 60.0f;
float const kIntersectionWaitTimeSecondsPerWay = 50.0f;

float const kMinSpeedMetersPerSecond = 8.33f;
float const kMaxSpeedMetersPerSecond = 38.9f;
constexpr float const kP50Population = 126.0f;
constexpr float const kP10Population = 30.6;
constexpr float const kLog9 = 2.197224577f;

constexpr float const kAcos04 = 1.159279481f;
constexpr float const kAcos10 = 0.0f;
constexpr float const kMaxTolerableTravelTimeSeconds = 3600.0f;

float EstimateAverageSpeed(float local_population) {
  constexpr float scale = kLog9 / (kP10Population - kP50Population);
  float interpolant =
      1.0f / (1.0 + std::exp(scale * (kP50Population - local_population)));
  return kMinSpeedMetersPerSecond +
         interpolant * (kMaxSpeedMetersPerSecond - kMinSpeedMetersPerSecond);
}

float EstimateAverageWaitTime(unsigned intersection_size) {
  switch (intersection_size) {
  case 0:
    return kUTurnWaitTimeSeconds;
  case 1:
    return kUTurnWaitTimeSeconds;
  case 2:
    return kPedestrianCrossingTimeSeconds;
  case 3:
    return k3WayIntersectionWaitTimeSeconds;
  case 4:
    return k4WayIntersectionWaitTimeSeconds;
  default:
    return kIntersectionWaitTimeSecondsPerWay * intersection_size;
  }
}

float EstimateLikelihoodToTravel(float time_cost) {
  assert(time_cost >= 0.0);

  if (time_cost > kMaxTolerableTravelTimeSeconds) {
    return 0;
  }

  constexpr float phi = -kAcos10;
  constexpr float omega =
      (std::numbers::pi - phi) / kMaxTolerableTravelTimeSeconds;
  return 0.5 * (1 + std::cos(omega * time_cost + phi));
}

CostMap CreateCostMapWithConnections(Topology const &topology) {
  assert(boost::num_vertices(topology) > 0);
  CostMap result(boost::num_vertices(topology));

  auto [current, end] = boost::edges(topology);
  for (; current != end; ++current) {
    assert(!boost::edge(current->m_source, current->m_target, result).second);
    boost::add_edge(current->m_source, current->m_target, result);
  }

  return result;
}

float PopulationTrasnportedFromSource(unsigned source_index,
                                      std::vector<float> const &target_costs,
                                      Topology const &topology) {
  assert(target_costs.size() == boost::num_vertices(topology));

  float proportion_transported = 0.0f;
  for (unsigned i = 0; i < target_costs.size(); ++i) {
    proportion_transported +=
        EstimateLikelihoodToTravel(target_costs[i]) * topology[i].importance;
  }
  return proportion_transported * topology[source_index].local_population;
}

} // namespace

float EstimateTravelTimeCost(unsigned u, unsigned v, Topology const &topology) {
  assert(u < boost::num_vertices(topology));
  assert(v < boost::num_vertices(topology));

  float speed_u = EstimateAverageSpeed(topology[u].local_population);
  float speed_v = EstimateAverageSpeed(topology[v].local_population);
  float distance = (topology[u].location - topology[v].location).norm();
  return 0.5 * (distance / speed_u + distance / speed_v);
}

float EstimateWaitTimeCost(unsigned u, unsigned v, CostMap const &cost_map) {
  assert(u < boost::num_vertices(cost_map));
  assert(v < boost::num_vertices(cost_map));

  return 0.5 * (EstimateAverageWaitTime(boost::degree(u, cost_map)) +
                EstimateAverageWaitTime(boost::degree(v, cost_map)));
}

float TotalTimeCost(float travel_time_cost, float wait_time_cost) {
  return travel_time_cost + wait_time_cost;
}

CostMap CreateCostMapForTopology(Topology const &topology) {
  CostMap result = CreateCostMapWithConnections(topology);

  auto [current, end] = boost::edges(result);
  for (; current != end; ++current) {
    assert(current->m_source < boost::num_vertices(topology));
    assert(current->m_target < boost::num_vertices(topology));

    auto [topology_edge, existence] =
        boost::edge(current->m_source, current->m_target, topology);
    assert(existence);

    float travel_time_cost =
        EstimateTravelTimeCost(current->m_source, current->m_target, topology);
    float wait_time_cost =
        EstimateWaitTimeCost(current->m_source, current->m_target, result);
    boost::put(boost::edge_weight_t(), result, *current,
               TotalTimeCost(travel_time_cost, wait_time_cost));
  }

  return result;
}

float EvaluateObjective(Topology const &topology, CostMap const &cost_map,
                        SourceSamplerInterface const &source_sampler) {
  assert(boost::num_vertices(topology) == boost::num_vertices(cost_map));
  assert(boost::num_vertices(cost_map) > 0);

  float transported = 0.0f;
  std::vector<float> min_time_costs(boost::num_vertices(cost_map));
  for (auto const &sample : source_sampler.SourceSamples()) {
    assert(sample.source_index < boost::num_vertices(cost_map));
    CostMap::vertex_descriptor source =
        boost::vertex(sample.source_index, cost_map);
    boost::dijkstra_shortest_paths(cost_map, source,
                                   boost::distance_map(&min_time_costs[0]));

    transported += sample.frequency * sample.correction *
                   PopulationTrasnportedFromSource(sample.source_index,
                                                   min_time_costs, topology);
  }

  return transported / source_sampler.SampleCount();
}

} // namespace procedural
} // namespace e8
