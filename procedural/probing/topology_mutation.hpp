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
namespace internal {

// Used by the class EdgeSetState.
struct PendingMutation {
  std::unordered_set<unsigned> additions;
  std::unordered_set<unsigned> deletions;
};

} // namespace internal

//
using EdgeSrcIndex = unsigned;
using EdgeDstIndex = unsigned;
using EdgeCostValue = float;

//
using Edge = std::tuple<EdgeSrcIndex, EdgeDstIndex>;

//
struct EdgeHash {
  auto operator()(Edge const &edge) const -> size_t {
    return std::hash<EdgeSrcIndex>{}(std::get<0>(edge)) ^
           std::hash<EdgeDstIndex>{}(std::get<1>(edge));
  }
};

//
struct Mutation {
  // Should be constructed by EdgeSetState::Mutate().
  Mutation(unsigned num_additions, unsigned num_deletions) {
    additions.reserve(num_additions);
    deletions.reserve(num_deletions);
  }

  //
  std::vector<Edge> additions;

  //
  std::vector<Edge> deletions;
};

//
class EdgeSetState {
public:
  EdgeSetState(CostMap const &cost_map,
               std::default_random_engine *random_engine);
  ~EdgeSetState() = default;

  //
  Mutation Mutate(unsigned operation_count);

  //
  void Revert();

  //
  std::vector<Edge> ActiveEdges() const;
  std::vector<Edge> DeletedEdges() const;

private:
  std::vector<Edge> edges_;
  unsigned separator_;
  internal::PendingMutation current_mutation_;
  std::default_random_engine *const random_engine_;
};

//
struct EdgeRecovery {
  //
  std::unordered_map<Edge, EdgeCostValue, EdgeHash> affected_edges;

  //
  std::vector<EdgeCostValue> deleted_edge_values;
};

//
EdgeRecovery CreateEdgeRecoveryFor(Mutation const &mutation,
                                   CostMap const &cost_map);

//
void ApplyMutation(Mutation const &mutation, EdgeRecovery const &recovery,
                   Topology const &topology, CostMap *cost_map);

//
void RevertMutation(Mutation const &mutation, EdgeRecovery const &recovery,
                    CostMap *cost_map);

} // namespace procedural
} // namespace e8
