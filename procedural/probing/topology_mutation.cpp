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

#include "procedural/probing/topology_mutation.hpp"
#include "procedural/probing/topology_definition.hpp"
#include "procedural/probing/topology_objective.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <cassert>
#include <functional>
#include <random>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace e8 {
namespace procedural {
namespace {

Edge Reverse(Edge const &edge) {
  return std::make_tuple(std::get<1>(edge), std::get<0>(edge));
}

void AddAffectedEdge(
    Edge const &affected_edge, CostMap const &cost_map,
    std::unordered_map<Edge, EdgeCostValue, EdgeHash> *affected_edges) {
  auto existing_it = affected_edges->find(affected_edge);
  if (existing_it == affected_edges->end()) {
    // Try the same edge, but written in the opposite order.
    existing_it = affected_edges->find(Reverse(affected_edge));
  }

  if (existing_it != affected_edges->end()) {
    // The edge has already been added.
    return;
  }

  auto [edge_desc, existence] = boost::edge(
      std::get<0>(affected_edge), std::get<1>(affected_edge), cost_map);
  assert(existence);
  float cost_value = boost::get(boost::edge_weight_t(), cost_map, edge_desc);
  affected_edges->insert(std::make_pair(affected_edge, cost_value));
}

void EdgesAffectedBy(
    unsigned vertex, CostMap const &cost_map,
    std::unordered_map<Edge, EdgeCostValue, EdgeHash> *affected_edges) {
  auto [current, end] = boost::adjacent_vertices(vertex, cost_map);
  for (; current != end; ++current) {
    Edge affected_edge = std::make_tuple(vertex, *current);
    AddAffectedEdge(affected_edge, cost_map, affected_edges);
  }
}

void EdgesAffectedBy(
    std::vector<Edge> const &mutation, CostMap const &cost_map,
    std::unordered_map<Edge, EdgeCostValue, EdgeHash> *affected_edges) {
  for (auto const &edge : mutation) {
    EdgesAffectedBy(std::get<0>(edge), cost_map, affected_edges);
    EdgesAffectedBy(std::get<1>(edge), cost_map, affected_edges);
  }
}

void UpdateCostFor(Edge const &edge, Topology const &topology,
                   CostMap *cost_map) {
  auto [topology_edge, topology_existence] =
      boost::edge(std::get<0>(edge), std::get<1>(edge), topology);
  assert(topology_existence);
  float travel_time_cost =
      boost::get(boost::edge_weight_t(), topology, topology_edge);
  float wait_time_cost =
      EstimateWaitTimeCost(std::get<0>(edge), std::get<1>(edge), *cost_map);
  float total_time_cost = TotalTimeCost(travel_time_cost, wait_time_cost);

  auto [cost_edge, cost_existence] =
      boost::edge(std::get<0>(edge), std::get<1>(edge), *cost_map);
  assert(cost_existence);
  boost::put(boost::edge_weight_t(), *cost_map, cost_edge, total_time_cost);
}

} // namespace

EdgeRecovery CreateEdgeRecoveryFor(Mutation const &mutation,
                                   CostMap const &cost_map) {
  EdgeRecovery recovery;

  // Saves the cost value for edges that are going to be deleted.
  recovery.deleted_edge_values.reserve(mutation.deletions.size());
  for (auto const &edge : mutation.deletions) {
    auto [edge_desc, existence] =
        boost::edge(std::get<0>(edge), std::get<1>(edge), cost_map);
    assert(existence);

    float cost_value = boost::get(boost::edge_weight_t(), cost_map, edge_desc);
    recovery.deleted_edge_values.push_back(cost_value);
  }

  // Saves the cost value for edges that are going to be affected by the
  // edge additions and deletions.
  EdgesAffectedBy(mutation.additions, cost_map, &recovery.affected_edges);
  EdgesAffectedBy(mutation.deletions, cost_map, &recovery.affected_edges);

  return recovery;
}

void ApplyMutation(Mutation const &mutation, EdgeRecovery const &recovery,
                   Topology const &topology, CostMap *cost_map) {
  // Updates the connections.
  for (auto const &edge_to_add : mutation.additions) {
    boost::add_edge(std::get<0>(edge_to_add), std::get<1>(edge_to_add),
                    *cost_map);
  }
  for (auto const &edge_to_delete : mutation.deletions) {
    boost::remove_edge(std::get<0>(edge_to_delete), std::get<1>(edge_to_delete),
                       *cost_map);
  }

  // Updates the cost values.
  for (auto const &edge : mutation.additions) {
    UpdateCostFor(edge, topology, cost_map);
  }
  for (auto const &[edge, _] : recovery.affected_edges) {
    UpdateCostFor(edge, topology, cost_map);
  }
}

void RevertMutation(Mutation const &mutation, EdgeRecovery const &recovery,
                    CostMap *cost_map) {
  // Removes added edges.
  for (auto const &added_edge : mutation.additions) {
    boost::remove_edge(std::get<0>(added_edge), std::get<1>(added_edge),
                       *cost_map);
  }

  // Recovers deleted edges.
  assert(mutation.deletions.size() == recovery.deleted_edge_values.size());
  for (unsigned i = 0; i < mutation.deletions.size(); ++i) {
    boost::add_edge(std::get<0>(mutation.deletions[i]),
                    std::get<1>(mutation.deletions[i]),
                    recovery.deleted_edge_values[i], *cost_map);
  }

  // Recovers affected edges.
  for (auto const &[edge, edge_cost] : recovery.affected_edges) {
    auto [edge_desc, existence] =
        boost::edge(std::get<0>(edge), std::get<1>(edge), *cost_map);
    assert(existence);
    boost::put(boost::edge_weight_t(), *cost_map, edge_desc, edge_cost);
  }
}

} // namespace procedural
} // namespace e8
