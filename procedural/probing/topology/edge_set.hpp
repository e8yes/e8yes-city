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
#include "procedural/probing/topology/mutation.hpp"
#include <random>
#include <unordered_set>
#include <utility>
#include <vector>

namespace e8 {
namespace procedural {
namespace internal {

// Used by the class EdgeSetState.
struct MutationLog {
  MutationLog();
  MutationLog(unsigned pending_operation_count, unsigned current_separator);

  std::vector<std::pair<unsigned, unsigned>> swaps;
  unsigned separator_before;
};

} // namespace internal

// Keeps track of the state of each edge in the edge set. The state of an edge
// can either be active or deleted.
class EdgeSetState {
public:
  EdgeSetState(std::default_random_engine *random_engine);
  ~EdgeSetState() = default;

  // Adds an active edge to the set. The client of this call must guarantee
  // its uniqueness.
  void Add(Edge const &edge);

  // Obtains a mutation by performing random operations. A random operation can
  // either turn a deleted edge into an active one or vice versa. It's possible
  // that it eventually yields an empty mutation through the process. The
  // mutation is applied to the actual edge states.
  Mutation Mutate(unsigned operation_count, float prob_add = 0.5f);

  // Reverts the application of the last mutation performed by
  // EdgeSetState::Mutate(). Note, it can't revert more than 1 mutation. Namely,
  // subsequent calls to this function does nothing.
  void Revert();

  // For testing purposes.
  std::vector<Edge> ActiveEdges() const;
  std::vector<Edge> DeletedEdges() const;

private:
  std::vector<Edge> edges_;
  unsigned separator_;
  internal::MutationLog log_;
  std::default_random_engine *const random_engine_;
};

// Copies the edge set of the cost map to the EdgeSetState object and sets the
// state of the edges to active.
EdgeSetState CreateEdgeSetStateFor(CostMap const &cost_map,
                                   std::default_random_engine *random_engine);

} // namespace procedural
} // namespace e8
