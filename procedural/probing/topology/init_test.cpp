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
#include "procedural/probing/topology/init.hpp"
#include "procedural/probing/topology/probe.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/test/unit_test.hpp>
#include <eigen3/Eigen/Core>
#include <vector>

namespace e8 {
namespace procedural {
namespace {

BOOST_AUTO_TEST_CASE(WhenProbesAreSquare_ThenCheckTopologyIsMesh) {
  std::vector<PopulationProbe> probes{
      PopulationProbe(
          /*location=*/Eigen::Vector3f(0, 0, 0), /*population_grid_200=*/100.f),
      PopulationProbe(
          /*location=*/Eigen::Vector3f(0, 1000, 0),
          /*population_grid_200=*/200.f),
      PopulationProbe(
          /*location=*/Eigen::Vector3f(1000, 1000, 0),
          /*population_grid_200=*/300.f),
      PopulationProbe(
          /*location=*/Eigen::Vector3f(1000, 0, 0),
          /*population_grid_200=*/400.f)};

  Topology topology = CreateDelaunayTopology(probes);
  BOOST_CHECK_EQUAL(5, boost::num_edges(topology));
  BOOST_CHECK(boost::edge(0, 1, topology).second);
  BOOST_CHECK(boost::edge(1, 2, topology).second);
  BOOST_CHECK(boost::edge(2, 3, topology).second);
  BOOST_CHECK(boost::edge(3, 0, topology).second);
  BOOST_CHECK(boost::edge(3, 1, topology).second);

  BOOST_CHECK_EQUAL(4, boost::num_vertices(topology));
  BOOST_CHECK_CLOSE(100.f, topology[0].local_population, 1);
  BOOST_CHECK_CLOSE(200.f, topology[1].local_population, 1);
  BOOST_CHECK_CLOSE(300.f, topology[2].local_population, 1);
  BOOST_CHECK_CLOSE(400.f, topology[3].local_population, 1);

  BOOST_CHECK_EQUAL(Eigen::Vector3f(0, 0, 0), topology[0].location);
  BOOST_CHECK_EQUAL(Eigen::Vector3f(0, 1000, 0), topology[1].location);
  BOOST_CHECK_EQUAL(Eigen::Vector3f(1000, 1000, 0), topology[2].location);
  BOOST_CHECK_EQUAL(Eigen::Vector3f(1000, 0, 0), topology[3].location);

  BOOST_CHECK_LT(topology[0].importance, topology[1].importance);
  BOOST_CHECK_LT(topology[1].importance, topology[2].importance);
  BOOST_CHECK_LT(topology[2].importance, topology[3].importance);

  BOOST_CHECK_CLOSE(1.f,
                    topology[0].importance + topology[1].importance +
                        topology[2].importance + topology[3].importance,
                    1);
}

} // namespace
} // namespace procedural
} // namespace e8
