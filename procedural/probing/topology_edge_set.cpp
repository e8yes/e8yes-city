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

#include "procedural/probing/topology_edge_set.hpp"
#include "procedural/probing/topology_definition.hpp"
#include "procedural/probing/topology_mutation.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <unordered_set>
#include <vector>

namespace e8 {
namespace procedural {
namespace {

float const kProbAdd = 0.5f;

void AddEdge(unsigned edge_index, internal::PendingMutation *mutation) {
  auto deletion_it = mutation->deletions.find(edge_index);
  if (deletion_it != mutation->deletions.end()) {
    mutation->deletions.erase(deletion_it);
    return;
  }

  mutation->additions.insert(edge_index);
}

void DeleteEdge(unsigned edge_index, internal::PendingMutation *mutation) {
  auto addition_it = mutation->additions.find(edge_index);
  if (addition_it != mutation->additions.end()) {
    mutation->additions.erase(addition_it);
    return;
  }

  mutation->deletions.insert(edge_index);
}

void AddEdge(unsigned edge_to_add, std::vector<Edge> *edges,
             unsigned *separator) {
  std::cout << "AddEdge(): " << edge_to_add << "," << *separator << std::endl;
  assert(edge_to_add >= *separator);
  assert(*separator < edges->size());

  std::swap(edges->at(edge_to_add), edges->at(*separator));
  ++(*separator);
}

void DeleteEdge(unsigned edge_to_delete, std::vector<Edge> *edges,
                unsigned *separator) {
  std::cout << "DeleteEdge(): " << edge_to_delete << "," << *separator
            << std::endl;
  assert(edge_to_delete < *separator);
  assert(*separator <= edges->size());

  std::swap(edges->at(edge_to_delete), edges->at(*separator - 1));
  --(*separator);
}

void ClearInternalMutation(internal::PendingMutation *mutation) {
  mutation->additions.clear();
  mutation->deletions.clear();
}

Mutation ToExternalMutation(internal::PendingMutation const &current_mutation,
                            std::vector<Edge> const &edges) {
  Mutation result(current_mutation.additions.size(),
                  current_mutation.deletions.size());
  for (auto const edge_index : current_mutation.additions) {
    result.additions.push_back(edges[edge_index]);
  }
  for (auto const edge_index : current_mutation.deletions) {
    result.deletions.push_back(edges[edge_index]);
  }
  return result;
}

bool ChooseAddEdgeOperation(std::vector<Edge> const &edges, unsigned separator,
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
  return p < kProbAdd;
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

EdgeSetState::EdgeSetState(std::default_random_engine *random_engine)
    : separator_(0), random_engine_(random_engine) {}

void EdgeSetState::Add(Edge const &edge) {
  edges_.push_back(edge);
  std::swap(edges_[separator_], edges_.back());
  ++separator_;
}

Mutation EdgeSetState::Mutate(unsigned operation_count) {
  ClearInternalMutation(&current_mutation_);

  for (unsigned i = 0; i < operation_count; ++i) {
    if (ChooseAddEdgeOperation(edges_, separator_, random_engine_)) {
      unsigned edge_to_add =
          SampleDeletedEdge(edges_, separator_, random_engine_);
      AddEdge(edge_to_add, &current_mutation_);
      AddEdge(edge_to_add, &edges_, &separator_);
    } else {
      unsigned edge_to_delete =
          SampleActiveEdge(edges_, separator_, random_engine_);
      DeleteEdge(edge_to_delete, &current_mutation_);
      DeleteEdge(edge_to_delete, &edges_, &separator_);
    }
  }

  return ToExternalMutation(current_mutation_, edges_);
}

void EdgeSetState::Revert() {
  for (auto const edge_index : current_mutation_.additions) {
    DeleteEdge(edge_index, &edges_, &separator_);
  }
  for (auto const edge_index : current_mutation_.deletions) {
    AddEdge(edge_index, &edges_, &separator_);
  }

  ClearInternalMutation(&current_mutation_);
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

EdgeSetState CreateEdgeSetStateFor(CostMap const &cost_map,
                                   std::default_random_engine *random_engine) {
  EdgeSetState edge_set(random_engine);
  auto [current, end] = boost::edges(cost_map);
  for (; current != end; ++current) {
    edge_set.Add(std::make_tuple(current->m_source, current->m_target));
  }
  return edge_set;
}

} // namespace procedural
} // namespace e8
