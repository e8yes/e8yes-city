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

#include "procedural/probing/topology/edge_set.hpp"
#include "procedural/probing/topology/definition.hpp"
#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <random>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

namespace e8 {
namespace procedural {
namespace {

void AddEdge(unsigned edge_to_add, std::vector<Edge> *edges,
             unsigned *separator, internal::MutationLog *log) {
  assert(edge_to_add >= *separator);
  assert(*separator < edges->size());

  std::swap(edges->at(edge_to_add), edges->at(*separator));
  log->swaps.push_back(std::make_pair(edge_to_add, *separator));
  ++(*separator);
}

void DeleteEdge(unsigned edge_to_delete, std::vector<Edge> *edges,
                unsigned *separator, internal::MutationLog *log) {
  assert(edge_to_delete < *separator);
  assert(*separator <= edges->size());

  std::swap(edges->at(edge_to_delete), edges->at(*separator - 1));
  log->swaps.push_back(std::make_pair(edge_to_delete, *separator - 1));
  --(*separator);
}

bool ChooseAddEdgeOperation(std::vector<Edge> const &edges, unsigned separator,
                            float prob_add,
                            std::default_random_engine *random_engine) {
  if (separator == edges.size()) {
    // There is no deleted edge to add. At this state, only delete-edge
    // operation is possible.
    return false;
  }

  if (0 == separator) {
    // There is no active edge to delete. At this state, only add-edge
    // operation is possible.
    return true;
  }

  float p = std::uniform_real_distribution<float>(.0f, 1.f)(*random_engine);
  return p < prob_add;
}

unsigned SampleActiveEdge(std::vector<Edge> const &edges, unsigned separator,
                          std::default_random_engine *random_engine) {
  assert(separator > 0 && separator <= edges.size());
  return std::uniform_int_distribution<unsigned>(0,
                                                 separator - 1)(*random_engine);
}

unsigned SampleDeletedEdge(std::vector<Edge> const &edges, unsigned separator,
                           std::default_random_engine *random_engine) {
  assert(separator < edges.size());
  return std::uniform_int_distribution<unsigned>(separator, edges.size() - 1)(
      *random_engine);
}

} // namespace

namespace internal {

MutationLog::MutationLog()
    : separator_before(std::numeric_limits<unsigned>::max()) {}

MutationLog::MutationLog(unsigned pending_operation_count,
                         unsigned current_separator)
    : separator_before(current_separator) {
  swaps.reserve(pending_operation_count);
}

} // namespace internal

Mutation::Mutation(unsigned num_additions, unsigned num_deletions) {
  additions.reserve(num_additions);
  deletions.reserve(num_deletions);
}

void Mutation::PushAddition(Edge const &edge) {
  if (deletions.erase(edge) > 0) {
    return;
  }
  additions.insert(edge);
}

void Mutation::PushDeletion(Edge const &edge) {
  if (additions.erase(edge) > 0) {
    return;
  }
  deletions.insert(edge);
}

EdgeSetState::EdgeSetState(std::default_random_engine *random_engine)
    : separator_(0), random_engine_(random_engine) {}

void EdgeSetState::Add(Edge const &edge) {
  edges_.push_back(edge);
  std::swap(edges_[separator_], edges_.back());
  ++separator_;
}

Mutation EdgeSetState::Mutate(unsigned operation_count, float prob_add) {
  Mutation result(/*num_additions=*/operation_count,
                  /*num_deletions=*/operation_count);

  log_ = internal::MutationLog(operation_count, separator_);
  for (unsigned i = 0; i < operation_count; ++i) {
    if (ChooseAddEdgeOperation(edges_, separator_, prob_add, random_engine_)) {
      unsigned edge_to_add =
          SampleDeletedEdge(edges_, separator_, random_engine_);
      result.PushAddition(edges_[edge_to_add]);
      AddEdge(edge_to_add, &edges_, &separator_, &log_);
    } else {
      unsigned edge_to_delete =
          SampleActiveEdge(edges_, separator_, random_engine_);
      result.PushDeletion(edges_[edge_to_delete]);
      DeleteEdge(edge_to_delete, &edges_, &separator_, &log_);
    }
  }

  return result;
}

void EdgeSetState::Revert() {
  if (log_.swaps.empty()) {
    return;
  }

  while (!log_.swaps.empty()) {
    auto const &[edge_index0, edge_index1] = log_.swaps.back();
    std::swap(edges_[edge_index0], edges_[edge_index1]);
    log_.swaps.pop_back();
  }

  separator_ = log_.separator_before;
}

std::vector<Edge> EdgeSetState::ActiveEdges() const {
  std::vector<Edge> result(separator_);
  std::copy(edges_.begin(), edges_.begin() + separator_, result.begin());
  return result;
}

std::vector<Edge> EdgeSetState::DeletedEdges() const {
  std::vector<Edge> result(edges_.size() - separator_);
  std::copy(edges_.begin() + separator_, edges_.end(), result.begin());
  return result;
}

EdgeSetState CreateEdgeSetStateFor(Topology const &topology,
                                   std::default_random_engine *random_engine) {
  EdgeSetState edge_set(random_engine);
  auto [current, end] = boost::edges(topology);
  for (; current != end; ++current) {
    edge_set.Add(Edge(current->m_source, current->m_target));
  }
  return edge_set;
}

} // namespace procedural
} // namespace e8
