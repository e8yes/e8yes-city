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
#include "procedural/probing/topology_objective.hpp"
#include <boost/test/unit_test.hpp>
#include <eigen3/Eigen/Core>

namespace e8 {
namespace procedural {
namespace probing {
namespace {

BOOST_AUTO_TEST_CASE(WhenPopulationIsSparse_ThenCheckTravelTime) {
  Topology topology(2);
  topology[0] =
      VertexProperties(/*location=*/Eigen::Vector3f(.0f, .0f, .0f),
                       /*local_population=*/.0f, /*area_population=*/.0f,
                       /*importance=*/0.5f);
  topology[1] =
      VertexProperties(/*location=*/Eigen::Vector3f(1000.0f, .0f, .0f),
                       /*local_population=*/.0f, /*area_population=*/.0f,
                       /*importance=*/.5f);
  BOOST_CHECK_CLOSE(25.706940874f,
                    EstimateTravelTimeCost(/*u=*/0, /*v=*/1, topology), 5.0f);
}

BOOST_AUTO_TEST_CASE(WhenPopulationIsDense_ThenCheckTravelTime) {
  Topology topology(2);
  topology[0] =
      VertexProperties(/*location=*/Eigen::Vector3f(.0f, .0f, .0f),
                       /*local_population=*/500.0f, /*area_population=*/2500.0f,
                       /*importance=*/0.5f);
  topology[1] =
      VertexProperties(/*location=*/Eigen::Vector3f(1000.0f, .0f, .0f),
                       /*local_population=*/500.0f, /*area_population=*/2500.0f,
                       /*importance=*/.5f);
  BOOST_CHECK_CLOSE(120.048019208f,
                    EstimateTravelTimeCost(/*u=*/0, /*v=*/1, topology), 5.0f);
}

BOOST_AUTO_TEST_CASE(WhenPopulationIsNormal_ThenCheckTravelTime) {
  Topology topology(2);
  topology[0] =
      VertexProperties(/*location=*/Eigen::Vector3f(.0f, .0f, .0f),
                       /*local_population=*/.0f, /*area_population=*/.0f,
                       /*importance=*/0.5f);
  topology[1] =
      VertexProperties(/*location=*/Eigen::Vector3f(1000.0f, .0f, .0f),
                       /*local_population=*/.0f, /*area_population=*/.0f,
                       /*importance=*/.5f);
  BOOST_CHECK_CLOSE(25.706940874f,
                    EstimateTravelTimeCost(/*u=*/0, /*v=*/1, topology), 5.0f);
}

} // namespace
} // namespace probing
} // namespace procedural
} // namespace e8
