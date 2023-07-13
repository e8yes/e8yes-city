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
#include "procedural/probing/topology_definition.hpp"
#include "procedural/probing/topology_mutation.hpp"
#include "procedural/probing/topology_objective.hpp"
#include <boost/test/unit_test.hpp>
#include <random>

namespace e8 {
namespace procedural {
namespace {

Topology CreateTopology() {
  // Structure:
  // 1----
  // |   |
  // 0---2-------3
  Topology topology(4);
  topology[0] = VertexProperties(/*location=*/Eigen::Vector3f(0, 0, 0),
                                 /*local_population=*/200,
                                 /*area_population=*/0,
                                 /*importance=*/1.0f / 4.0f);
  topology[1] = VertexProperties(/*location=*/Eigen::Vector3f(0, 1000, 0),
                                 /*local_population=*/100,
                                 /*area_population=*/0,
                                 /*importance=*/1.0f / 4.0f);
  topology[2] = VertexProperties(/*location=*/Eigen::Vector3f(1000, 0, 0),
                                 /*local_population=*/300,
                                 /*area_population=*/0,
                                 /*importance=*/1.0f / 4.0f);
  topology[3] = VertexProperties(/*location=*/Eigen::Vector3f(3000, 0, 0),
                                 /*local_population=*/50,
                                 /*area_population=*/0,
                                 /*importance=*/1.0f / 4.0f);

  boost::add_edge(0, 1, EstimateTravelTimeCost(0, 1, topology), topology);
  boost::add_edge(0, 2, EstimateTravelTimeCost(0, 2, topology), topology);
  boost::add_edge(1, 2, EstimateTravelTimeCost(1, 2, topology), topology);
  boost::add_edge(2, 3, EstimateTravelTimeCost(2, 3, topology), topology);

  return topology;
}

BOOST_AUTO_TEST_CASE(CheckRevertibleMutation) {
  Topology topology = CreateTopology();
  CostMap cost_map = CreateCostMapForTopology(topology);

  Mutation mutation(/*num_additions=*/0, /*num_deletions=*/2);
  mutation.deletions.insert(Edge(0, 2));
  mutation.deletions.insert(Edge(2, 3));

  RevertibleMutation revertible(std::move(mutation), cost_map);
  BOOST_CHECK_EQUAL(0, revertible.mutation.additions.size());
  BOOST_CHECK_EQUAL(2, revertible.mutation.deletions.size());
  BOOST_CHECK(revertible.mutation.deletions.find(Edge(0, 2)) !=
              revertible.mutation.deletions.end());
  BOOST_CHECK(revertible.mutation.deletions.find(Edge(2, 3)) !=
              revertible.mutation.deletions.end());

  BOOST_CHECK_EQUAL(2, revertible.affected_edges.size());
  float cost_01 = boost::get(boost::edge_weight_t(), cost_map,
                             boost::edge(0, 1, cost_map).first);
  float cost_12 = boost::get(boost::edge_weight_t(), cost_map,
                             boost::edge(1, 2, cost_map).first);

  BOOST_CHECK(cost_01 == revertible.affected_edges[Edge(0, 1)] ||
              cost_01 == revertible.affected_edges[Edge(1, 0)]);
  BOOST_CHECK(cost_12 == revertible.affected_edges[Edge(1, 2)] ||
              cost_12 == revertible.affected_edges[Edge(2, 1)]);

  float cost_02 = boost::get(boost::edge_weight_t(), cost_map,
                             boost::edge(0, 2, cost_map).first);
  float cost_23 = boost::get(boost::edge_weight_t(), cost_map,
                             boost::edge(2, 3, cost_map).first);
  BOOST_CHECK_EQUAL(2, revertible.deleted_edges.size());
  BOOST_CHECK_EQUAL(cost_02, revertible.deleted_edges[Edge(0, 2)]);
  BOOST_CHECK_EQUAL(cost_23, revertible.deleted_edges[Edge(2, 3)]);
}

BOOST_AUTO_TEST_CASE(WhenDeleteEdgesAndRevert_ThenCheckCostMap) {
  Topology topology = CreateTopology();
  CostMap cost_map = CreateCostMapForTopology(topology);

  Mutation mutation(/*num_additions=*/0, /*num_deletions=*/2);
  mutation.deletions.insert(Edge(0, 2));
  mutation.deletions.insert(Edge(2, 3));

  RevertibleMutation revertible(std::move(mutation), cost_map);

  ApplyMutation(revertible, topology, &cost_map);
  BOOST_CHECK_EQUAL(2, boost::num_edges(cost_map));
  BOOST_CHECK(boost::edge(0, 1, cost_map).second);
  BOOST_CHECK(boost::edge(1, 2, cost_map).second);
  float cost_01 = boost::get(boost::edge_weight_t(), cost_map,
                             boost::edge(0, 1, cost_map).first);
  float cost_12 = boost::get(boost::edge_weight_t(), cost_map,
                             boost::edge(1, 2, cost_map).first);
  BOOST_CHECK_CLOSE(61, cost_01, 1);
  BOOST_CHECK_CLOSE(110, cost_12, 1);

  RevertMutation(revertible, &cost_map);
  BOOST_CHECK_EQUAL(4, boost::num_edges(cost_map));
  BOOST_CHECK(boost::edge(0, 1, cost_map).second);
  BOOST_CHECK(boost::edge(0, 2, cost_map).second);
  BOOST_CHECK(boost::edge(1, 2, cost_map).second);
  BOOST_CHECK(boost::edge(2, 3, cost_map).second);

  cost_01 = boost::get(boost::edge_weight_t(), cost_map,
                       boost::edge(0, 1, cost_map).first);
  float cost_02 = boost::get(boost::edge_weight_t(), cost_map,
                             boost::edge(0, 2, cost_map).first);
  cost_12 = boost::get(boost::edge_weight_t(), cost_map,
                       boost::edge(1, 2, cost_map).first);
  float cost_23 = boost::get(boost::edge_weight_t(), cost_map,
                             boost::edge(2, 3, cost_map).first);
  BOOST_CHECK_CLOSE(66, cost_01, 1);
  BOOST_CHECK_CLOSE(120, cost_02, 1);
  BOOST_CHECK_CLOSE(130, cost_12, 1);
  BOOST_CHECK_CLOSE(162, cost_23, 1);
}

BOOST_AUTO_TEST_CASE(WhenMutateEdgesAndRevert_ThenCheckCostMap) {
  Topology topology = CreateTopology();
  CostMap cost_map = CreateCostMapForTopology(topology);

  Mutation mutation(/*num_additions=*/0, /*num_deletions=*/2);
  mutation.deletions.insert(Edge(0, 2));
  mutation.deletions.insert(Edge(2, 3));
  RevertibleMutation revertible(std::move(mutation), cost_map);
  ApplyMutation(revertible, topology, &cost_map);

  Mutation mutation2(/*num_additions=*/1, /*num_deletions=*/1);
  mutation2.additions.insert(Edge(0, 2));
  mutation2.deletions.insert(Edge(0, 1));
  RevertibleMutation revertible2(std::move(mutation2), cost_map);

  ApplyMutation(revertible2, topology, &cost_map);
  BOOST_CHECK_EQUAL(2, boost::num_edges(cost_map));
  BOOST_CHECK(boost::edge(0, 2, cost_map).second);
  BOOST_CHECK(boost::edge(1, 2, cost_map).second);

  float cost_02 = boost::get(boost::edge_weight_t(), cost_map,
                             boost::edge(0, 2, cost_map).first);
  float cost_12 = boost::get(boost::edge_weight_t(), cost_map,
                             boost::edge(1, 2, cost_map).first);
  BOOST_CHECK_CLOSE(100, cost_02, 1);
  BOOST_CHECK_CLOSE(110, cost_12, 1);

  RevertMutation(revertible2, &cost_map);
  BOOST_CHECK_EQUAL(2, boost::num_edges(cost_map));
  BOOST_CHECK(boost::edge(0, 1, cost_map).second);
  BOOST_CHECK(boost::edge(1, 2, cost_map).second);
  float cost_01 = boost::get(boost::edge_weight_t(), cost_map,
                             boost::edge(0, 1, cost_map).first);
  cost_12 = boost::get(boost::edge_weight_t(), cost_map,
                       boost::edge(1, 2, cost_map).first);
  BOOST_CHECK_CLOSE(61, cost_01, 1);
  BOOST_CHECK_CLOSE(110, cost_12, 1);
}

} // namespace
} // namespace procedural
} // namespace e8
