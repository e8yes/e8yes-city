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
#include "procedural/probing/topology/mutation_efficiency.hpp"
#include "procedural/probing/topology/mutation_regularity.hpp"
#include "procedural/probing/topology/objective_regularity.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/test/unit_test.hpp>
#include <eigen3/Eigen/Core>

namespace e8 {
namespace procedural {
namespace {

BOOST_AUTO_TEST_CASE(WhenMutateGridTopology_ThenCheckRevertible) {
  Topology grid = testing::CreateGridTopology(/*side=*/3, /*scale=*/1000.f,
                                              /*population=*/1e3f);
  RegularityScoreMap score_map = CreateRegularityScoreMapFor(grid);
  float score = EvaluateRegularityObjective(score_map);

  Mutation mutation(/*num_additions=*/1, /*num_deletions=*/1);
  mutation.PushAddition(Edge(0, 1));
  mutation.PushDeletion(Edge(3, 4));

  RevertibleRegularityMutation revertible(std::move(mutation), score_map,
                                          score);

  BOOST_CHECK_EQUAL(1, revertible.mutation.additions.size());
  BOOST_CHECK_EQUAL(1, revertible.mutation.deletions.size());

  BOOST_CHECK_EQUAL(4, revertible.affected_vertices.size());
  BOOST_CHECK_CLOSE(-1.f, revertible.affected_vertices[0], 1.f);
  BOOST_CHECK_CLOSE(-.9f, revertible.affected_vertices[1], 1.f);
  BOOST_CHECK_CLOSE(-.9f, revertible.affected_vertices[3], 1.f);
  BOOST_CHECK_CLOSE(1.f, revertible.affected_vertices[4], 1.f);

  BOOST_CHECK_CLOSE(score, revertible.score, 1.f);
}

BOOST_AUTO_TEST_CASE(WhenApplyAndRevertMutation_ThenCheckTopologyAndScore) {
  Topology grid = testing::CreateGridTopology(/*side=*/3, /*scale=*/1000.f,
                                              /*population=*/1e3f);
  RegularityScoreMap score_map = CreateRegularityScoreMapFor(grid);
  float score = EvaluateRegularityObjective(score_map);

  Mutation mutation(/*num_additions=*/1, /*num_deletions=*/1);
  mutation.PushAddition(Edge(0, 1));
  mutation.PushDeletion(Edge(3, 4));

  RevertibleRegularityMutation revertible(std::move(mutation), score_map,
                                          score);
  double new_score = ApplyMutation(revertible, &grid, &score_map);
  BOOST_CHECK_CLOSE(-5.8f, new_score, 1.f);
  BOOST_CHECK_EQUAL(4, boost::num_edges(grid));
  BOOST_CHECK(boost::edge(0, 1, grid).second);
  BOOST_CHECK(boost::edge(1, 4, grid).second);
  BOOST_CHECK(boost::edge(4, 5, grid).second);
  BOOST_CHECK(boost::edge(4, 7, grid).second);

  double old_score = RevertMutation(revertible, &grid, &score_map);
  BOOST_CHECK_CLOSE(-6.6f, old_score, 1.f);
  BOOST_CHECK_EQUAL(4, boost::num_edges(grid));
  BOOST_CHECK(boost::edge(4, 1, grid).second);
  BOOST_CHECK(boost::edge(4, 3, grid).second);
  BOOST_CHECK(boost::edge(4, 5, grid).second);
  BOOST_CHECK(boost::edge(4, 7, grid).second);
}

} // namespace
} // namespace procedural
} // namespace e8
