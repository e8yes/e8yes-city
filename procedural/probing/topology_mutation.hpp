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

#include "procedural/probing/topology_definition.hpp"
#include <functional>
#include <random>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace e8 {
namespace procedural {

// Since the mutation operations apply solely on the edge set, it's useful to
// define the components that define an edge.
using EdgeSrcIndex = unsigned;
using EdgeDstIndex = unsigned;
using EdgeCostValue = float;
using Edge = std::tuple<EdgeSrcIndex, EdgeDstIndex>;

struct EdgeHash {
  auto operator()(Edge const &edge) const -> size_t {
    return std::hash<EdgeSrcIndex>{}(std::get<0>(edge)) ^
           std::hash<EdgeDstIndex>{}(std::get<1>(edge));
  }
};

// A mutation is the set of edges to be added to/deleted from the current edge
// set.
struct Mutation {
  // Should be constructed by EdgeSetState::Mutate().
  Mutation(unsigned num_additions, unsigned num_deletions) {
    additions.reserve(num_additions);
    deletions.reserve(num_deletions);
  }

  // The set of edges to be added.
  std::vector<Edge> additions;

  // The set of edges to be deleted.
  std::vector<Edge> deletions;
};

// Saves the current edge state, so the later mutation made to the edge set can
// be reverted.
struct EdgeRecovery {
  // Edges whose cost value will be affected by the mutation.
  std::unordered_map<Edge, EdgeCostValue, EdgeHash> affected_edges;

  // Edges which will be deleted by the mutation. The order of the values
  // corresponds to that of Mutation::deletions.
  std::vector<EdgeCostValue> deleted_edge_values;
};

// Creates an edge recovery for the mutation. It assumes the mutation is
// generated based on the state of the specified cost map. Please see the above
// EdgeRecovery struct for what states are saved.
EdgeRecovery CreateEdgeRecoveryFor(Mutation const &mutation,
                                   CostMap const &cost_map);

// Actuates the mutation onto the specified cost map, assuming the mutation and
// edge recovery is generated based on the state of the cost map.
void ApplyMutation(Mutation const &mutation, EdgeRecovery const &recovery,
                   Topology const &topology, CostMap *cost_map);

// Reverts the mutation previously applied to the cost map.
void RevertMutation(Mutation const &mutation, EdgeRecovery const &recovery,
                    CostMap *cost_map);

} // namespace procedural
} // namespace e8
