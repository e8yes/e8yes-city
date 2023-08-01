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
#include "procedural/probing/topology/objective_efficiency.hpp"
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace e8 {
namespace procedural {

using EdgeEfficiencyCostValue = float;

// Extends the mutation to support the saving of the current edge costs, so the
// later mutation made to the edge set can be reverted.
struct RevertibleEfficiencyMutation {
  // It assumes the mutation is generated based on the state of the specified
  // cost map.
  RevertibleEfficiencyMutation(Mutation &&other,
                               EfficiencyCostMap const &cost_map);

  // The mutation to prepare recovery for.
  Mutation mutation;

  // Edges whose cost value will be affected by the mutation.
  std::unordered_map<Edge, EdgeEfficiencyCostValue, EdgeHash> affected_edges;

  // Edges which will be deleted by the mutation.
  std::unordered_map<Edge, EdgeEfficiencyCostValue, EdgeHash> deleted_edges;
};

// Actuates the mutation onto the specified cost map, assuming the mutation is
// generated based on the state of the cost map.
void ApplyMutation(RevertibleEfficiencyMutation const &revertible,
                   Topology const &topology, EfficiencyCostMap *cost_map);

// Reverts the mutation previously applied to the cost map.
void RevertMutation(RevertibleEfficiencyMutation const &revertible,
                    EfficiencyCostMap *cost_map);

} // namespace procedural
} // namespace e8
