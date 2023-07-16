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

#include "procedural/probing/topology/init.hpp"
#include "procedural/probing/topology/definition.hpp"
#include "procedural/probing/topology/objective.hpp"
#include "procedural/probing/topology/probe.hpp"
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <boost/graph/adjacency_list.hpp>
#include <cassert>
#include <eigen3/Eigen/Core>
#include <unordered_map>
#include <vector>

namespace e8 {
namespace procedural {
namespace {

using ConstructionKernel = CGAL::Exact_predicates_inexact_constructions_kernel;
using DelaunayTriangulation =
    CGAL::Delaunay_triangulation_2<ConstructionKernel>;
using VertexHandle = DelaunayTriangulation::Vertex_handle;
using FiniteVertexHandles = DelaunayTriangulation::Finite_vertex_handles;
using Point = DelaunayTriangulation::Point;

std::vector<Point> ToVerticecs(std::vector<PopulationProbe> const &probes) {
  std::vector<Point> points(probes.size());
  std::transform(probes.begin(), probes.end(), points.begin(),
                 [](PopulationProbe const &probe) {
                   return Point(probe.location.x(), probe.location.y());
                 });

  return points;
}

std::unordered_map<Point, unsigned>
CreateVertexToIndexMapping(std::vector<Point> const &vertices) {
  std::unordered_map<Point, unsigned> result;
  for (unsigned i = 0; i < vertices.size(); ++i) {
    result[vertices[i]] = i;
  }
  return result;
}

void SetVertexProperties(std::vector<PopulationProbe> const &probes,
                         Topology *topology) {
  float total_population = 0;
  for (auto const &probe : probes) {
    total_population += probe.population_grid_200;
  }

  for (unsigned i = 0; i < probes.size(); ++i) {
    Topology::vertex_descriptor v = boost::vertex(i, *topology);

    float importance = probes[i].population_grid_200 / total_population;
    (*topology)[v] = VertexProperties(
        probes[i].location, probes[i].population_grid_200, importance);
  }
}

void CreateDelaunayConnections(std::vector<PopulationProbe> const &probes,
                               Topology *topology) {
  std::vector<Point> vertices = ToVerticecs(probes);
  std::unordered_map<Point, unsigned> vertex_to_index =
      CreateVertexToIndexMapping(vertices);
  DelaunayTriangulation triangulation(vertices.begin(), vertices.end());

  for (auto const &edge : triangulation.finite_edges()) {
    DelaunayTriangulation::Segment const &segment = triangulation.segment(edge);
    unsigned start_index = vertex_to_index[segment.start()];
    unsigned end_index = vertex_to_index[segment.end()];

    if (!boost::edge(start_index, end_index, *topology).second) {
      boost::add_edge(start_index, end_index, *topology);
    }
  }
}

void SetEdgeCost(Topology *topology) {
  auto [current, end] = boost::edges(*topology);
  for (; current != end; ++current) {
    float edge_static_cost =
        EstimateTravelTimeCost(current->m_source, current->m_target, *topology);
    boost::put(boost::edge_weight_t(), *topology, *current, edge_static_cost);
  }
}

} // namespace

Topology CreateDelaunayTopology(std::vector<PopulationProbe> const &probes) {
  Topology topology(probes.size());
  SetVertexProperties(probes, &topology);
  CreateDelaunayConnections(probes, &topology);
  SetEdgeCost(&topology);
  return topology;
}

} // namespace procedural
} // namespace e8
