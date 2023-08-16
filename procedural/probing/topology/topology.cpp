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

#include "procedural/probing/topology/topology.hpp"
#include "procedural/probing/probe/probe.hpp"
#include "procedural/probing/topology/definition.hpp"
#include "procedural/probing/topology/init.hpp"
#include "procedural/probing/topology/optimize_efficiency.hpp"
#include "procedural/probing/topology/optimize_regularity.hpp"
#include <algorithm>
#include <random>
#include <vector>

namespace e8 {
namespace procedural {
namespace {

unsigned long const kSeed = 13L;

std::vector<ProbeConnection> ToProbeConnection(Topology const &topology) {
  auto [edge_it, _] = boost::edges(topology);

  std::vector<ProbeConnection> result(boost::num_edges(topology));
  for (auto &result_edge : result) {
    result_edge = ProbeConnection(edge_it->m_source, edge_it->m_target);
    ++edge_it;
  }

  return result;
}

} // namespace

ProbeTopologyResult
ComputeProbeTopology(std::vector<PopulationProbe> const &probes,
                     unsigned regularity_optimization_steps,
                     unsigned efficiency_optimization_steps) {
  Topology initial_topology = CreateDelaunayTopology(probes);
  std::default_random_engine random_engine(kSeed);
  OptimizeRegularityResult regularized_result = OptimizeRegularity(
      initial_topology, regularity_optimization_steps, &random_engine);
  OptimizeEfficiencyResult optimization_result =
      OptimizeEfficiency(regularized_result.topology,
                         efficiency_optimization_steps, &random_engine);
  return ProbeTopologyResult{
      .connections = ToProbeConnection(optimization_result.topology),
      .score = optimization_result.score,
  };
}

} // namespace procedural
} // namespace e8
