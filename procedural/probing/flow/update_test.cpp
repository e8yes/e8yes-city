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
#include "procedural/probing/flow/topology.hpp"
#include "procedural/probing/flow/update.hpp"
#include "procedural/probing/probe/probe.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/test/unit_test.hpp>
#include <eigen3/Eigen/Core>
#include <vector>

namespace e8 {
namespace procedural {
namespace {

BOOST_AUTO_TEST_CASE(
    WhenSimulatedFlowIsCurrentFlow_ThenCheckUpdatedFlowIsInvariant) {
  TopologyFlow current = testing::CreateGridTopologyFlow(/*flow_value=*/1.f);
  TopologyFlow simulated = testing::CreateGridTopologyFlow(/*flow_value=*/1.f);

  FlowStatistics stats = UpdateFlow(simulated, &current);
  BOOST_CHECK_EQUAL(1.f, stats.min);
  BOOST_CHECK_EQUAL(1.f, stats.max);
  BOOST_CHECK_EQUAL(1.f, stats.mean);
  BOOST_CHECK_EQUAL(0.f, stats.std);

  BOOST_CHECK_EQUAL(9, boost::num_vertices(current));
  BOOST_CHECK_EQUAL(8, boost::num_edges(current));

  auto [edge_14, exists_14] = boost::edge(1, 4, current);
  auto [edge_41, exists_41] = boost::edge(4, 1, current);
  BOOST_CHECK(exists_14);
  BOOST_CHECK(exists_41);
  BOOST_CHECK_CLOSE(1.f, boost::get(&FlowProperties::flow, current, edge_14),
                    1.f);
  BOOST_CHECK_EQUAL(1,
                    boost::get(&FlowProperties::lane_count, current, edge_14));
  BOOST_CHECK_CLOSE(1.f, boost::get(&FlowProperties::flow, current, edge_41),
                    1.f);
  BOOST_CHECK_EQUAL(1,
                    boost::get(&FlowProperties::lane_count, current, edge_41));

  auto [edge_34, exists_34] = boost::edge(3, 4, current);
  auto [edge_43, exists_43] = boost::edge(4, 3, current);
  BOOST_CHECK(exists_34);
  BOOST_CHECK(exists_43);
  BOOST_CHECK_CLOSE(1.f, boost::get(&FlowProperties::flow, current, edge_34),
                    1.f);
  BOOST_CHECK_EQUAL(1,
                    boost::get(&FlowProperties::lane_count, current, edge_34));
  BOOST_CHECK_CLOSE(1.f, boost::get(&FlowProperties::flow, current, edge_43),
                    1.f);
  BOOST_CHECK_EQUAL(1,
                    boost::get(&FlowProperties::lane_count, current, edge_43));

  auto [edge_54, exists_54] = boost::edge(5, 4, current);
  auto [edge_45, exists_45] = boost::edge(4, 5, current);
  BOOST_CHECK(exists_54);
  BOOST_CHECK(exists_45);
  BOOST_CHECK_CLOSE(1.f, boost::get(&FlowProperties::flow, current, edge_54),
                    1.f);
  BOOST_CHECK_EQUAL(1,
                    boost::get(&FlowProperties::lane_count, current, edge_54));
  BOOST_CHECK_CLOSE(1.f, boost::get(&FlowProperties::flow, current, edge_45),
                    1.f);
  BOOST_CHECK_EQUAL(1,
                    boost::get(&FlowProperties::lane_count, current, edge_45));

  auto [edge_74, exists_74] = boost::edge(7, 4, current);
  auto [edge_47, exists_47] = boost::edge(4, 7, current);
  BOOST_CHECK(exists_74);
  BOOST_CHECK(exists_47);
  BOOST_CHECK_CLOSE(1.f, boost::get(&FlowProperties::flow, current, edge_74),
                    1.f);
  BOOST_CHECK_EQUAL(1,
                    boost::get(&FlowProperties::lane_count, current, edge_74));
  BOOST_CHECK_CLOSE(1.f, boost::get(&FlowProperties::flow, current, edge_47),
                    1.f);
  BOOST_CHECK_EQUAL(1,
                    boost::get(&FlowProperties::lane_count, current, edge_47));
}

BOOST_AUTO_TEST_CASE(WhenSimulatedFlowIsDifferent_ThenCheckFlowValueIsUpdated) {
  TopologyFlow current = testing::CreateGridTopologyFlow(/*flow_value=*/1.f);

  TopologyFlow simulated = current;
  auto simulated_edge_14 = boost::edge(1, 4, simulated).first;
  boost::put(boost::edge_bundle_t(), simulated, simulated_edge_14,
             FlowProperties(/*flow=*/1e3f, /*lane_count=*/1));

  FlowStatistics stats = UpdateFlow(simulated, &current);
  BOOST_CHECK_EQUAL(1.f, stats.min);
  BOOST_CHECK_CLOSE(1e2f, stats.max, 1.f);
  BOOST_CHECK_CLOSE(13.5f, stats.mean, 1.f);
  BOOST_CHECK_CLOSE(33.f, stats.std, 1.f);

  BOOST_CHECK_EQUAL(9, boost::num_vertices(current));
  BOOST_CHECK_EQUAL(8, boost::num_edges(current));

  auto [edge_14, exists_14] = boost::edge(1, 4, current);
  auto [edge_41, exists_41] = boost::edge(4, 1, current);
  BOOST_CHECK(exists_14);
  BOOST_CHECK(exists_41);
  BOOST_CHECK_CLOSE(100.f, boost::get(&FlowProperties::flow, current, edge_14),
                    1.f);
  BOOST_CHECK_EQUAL(2,
                    boost::get(&FlowProperties::lane_count, current, edge_14));
  BOOST_CHECK_CLOSE(1.f, boost::get(&FlowProperties::flow, current, edge_41),
                    1.f);
  BOOST_CHECK_EQUAL(1,
                    boost::get(&FlowProperties::lane_count, current, edge_41));

  auto [edge_34, exists_34] = boost::edge(3, 4, current);
  auto [edge_43, exists_43] = boost::edge(4, 3, current);
  BOOST_CHECK(exists_34);
  BOOST_CHECK(exists_43);
  BOOST_CHECK_CLOSE(1.f, boost::get(&FlowProperties::flow, current, edge_34),
                    1.f);
  BOOST_CHECK_EQUAL(1,
                    boost::get(&FlowProperties::lane_count, current, edge_34));
  BOOST_CHECK_CLOSE(1.f, boost::get(&FlowProperties::flow, current, edge_43),
                    1.f);
  BOOST_CHECK_EQUAL(1,
                    boost::get(&FlowProperties::lane_count, current, edge_43));

  auto [edge_54, exists_54] = boost::edge(5, 4, current);
  auto [edge_45, exists_45] = boost::edge(4, 5, current);
  BOOST_CHECK(exists_54);
  BOOST_CHECK(exists_45);
  BOOST_CHECK_CLOSE(1.f, boost::get(&FlowProperties::flow, current, edge_54),
                    1.f);
  BOOST_CHECK_EQUAL(1,
                    boost::get(&FlowProperties::lane_count, current, edge_54));
  BOOST_CHECK_CLOSE(1.f, boost::get(&FlowProperties::flow, current, edge_45),
                    1.f);
  BOOST_CHECK_EQUAL(1,
                    boost::get(&FlowProperties::lane_count, current, edge_45));

  auto [edge_74, exists_74] = boost::edge(7, 4, current);
  auto [edge_47, exists_47] = boost::edge(4, 7, current);
  BOOST_CHECK(exists_74);
  BOOST_CHECK(exists_47);
  BOOST_CHECK_CLOSE(1.f, boost::get(&FlowProperties::flow, current, edge_74),
                    1.f);
  BOOST_CHECK_EQUAL(1,
                    boost::get(&FlowProperties::lane_count, current, edge_74));
  BOOST_CHECK_CLOSE(1.f, boost::get(&FlowProperties::flow, current, edge_47),
                    1.f);
  BOOST_CHECK_EQUAL(1,
                    boost::get(&FlowProperties::lane_count, current, edge_47));
}

} // namespace
} // namespace procedural
} // namespace e8
