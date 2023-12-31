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
#include "procedural/probing/topology/edge_set.hpp"
#include "procedural/probing/topology/objective_regularity.hpp"
#include <unordered_map>

namespace e8 {
namespace procedural {

// Regularity mutations affects vertex level objective. Definong the index to a
// topology vertex as vertex provides reasonable abstraction.
using Vertex = unsigned;

// Extends the mutation to support the saving of the current vertex score, so
// the later mutation made to the edge set can be reverted.
struct RevertibleRegularityMutation {
  RevertibleRegularityMutation(Mutation &&other,
                               RegularityScoreMap const &score_map,
                               RegularityScore score);

  // The mutation to prepare recovery for.
  Mutation mutation;

  // The set of vertices and their current objective values that are going to be
  // affected by the mutation.
  std::unordered_map<Vertex, RegularityScore> affected_vertices;

  // The current total objective score before applying the mutation.
  RegularityScore score;
};

// Actuates the mutation onto the topology and the regularity score map then
// returns the updated objective score. It assumes the mutation is derived from
// the current state of the topology.
RegularityScore ApplyMutation(RevertibleRegularityMutation const &revertible,
                              Topology *topology,
                              RegularityScoreMap *score_map);

// Reverts the mutation previously applied to the topology and the regularity
// cost map then returns the previous objective score.
RegularityScore RevertMutation(RevertibleRegularityMutation const &revertible,
                               Topology *topology,
                               RegularityScoreMap *score_map);

} // namespace procedural
} // namespace e8
