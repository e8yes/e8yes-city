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

#include "procedural/probing/topology/mutation_regularity.hpp"
#include "procedural/probing/topology/definition.hpp"
#include "procedural/probing/topology/mutation.hpp"
#include "procedural/probing/topology/objective_regularity.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <cassert>
#include <tuple>
#include <unordered_map>

namespace e8 {
namespace procedural {
namespace {

void AddAffectedVertices(
    std::unordered_set<Edge, EdgeHash> const &edges,
    RegularityScoreMap const &score_map,
    std::unordered_map<Vertex, RegularityScore> *affected_vertices) {
  for (auto const &edge : edges) {
    auto [src, dst] = edge;
    affected_vertices->insert(std::make_pair(src, score_map[src]));
    affected_vertices->insert(std::make_pair(dst, score_map[dst]));
  }
}

} // namespace

RevertibleRegularityMutation::RevertibleRegularityMutation(
    Mutation &&other, RegularityScoreMap const &score_map, float score)
    : mutation(std::move(other)), score(score) {
  affected_vertices.reserve(
      2 * (mutation.additions.size() + mutation.deletions.size()));
  AddAffectedVertices(other.additions, score_map, &affected_vertices);
  AddAffectedVertices(other.deletions, score_map, &affected_vertices);
}

float ApplyMutation(RevertibleRegularityMutation const &revertible,
                    Topology *topology, RegularityScoreMap *score_map) {
  for (auto addition : revertible.mutation.additions) {
    auto [u, v] = addition;

    assert(u < boost::num_vertices(*topology));
    assert(v < boost::num_vertices(*topology));
    boost::add_edge(u, v, *topology);
  }

  for (auto deletion : revertible.mutation.deletions) {
    auto [u, v] = deletion;

    assert(u < boost::num_vertices(*topology));
    assert(v < boost::num_vertices(*topology));
    boost::remove_edge(u, v, *topology);
  }

  RegularityScore score_diff = 0;
  for (auto const &[vertex, _] : revertible.affected_vertices) {
    assert(vertex < boost::num_vertices(*topology));

    float old_score = (*score_map)[vertex];
    float new_score = RegularityObjectiveAt(vertex, *topology);
    score_diff += new_score - old_score;

    (*score_map)[vertex] = new_score;
  }

  return revertible.score + score_diff;
}

float RevertMutation(RevertibleRegularityMutation const &revertible,
                     Topology *topology, RegularityScoreMap *score_map) {
  for (auto addition : revertible.mutation.additions) {
    auto [u, v] = addition;

    assert(u < boost::num_vertices(*topology));
    assert(v < boost::num_vertices(*topology));
    boost::remove_edge(u, v, *topology);
  }

  for (auto deletion : revertible.mutation.deletions) {
    auto [u, v] = deletion;

    assert(u < boost::num_vertices(*topology));
    assert(v < boost::num_vertices(*topology));
    auto [_, existence] = boost::add_edge(u, v, *topology);
    assert(!existence);
  }

  for (auto const &[vertex, old_score] : revertible.affected_vertices) {
    assert(vertex < boost::num_vertices(*topology));
    (*score_map)[vertex] = old_score;
  }

  return revertible.score;
}

} // namespace procedural
} // namespace e8
