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

#pragma once

#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map/property_map.hpp>
#include <eigen3/Eigen/Core>

namespace e8 {
namespace procedural {

// Defines the vertex properties (a variant of population probe) of a topology
// map.
struct VertexProperties {
  VertexProperties() = default;

  VertexProperties(Eigen::Vector3f const &location, float local_population,
                   float area_population, float importance)
      : location(location), local_population(local_population),
        area_population(area_population), importance(importance) {}

  // The spatial location of the vertex.
  Eigen::Vector3f location;

  // The number of residents around the immediate area centered at the above
  // location.
  float local_population;

  // The representative number of residents around the larger area centered at
  // the above location.
  float area_population;

  // The normalized importance of the vertex. In particular, normalization
  // means: \sum_{v \in V} importance(v) = 1.
  float importance;
};

// For storing the part of edge cost which is invariant to topological change.
using StaticEdgeCost = boost::property<boost::edge_weight_t, float>;

// Represents the topology of the population probes.
using Topology = boost::adjacency_list<
    /*OutEdgeListS=*/boost::vecS, /*VertexListS=*/boost::vecS,
    /*DirectedS=*/boost::undirectedS, /*VertexProperty=*/VertexProperties,
    /*EdgeProperty=*/StaticEdgeCost>;

// For storing the total edge cost.
using EdgeCost = boost::property<boost::edge_weight_t, float>;

// Stores the total transportation cost of any two adjacent population probes.
using CostMap = boost::adjacency_list<
    /*OutEdgeListS=*/boost::vecS, /*VertexListS=*/boost::vecS,
    /*DirectedS=*/boost::undirectedS, /*VertexProperty=*/boost::no_property,
    /*EdgeProperty=*/EdgeCost>;

namespace testing {

//
Topology CreateGridTopology(unsigned side, float scale, float population);

} // namespace testing
} // namespace procedural
} // namespace e8
