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

//
using RegularityScoreMap = std::vector<RegularityScore>;

//
RegularityScore RegularityObjectiveAt(unsigned u, Topology const &topology);

//
RegularityScoreMap CreateRegularityScoreMapFor(Topology const &topology);

//
RegularityScore
EvaluateRegularityObjective(RegularityScoreMap const &score_map);

} // namespace procedural
} // namespace e8
