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

#include "procedural/probing/topology/definition.hpp"
#include <random>

namespace e8 {
namespace procedural {

// Returns by OptimizeRegularity().
struct OptimizeRegularityResult {
  // The optimized topology.
  Topology topology;

  // The objective score of the topology.
  float score;
};

// It performs combinatorial optimization over the regularity objective on the
// specified topology by hill climbing.
OptimizeRegularityResult
OptimizeRegularity(Topology const &topology, unsigned iteration_count,
                   std::default_random_engine *random_engine);

} // namespace procedural
} // namespace e8
