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

#include "procedural/probing/flow/topology.hpp"
#include <vector>

namespace e8 {
namespace procedural {

// Flow value statistics.
struct FlowStatistics {
  explicit FlowStatistics(std::vector<float> const &flows);

  // Minimum flow value.
  float min;

  // Maximum flow value.
  float max;

  // Average flow value.
  float mean;

  // Flow value standard deviation.
  float std;
};

// Updates the current topology flow using the simulated flow. It applies
// exponential moving average on the current and simualted flows to prevent
// oscillation of the shortest paths.
FlowStatistics UpdateFlow(TopologyFlow const &simulated_flow,
                          TopologyFlow *current_flow);

} // namespace procedural
} // namespace e8
