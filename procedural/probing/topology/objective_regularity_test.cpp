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
#include "procedural/probing/topology/objective_regularity.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/test/unit_test.hpp>
#include <eigen3/Eigen/Core>

namespace e8 {
namespace procedural {
namespace {

BOOST_AUTO_TEST_CASE(WhenAtZeroWayIntersection_ThenCheckRegularityScore) {
  Topology topology(1);
  BOOST_CHECK_EQUAL(-1.f, RegularityObjectiveAt(0, topology));
}

BOOST_AUTO_TEST_CASE(WhenAtOneWayIntersection_ThenCheckRegularityScore) {
  Topology topology(2);
  topology[0] = VertexProperties(Eigen::Vector3f(0, 0, 0), 0, 0);
  topology[1] = VertexProperties(Eigen::Vector3f(200, 0, 0), 0, 0);
  boost::add_edge(0, 1, topology);

  BOOST_CHECK_EQUAL(-.9f, RegularityObjectiveAt(0, topology));
}

BOOST_AUTO_TEST_CASE(WhenAtTwoWayIntersection_ThenCheckRegularityScore) {
  Topology topology(3);
  topology[0] = VertexProperties(Eigen::Vector3f(0, 0, 0), 0, 0);
  topology[1] = VertexProperties(Eigen::Vector3f(200, 0, 0), 0, 0);
  topology[2] = VertexProperties(Eigen::Vector3f(-200, 200, 0), 0, 0);

  boost::add_edge(0, 1, topology);
  boost::add_edge(0, 2, topology);

  BOOST_CHECK_CLOSE(.64f, RegularityObjectiveAt(0, topology), 1.f);
}

BOOST_AUTO_TEST_CASE(
    WhenAtPerfectThreeWayIntersection_ThenCheckRegularityScore) {
  Topology topology(4);
  topology[0] = VertexProperties(Eigen::Vector3f(0, 0, 0), 0, 0);
  topology[1] = VertexProperties(Eigen::Vector3f(200, 0, 0), 0, 0);
  topology[2] = VertexProperties(Eigen::Vector3f(-200, 0, 0), 0, 0);
  topology[3] = VertexProperties(Eigen::Vector3f(0, 100, 0), 0, 0);

  boost::add_edge(0, 1, topology);
  boost::add_edge(0, 2, topology);
  boost::add_edge(0, 3, topology);

  BOOST_CHECK_CLOSE(.9f, RegularityObjectiveAt(0, topology), 1.f);
}

BOOST_AUTO_TEST_CASE(
    WhenAtDisortedThreeWayIntersection_ThenCheckRegularityScore) {
  Topology topology(4);
  topology[0] = VertexProperties(Eigen::Vector3f(0, 0, 0), 0, 0);
  topology[1] = VertexProperties(Eigen::Vector3f(200, 0, 0), 0, 0);
  topology[2] = VertexProperties(Eigen::Vector3f(-200, 0, 0), 0, 0);
  topology[3] = VertexProperties(Eigen::Vector3f(200, 100, 0), 0, 0);

  boost::add_edge(0, 1, topology);
  boost::add_edge(0, 2, topology);
  boost::add_edge(0, 3, topology);

  BOOST_CHECK_CLOSE(-1.f, RegularityObjectiveAt(0, topology), 1.f);
}

BOOST_AUTO_TEST_CASE(
    WhenAtPerfectFourWayIntersection_ThenCheckRegularityScore) {
  Topology topology(5);
  topology[0] = VertexProperties(Eigen::Vector3f(0, 0, 0), 0, 0);
  topology[1] = VertexProperties(Eigen::Vector3f(200, 0, 0), 0, 0);
  topology[2] = VertexProperties(Eigen::Vector3f(-200, 0, 0), 0, 0);
  topology[3] = VertexProperties(Eigen::Vector3f(0, 100, 0), 0, 0);
  topology[4] = VertexProperties(Eigen::Vector3f(0, -100, 0), 0, 0);

  boost::add_edge(0, 1, topology);
  boost::add_edge(0, 2, topology);
  boost::add_edge(0, 3, topology);
  boost::add_edge(0, 4, topology);

  BOOST_CHECK_CLOSE(1.f, RegularityObjectiveAt(0, topology), 1.f);
}

BOOST_AUTO_TEST_CASE(
    WhenAtDistortedFourWayIntersection_ThenCheckRegularityScore) {
  Topology topology(5);
  topology[0] = VertexProperties(Eigen::Vector3f(0, 0, 0), 0, 0);
  topology[1] = VertexProperties(Eigen::Vector3f(200, 0, 0), 0, 0);
  topology[2] = VertexProperties(Eigen::Vector3f(-200, 0, 0), 0, 0);
  topology[3] = VertexProperties(Eigen::Vector3f(0, 100, 0), 0, 0);
  topology[4] = VertexProperties(Eigen::Vector3f(-200, -100, 0), 0, 0);

  boost::add_edge(0, 1, topology);
  boost::add_edge(0, 2, topology);
  boost::add_edge(0, 3, topology);
  boost::add_edge(0, 4, topology);

  BOOST_CHECK_CLOSE(-1.f, RegularityObjectiveAt(0, topology), 1.f);
}

BOOST_AUTO_TEST_CASE(WhenTopologyIsGrid_ThenCheckObjective) {
  Topology grid = testing::CreateGridTopology(/*side=*/10, /*scale=*/1000.f,
                                              /*population=*/1e3f);
  RegularityScoreMap score_map = CreateRegularityScoreMapFor(grid);
  BOOST_CHECK_CLOSE(31.f, EvaluateRegularityObjective(score_map), 1.f);
}

} // namespace
} // namespace procedural
} // namespace e8