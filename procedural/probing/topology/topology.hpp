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
#include <vector>

namespace e8 {
namespace procedural {

// In the order of the specified probe array, this indicate two probes at the
// following indices should be connected bidirectionally.
struct ProbeConnection {
  ProbeConnection() = default;
  ProbeConnection(unsigned src_probe_index, unsigned dst_probe_index)
      : src_probe_index(src_probe_index), dst_probe_index(dst_probe_index) {}

  unsigned src_probe_index;
  unsigned dst_probe_index;
};

// Returns by ComputeProbeTopology().
struct ProbeTopologyResult {
  // The unique connections between the population probes.
  std::vector<ProbeConnection> connections;

  // The objective score of the connection. It estimates the average number of
  // residents transported at the probes.
  float score;
};

// It computes the connections amongst the specified population probes in a way
// that the transportation between any two probes is reasonably efficient.
ProbeTopologyResult
ComputeProbeTopology(std::vector<PopulationProbe> const &probes,
                     unsigned optimization_step_count,
                     bool optimize_efficiency);

} // namespace procedural
} // namespace e8
