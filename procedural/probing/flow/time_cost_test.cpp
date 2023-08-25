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
#include "procedural/probing/flow/time_cost.hpp"
#include "procedural/probing/flow/topology.hpp"
#include "procedural/probing/probe/probe.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/test/unit_test.hpp>
#include <eigen3/Eigen/Core>
#include <vector>

namespace e8 {
namespace procedural {
namespace {

std::vector<PopulationProbe> CreatePopulationProbes() {
  return std::vector<PopulationProbe>{
      PopulationProbe(Eigen::Vector3f(-1000, 1000, 0), 0),
      PopulationProbe(Eigen::Vector3f(0, 1000, 0), 0),
      PopulationProbe(Eigen::Vector3f(1000, 1000, 0), 0),

      PopulationProbe(Eigen::Vector3f(-1000, 0, 0), 0),
      PopulationProbe(Eigen::Vector3f(0, 0, 0), 0),
      PopulationProbe(Eigen::Vector3f(1000, 0, 0), 0),

      PopulationProbe(Eigen::Vector3f(-1000, -1000, 0), 0),
      PopulationProbe(Eigen::Vector3f(0, -1000, 0), 0),
      PopulationProbe(Eigen::Vector3f(1000, -1000, 0), 0),
  };
}

BOOST_AUTO_TEST_CASE(WhenTopologyHasZeroFlow_ThenCheckCost) {
  std::vector<PopulationProbe> probes = CreatePopulationProbes();
  TopologyFlow flows = testing::CreateGridTopologyFlow(/*flow_value=*/.0f);
  FlowCostMap cost_map = CreateFlowCostMapFrom(flows, probes);

  BOOST_CHECK_EQUAL(9, boost::num_vertices(cost_map));
  BOOST_CHECK_EQUAL(8, boost::num_edges(cost_map));

  auto [edge_14, exists_14] = boost::edge(1, 4, cost_map);
  auto [edge_41, exists_41] = boost::edge(4, 1, cost_map);
  BOOST_CHECK(exists_14);
  BOOST_CHECK(exists_41);
  BOOST_CHECK_CLOSE(25.8f,
                    boost::get(boost::edge_weight_t(), cost_map, edge_14), 1.f);
  BOOST_CHECK_CLOSE(25.8f,
                    boost::get(boost::edge_weight_t(), cost_map, edge_41), 1.f);

  auto [edge_34, exists_34] = boost::edge(3, 4, cost_map);
  auto [edge_43, exists_43] = boost::edge(4, 3, cost_map);
  BOOST_CHECK(exists_34);
  BOOST_CHECK(exists_43);
  BOOST_CHECK_CLOSE(25.8f,
                    boost::get(boost::edge_weight_t(), cost_map, edge_34), 1.f);
  BOOST_CHECK_CLOSE(25.8f,
                    boost::get(boost::edge_weight_t(), cost_map, edge_43), 1.f);

  auto [edge_54, exists_54] = boost::edge(5, 4, cost_map);
  auto [edge_45, exists_45] = boost::edge(4, 5, cost_map);
  BOOST_CHECK(exists_54);
  BOOST_CHECK(exists_45);
  BOOST_CHECK_CLOSE(25.8f,
                    boost::get(boost::edge_weight_t(), cost_map, edge_54), 1.f);
  BOOST_CHECK_CLOSE(25.8f,
                    boost::get(boost::edge_weight_t(), cost_map, edge_45), 1.f);

  auto [edge_74, exists_74] = boost::edge(7, 4, cost_map);
  auto [edge_47, exists_47] = boost::edge(4, 7, cost_map);
  BOOST_CHECK(exists_74);
  BOOST_CHECK(exists_47);
  BOOST_CHECK_CLOSE(25.8f,
                    boost::get(boost::edge_weight_t(), cost_map, edge_74), 1.f);
  BOOST_CHECK_CLOSE(25.8f,
                    boost::get(boost::edge_weight_t(), cost_map, edge_47), 1.f);
}

BOOST_AUTO_TEST_CASE(WhenTopologyHasMedianFlow_ThenCheckCost) {
  std::vector<PopulationProbe> probes = CreatePopulationProbes();
  TopologyFlow flows = testing::CreateGridTopologyFlow(/*flow_value=*/122.f);
  FlowCostMap cost_map = CreateFlowCostMapFrom(flows, probes);

  BOOST_CHECK_EQUAL(9, boost::num_vertices(cost_map));
  BOOST_CHECK_EQUAL(8, boost::num_edges(cost_map));

  auto [edge_14, exists_14] = boost::edge(1, 4, cost_map);
  auto [edge_41, exists_41] = boost::edge(4, 1, cost_map);
  BOOST_CHECK(exists_14);
  BOOST_CHECK(exists_41);
  BOOST_CHECK_CLOSE(61.9f,
                    boost::get(boost::edge_weight_t(), cost_map, edge_14), 1.f);
  BOOST_CHECK_CLOSE(42.3f,
                    boost::get(boost::edge_weight_t(), cost_map, edge_41), 1.f);

  auto [edge_34, exists_34] = boost::edge(3, 4, cost_map);
  auto [edge_43, exists_43] = boost::edge(4, 3, cost_map);
  BOOST_CHECK(exists_34);
  BOOST_CHECK(exists_43);
  BOOST_CHECK_CLOSE(61.9f,
                    boost::get(boost::edge_weight_t(), cost_map, edge_34), 1.f);
  BOOST_CHECK_CLOSE(42.3f,
                    boost::get(boost::edge_weight_t(), cost_map, edge_43), 1.f);

  auto [edge_54, exists_54] = boost::edge(5, 4, cost_map);
  auto [edge_45, exists_45] = boost::edge(4, 5, cost_map);
  BOOST_CHECK(exists_54);
  BOOST_CHECK(exists_45);
  BOOST_CHECK_CLOSE(61.9f,
                    boost::get(boost::edge_weight_t(), cost_map, edge_54), 1.f);
  BOOST_CHECK_CLOSE(42.3f,
                    boost::get(boost::edge_weight_t(), cost_map, edge_45), 1.f);

  auto [edge_74, exists_74] = boost::edge(7, 4, cost_map);
  auto [edge_47, exists_47] = boost::edge(4, 7, cost_map);
  BOOST_CHECK(exists_74);
  BOOST_CHECK(exists_47);
  BOOST_CHECK_CLOSE(61.9f,
                    boost::get(boost::edge_weight_t(), cost_map, edge_74), 1.f);
  BOOST_CHECK_CLOSE(42.3f,
                    boost::get(boost::edge_weight_t(), cost_map, edge_47), 1.f);
}

} // namespace
} // namespace procedural
} // namespace e8
