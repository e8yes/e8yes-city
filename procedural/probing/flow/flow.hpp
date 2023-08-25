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

#include "procedural/probing/probe/probe.hpp"
#include "procedural/probing/topology/topology.hpp"
#include <vector>

namespace e8 {
namespace procedural {

// Represents the measured flow on a directed connection.
struct ProbeConnectionFlow {
  ProbeConnectionFlow() = default;
  ProbeConnectionFlow(unsigned src_probe_index, unsigned dst_probe_index,
                      float flow, unsigned lane_count)
      : src_probe_index(src_probe_index), dst_probe_index(dst_probe_index),
        flow(flow), lane_count(lane_count) {}

  // Indices to the probe array specifying a directed connection.
  unsigned src_probe_index;
  unsigned dst_probe_index;

  // The total number of people transported through the connection.
  float flow;

  // The suitable number of lanes for the flow.
  unsigned lane_count;
};

// It estimates the directed transportation flow over the connections. It uses
// an iterative algorithm to simulate and measure the flow. The estimate
// converges as more iterations are run.
std::vector<ProbeConnectionFlow>
EstimateProbeTopologyFlow(std::vector<PopulationProbe> const &probes,
                          std::vector<ProbeConnection> const &connections,
                          unsigned iteration_count);

} // namespace procedural
} // namespace e8
