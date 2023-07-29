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
#include "procedural/probing/topology/objective.hpp"
#include "procedural/probing/topology/sampler.hpp"
#include <boost/test/unit_test.hpp>
#include <eigen3/Eigen/Core>

namespace e8 {
namespace procedural {
namespace {

BOOST_AUTO_TEST_CASE(WhenPopulationIsSparse_ThenCheckTravelTime) {
  Topology topology(2);
  topology[0] = VertexProperties(/*location=*/Eigen::Vector3f(.0f, .0f, .0f),
                                 /*local_population=*/.0f,
                                 /*importance=*/0.5f);
  topology[1] =
      VertexProperties(/*location=*/Eigen::Vector3f(1000.0f, .0f, .0f),
                       /*local_population=*/.0f,
                       /*importance=*/.5f);
  BOOST_CHECK_CLOSE(25.706940874f,
                    EstimateTravelTimeCost(/*u=*/0, /*v=*/1, topology), 5.0f);
  BOOST_CHECK_EQUAL(EstimateTravelTimeCost(/*u=*/0, /*v=*/1, topology),
                    EstimateTravelTimeCost(/*u=*/1, /*v=*/0, topology));
}

BOOST_AUTO_TEST_CASE(WhenPopulationIsDense_ThenCheckTravelTime) {
  Topology topology(2);
  topology[0] = VertexProperties(/*location=*/Eigen::Vector3f(.0f, .0f, .0f),
                                 /*local_population=*/500.0f,
                                 /*importance=*/0.5f);
  topology[1] =
      VertexProperties(/*location=*/Eigen::Vector3f(1000.0f, .0f, .0f),
                       /*local_population=*/500.0f,
                       /*importance=*/.5f);
  BOOST_CHECK_CLOSE(120.048019208f,
                    EstimateTravelTimeCost(/*u=*/0, /*v=*/1, topology), 5.0f);
  BOOST_CHECK_EQUAL(EstimateTravelTimeCost(/*u=*/0, /*v=*/1, topology),
                    EstimateTravelTimeCost(/*u=*/1, /*v=*/0, topology));
}

BOOST_AUTO_TEST_CASE(CheckWaitTimeCost) {
  CostMap cost_map(4);
  boost::add_edge(0, 1, cost_map);
  boost::add_edge(0, 2, cost_map);
  boost::add_edge(2, 3, cost_map);

  float cost_01 = EstimateWaitTimeCost(0, 1, cost_map);
  float cost_02 = EstimateWaitTimeCost(0, 2, cost_map);

  BOOST_CHECK_CLOSE(35.0f, cost_01, 1);
  BOOST_CHECK_CLOSE(10.0f, cost_02, 1);

  BOOST_CHECK_EQUAL(EstimateWaitTimeCost(0, 1, cost_map),
                    EstimateWaitTimeCost(1, 0, cost_map));
  BOOST_CHECK_EQUAL(EstimateWaitTimeCost(0, 2, cost_map),
                    EstimateWaitTimeCost(2, 0, cost_map));
}

BOOST_AUTO_TEST_CASE(WhenPopulationDensityIsFixed_ThenCheckCostMapForTopology) {
  // Structure:
  // 1----
  // |   |
  // 0---2-------3
  unsigned const kVertexCount = 4;
  float const kLocalPopulation = 126.0f;

  Topology topology(kVertexCount);
  topology[0] = VertexProperties(/*location=*/Eigen::Vector3f(0, 0, 0),
                                 /*local_population=*/kLocalPopulation,
                                 /*importance=*/1.0f / kVertexCount);
  topology[1] = VertexProperties(/*location=*/Eigen::Vector3f(0, 1000, 0),
                                 /*local_population=*/kLocalPopulation,
                                 /*importance=*/1.0f / kVertexCount);
  topology[2] = VertexProperties(/*location=*/Eigen::Vector3f(1000, 0, 0),
                                 /*local_population=*/kLocalPopulation,
                                 /*importance=*/1.0f / kVertexCount);
  topology[3] = VertexProperties(/*location=*/Eigen::Vector3f(3000, 0, 0),
                                 /*local_population=*/kLocalPopulation,
                                 /*importance=*/1.0f / kVertexCount);

  boost::add_edge(0, 1, topology);
  boost::add_edge(0, 2, topology);
  boost::add_edge(1, 2, topology);
  boost::add_edge(2, 3, topology);

  CostMap cost_map = CreateCostMapForTopology(topology);

  float cost_01 = boost::get(boost::edge_weight_t(), cost_map,
                             boost::edge(0, 1, cost_map).first);
  float cost_02 = boost::get(boost::edge_weight_t(), cost_map,
                             boost::edge(0, 2, cost_map).first);
  float cost_12 = boost::get(boost::edge_weight_t(), cost_map,
                             boost::edge(1, 2, cost_map).first);
  float cost_23 = boost::get(boost::edge_weight_t(), cost_map,
                             boost::edge(2, 3, cost_map).first);

  BOOST_CHECK_CLOSE(52, cost_01, 1);
  BOOST_CHECK_CLOSE(72, cost_02, 1);
  BOOST_CHECK_CLOSE(90, cost_12, 1);
  BOOST_CHECK_CLOSE(140, cost_23, 1);
}

BOOST_AUTO_TEST_CASE(WhenEvaluateFullObjective_ThenCheckScore) {
  Topology topology = testing::CreateGridTopology(/*side=*/5, /*scale=*/1e3f,
                                                  /*population=*/4e3);
  CostMap cost_map = CreateCostMapForTopology(topology);

  SourcePopulationSampler sampler(topology);
  float objective = EvaluateObjective(topology, cost_map, sampler);
  BOOST_CHECK_CLOSE(112, objective, 1);
}

} // namespace
} // namespace procedural
} // namespace e8
