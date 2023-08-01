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

#define BOOST_TEST_MAIN
#include "procedural/probing/topology/definition.hpp"
#include "procedural/probing/topology/edge_set.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/test/unit_test.hpp>
#include <random>
#include <unordered_set>
#include <vector>

namespace e8 {
namespace procedural {
namespace {

bool ContainsAllEdges(std::vector<Edge> const &edges,
                      Topology const &topology) {
  std::unordered_set<Edge, EdgeHash> edge_set(edges.size());
  for (auto const &edge : edges) {
    edge_set.insert(edge);
  }

  if (edge_set.size() != edges.size()) {
    return false;
  }

  auto [current, end] = boost::edges(topology);
  for (; current != end; ++current) {
    if (edge_set.find(std::make_tuple(current->m_source, current->m_target)) ==
        edge_set.end()) {
      return false;
    }
  }

  return true;
}

BOOST_AUTO_TEST_CASE(WhenAtOriginalState_ThenCheckActiveAndDeletedEdges) {
  Topology topology = testing::CreateGridTopology(/*side=*/5, /*scale=*/1e3f,
                                                  /*population=*/4e3);
  std::default_random_engine random_engine;
  EdgeSetState edge_set_state = CreateEdgeSetStateFor(topology, &random_engine);

  std::vector<Edge> active_edges = edge_set_state.ActiveEdges();
  std::vector<Edge> deleted_edges = edge_set_state.DeletedEdges();

  BOOST_CHECK_EQUAL(active_edges.size(), 24);
  BOOST_CHECK_EQUAL(deleted_edges.size(), 0);
}

BOOST_AUTO_TEST_CASE(WhenFollowsOneMutation_ThenCheckActiveAndDeletedEdges) {
  Topology topology = testing::CreateGridTopology(/*side=*/5, /*scale=*/1e3f,
                                                  /*population=*/4e3);
  std::default_random_engine random_engine;
  EdgeSetState edge_set_state = CreateEdgeSetStateFor(topology, &random_engine);

  Mutation mutation = edge_set_state.Mutate(/*operation_count=*/1);
  BOOST_CHECK(mutation.additions.empty() && !mutation.deletions.empty());

  std::vector<Edge> active_edges = edge_set_state.ActiveEdges();
  std::vector<Edge> deleted_edges = edge_set_state.DeletedEdges();
  BOOST_CHECK_EQUAL(active_edges.size(), 23);
  BOOST_CHECK_EQUAL(deleted_edges.size(), 1);
}

BOOST_AUTO_TEST_CASE(WhenMutateAndRevertToGoal_ThenCheckActiveAndDeletedEdges) {
  Topology topology = testing::CreateGridTopology(/*side=*/5, /*scale=*/1e3f,
                                                  /*population=*/4e3);
  std::default_random_engine random_engine;
  EdgeSetState edge_set_state = CreateEdgeSetStateFor(topology, &random_engine);

  // Mutate to empty.
  while (!edge_set_state.ActiveEdges().empty()) {
    Mutation mutation = edge_set_state.Mutate(/*operation_count=*/2);
    if (mutation.deletions.empty()) {
      edge_set_state.Revert();
    }
  }

  std::vector<Edge> active_edges = edge_set_state.ActiveEdges();
  std::vector<Edge> deleted_edges = edge_set_state.DeletedEdges();
  BOOST_CHECK_EQUAL(active_edges.size(), 0);
  BOOST_CHECK_EQUAL(deleted_edges.size(), 24);
  BOOST_CHECK(ContainsAllEdges(deleted_edges, topology));

  // Mutate to full.
  while (!edge_set_state.DeletedEdges().empty()) {
    Mutation mutation = edge_set_state.Mutate(/*operation_count=*/2);
    if (mutation.additions.empty()) {
      edge_set_state.Revert();
    }
  }

  active_edges = edge_set_state.ActiveEdges();
  deleted_edges = edge_set_state.DeletedEdges();
  BOOST_CHECK_EQUAL(active_edges.size(), 24);
  BOOST_CHECK_EQUAL(deleted_edges.size(), 0);
  BOOST_CHECK(ContainsAllEdges(active_edges, topology));
}

} // namespace
} // namespace procedural
} // namespace e8
