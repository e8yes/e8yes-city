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

#include "procedural/probing/topology/objective_regularity.hpp"
#include "procedural/probing/topology/definition.hpp"
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <eigen3/Eigen/Core>
#include <limits>
#include <vector>

namespace e8 {
namespace procedural {
namespace {

RegularityScore MinimumDissimiarlity(unsigned u, Topology const &topology) {
  auto [begin, end] = boost::adjacent_vertices(u, topology);
  std::vector<Eigen::Vector3f> streets(boost::degree(u, topology));
  Eigen::Vector3f const &origin = topology[u].location;
  std::transform(begin, end, streets.begin(), [&origin, &topology](unsigned v) {
    return (topology[v].location - origin).normalized();
  });

  RegularityScore min_dissim = std::numeric_limits<RegularityScore>::max();
  for (unsigned i = 0; i < streets.size(); ++i) {
    for (unsigned j = i + 1; j < streets.size(); ++j) {
      min_dissim = std::min(min_dissim, -streets[i].dot(streets[j]));
    }
  }

  return min_dissim;
}

RegularityScore
RegularityObjectiveAt2WayIntersection(unsigned u, Topology const &topology) {
  return 0.8f * MinimumDissimiarlity(u, topology);
}

RegularityScore
RegularityObjectiveAt3WayIntersection(unsigned u, Topology const &topology) {
  return 0.4f + MinimumDissimiarlity(u, topology);
}

RegularityScore
RegularityObjectiveAt4WayIntersection(unsigned u, Topology const &topology) {
  return 1.0f + MinimumDissimiarlity(u, topology);
}

} // namespace

RegularityScore RegularityObjectiveAt(unsigned u, Topology const &topology) {
  switch (boost::degree(u, topology)) {
  case 0:
  case 1:
    return -1.0f;
  case 2:
    return RegularityObjectiveAt2WayIntersection(u, topology);
  case 3:
    return RegularityObjectiveAt3WayIntersection(u, topology);
  case 4:
    return RegularityObjectiveAt4WayIntersection(u, topology);
  default:
    return -1.2f;
  }
}

RegularityScoreMap CreateRegularityScoreMapFor(Topology const &topology) {
  RegularityScoreMap score_map(boost::num_vertices(topology));
  for (unsigned i = 0; i < score_map.size(); ++i) {
    score_map[i] = RegularityObjectiveAt(i, topology);
  }
  return score_map;
}

RegularityScore
EvaluateRegularityObjective(RegularityScoreMap const &score_map) {
  RegularityScore score = 0;
  for (RegularityScore vertex_score : score_map) {
    score += vertex_score;
  }
  return score;
}

} // namespace procedural
} // namespace e8
