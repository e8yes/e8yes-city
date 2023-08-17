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
#include "procedural/probing/flow/simulate.hpp"
#include "procedural/probing/flow/topology.hpp"
#include "procedural/probing/probe/probe.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/test/unit_test.hpp>
#include <eigen3/Eigen/Core>
#include <vector>

namespace e8 {
namespace procedural {
namespace {

BOOST_AUTO_TEST_CASE(WhenTopologyIsIslands_ThenCheckFlow) {
  std::vector<PopulationProbe> probes{
      PopulationProbe(Eigen::Vector3f(0, 0, 0),
                      /*population_grid_200=*/1e3f),
      PopulationProbe(Eigen::Vector3f(1000, 0, 0),
                      /*population_grid_200=*/2e3f)};
  TopologyFlow flow(2);

  TopologyFlow simulated_flow = SimulateFlow(flow, probes);
  BOOST_CHECK_EQUAL(2, boost::num_vertices(simulated_flow));
  BOOST_CHECK_EQUAL(0, boost::num_edges(simulated_flow));
}

BOOST_AUTO_TEST_CASE(WhenTopologyIsLine_ThenCheckFlow) {
  std::vector<PopulationProbe> probes{
      PopulationProbe(Eigen::Vector3f(0, 0, 0), /*population_grid_200=*/1e2f),
      PopulationProbe(Eigen::Vector3f(1000, 0, 0),
                      /*population_grid_200=*/1e3f),
      PopulationProbe(Eigen::Vector3f(2000, 0, 0),
                      /*population_grid_200=*/1e4f)};

  TopologyFlow flow(3);
  boost::add_edge(0, 1, FlowProperties(0, 1), flow);
  boost::add_edge(1, 0, FlowProperties(0, 1), flow);

  boost::add_edge(1, 2, FlowProperties(0, 1), flow);
  boost::add_edge(2, 1, FlowProperties(0, 1), flow);

  TopologyFlow simulated_flow = SimulateFlow(flow, probes);
  BOOST_CHECK_EQUAL(3, boost::num_vertices(simulated_flow));
  BOOST_CHECK_EQUAL(4, boost::num_edges(simulated_flow));

  auto [edge_01, exists_01] = boost::edge(0, 1, simulated_flow);
  auto [edge_10, exists_10] = boost::edge(1, 0, simulated_flow);
  auto [edge_12, exists_12] = boost::edge(1, 2, simulated_flow);
  auto [edge_21, exists_21] = boost::edge(2, 1, simulated_flow);
  BOOST_CHECK(exists_01);
  BOOST_CHECK(exists_10);
  BOOST_CHECK(exists_12);
  BOOST_CHECK(exists_21);

  BOOST_CHECK_CLOSE(
      99, boost::get(&FlowProperties::flow, simulated_flow, edge_01), 1.f);
  BOOST_CHECK_CLOSE(
      103, boost::get(&FlowProperties::flow, simulated_flow, edge_10), 1.f);
  BOOST_CHECK_CLOSE(
      993, boost::get(&FlowProperties::flow, simulated_flow, edge_12), 1.f);
  BOOST_CHECK_CLOSE(
      1011, boost::get(&FlowProperties::flow, simulated_flow, edge_21), 1.f);
}

} // namespace
} // namespace procedural
} // namespace e8
