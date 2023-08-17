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

#include "procedural/probing/flow/simulate.hpp"
#include "procedural/probing/flow/time_cost.hpp"
#include "procedural/probing/flow/topology.hpp"
#include "procedural/probing/probe/probe.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <cassert>
#include <numbers>
#include <vector>

namespace e8 {
namespace procedural {
namespace {

constexpr float const kAcos04 = 1.159279481f;
constexpr float const kMaxTolerableTravelTimeSeconds = 3600.0f;

TopologyFlow CreateNewFlowsFrom(TopologyFlow const &previous_flow) {
  TopologyFlow new_flows(boost::num_vertices(previous_flow));

  for (auto [current, end] = boost::edges(previous_flow); current != end;
       ++current) {
    unsigned previous_lane_count =
        boost::get(&FlowProperties::lane_count, previous_flow, *current);
    FlowProperties new_properties(/*flow=*/0,
                                  /*lane_count=*/previous_lane_count);
    auto [edge, successful] =
        boost::add_edge(current->m_source, current->m_target, new_flows);
    assert(successful);
  }

  return new_flows;
}

struct SimulatedPaths {
  SimulatedPaths(unsigned vertex_count,
                 FlowCostMap::vertex_descriptor const &source)
      : time_cost(vertex_count), source(source), predecessors(vertex_count) {}

  std::vector<float> time_cost;
  FlowCostMap::vertex_descriptor source;
  std::vector<FlowCostMap::vertex_descriptor> predecessors;
};

SimulatedPaths SimulatePathsFrom(unsigned probe_index,
                                 FlowCostMap const &flow_cost) {
  FlowCostMap::vertex_descriptor source = boost::vertex(probe_index, flow_cost);
  SimulatedPaths result(boost::num_vertices(flow_cost), source);
  boost::dijkstra_shortest_paths(flow_cost, source,
                                 boost::distance_map(&result.time_cost[0])
                                     .predecessor_map(&result.predecessors[0]));
  return result;
}

float LikelihoodToTravel(float time_cost) {
  assert(time_cost >= 0.0);

  if (time_cost > kMaxTolerableTravelTimeSeconds) {
    return 0;
  }

  constexpr float phi = -kAcos04;
  constexpr float omega =
      (std::numbers::pi - phi) / kMaxTolerableTravelTimeSeconds;
  return 0.5 * (1 + std::cos(omega * time_cost + phi));
}

std::vector<float>
ComputeTravelProbability(SimulatedPaths const &paths,
                         std::vector<PopulationProbe> const &probes) {
  assert(paths.time_cost.size() == probes.size());

  // Calculates the likelihood for a person to travel to each destination probe.
  std::vector<float> p_travel(probes.size());
  float evidence = 0;
  for (unsigned i = 0; i < probes.size(); ++i) {
    float p_travel_to_i =
        LikelihoodToTravel(paths.time_cost[i]) * probes[i].population_grid_200;
    p_travel[i] = p_travel_to_i;
    evidence += p_travel_to_i;
  }

  // Normalizes to posterior distribution.
  for (unsigned i = 0; i < probes.size(); ++i) {
    p_travel[i] /= evidence;
  }

  return p_travel;
}

std::vector<float>
ComputeTravelPopulation(unsigned source_probe_index,
                        SimulatedPaths const &paths,
                        std::vector<PopulationProbe> const &probes) {
  std::vector<float> p_travel = ComputeTravelProbability(paths, probes);

  std::vector<float> result(probes.size());
  for (unsigned i = 0; i < probes.size(); ++i) {
    result[i] = p_travel[i] * probes[source_probe_index].population_grid_200;
  }
  return result;
}

void AccumulateFlowsToPath(
    unsigned path_destination,
    std::vector<FlowCostMap::vertex_descriptor> const &predecessors,
    float to_be_added, TopologyFlow *topology_flow) {
  FlowCostMap::vertex_descriptor current = path_destination;
  FlowCostMap::vertex_descriptor parent = predecessors[path_destination];

  while (parent != current) {
    auto [edge, existence] = boost::edge(parent, current, *topology_flow);
    assert(existence);

    float current_flow =
        boost::get(&FlowProperties::flow, *topology_flow, edge);
    boost::put(&FlowProperties::flow, *topology_flow, edge,
               current_flow + to_be_added);

    current = parent;
    parent = predecessors[current];
  }
}

void AccumulateFlows(std::vector<float> const &travel_population,
                     SimulatedPaths const &paths, TopologyFlow *topology_flow) {
  for (unsigned destination = 0; destination < travel_population.size();
       ++destination) {
    AccumulateFlowsToPath(destination, paths.predecessors,
                          travel_population[destination], topology_flow);
  }
}

} // namespace

TopologyFlow SimulateFlow(TopologyFlow const &previous_flow,
                          std::vector<PopulationProbe> const &probes) {
  TopologyFlow current_flows = CreateNewFlowsFrom(previous_flow);

  FlowCostMap flow_cost = CreateFlowCostMapFrom(previous_flow, probes);
  for (unsigned source = 0; source < boost::num_vertices(previous_flow);
       ++source) {
    SimulatedPaths simulated_paths = SimulatePathsFrom(source, flow_cost);
    std::vector<float> travel_population =
        ComputeTravelPopulation(source, simulated_paths, probes);
    AccumulateFlows(travel_population, simulated_paths, &current_flows);
  }

  return current_flows;
}

} // namespace procedural
} // namespace e8
