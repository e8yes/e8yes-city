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
#include <vector>

namespace e8 {
namespace procedural {

// Represents the regularity score at a vertex (population probe).
using RegularityScore = float;

// For storing the objective score at each vertex.
using RegularityScoreMap = std::vector<RegularityScore>;

// Computes the objective at the specified vertex in the topology. The objective
// score is can be thought as the distance from the standard street patterns. In
// particular, Zero/one way intersections are undesirable. In a two-way
// intersection, the two streets are expected to run in a straight line. In a
// three-way intersection, the three streets are expected to form a T shape. In
// a four-way intersection, the four streets are expected to form a + shape.
// Intersections with greater number of ways are more undesirable than those of
// zero/one way.
RegularityScore RegularityObjectiveAt(unsigned u, Topology const &topology);

// Creates a score map from the specified topology.
RegularityScoreMap CreateRegularityScoreMapFor(Topology const &topology);

// Computes the total objective score for the entire score map. It basically
// sums the objective scores over the vertices.
RegularityScore
EvaluateRegularityObjective(RegularityScoreMap const &score_map);

} // namespace procedural
} // namespace e8
