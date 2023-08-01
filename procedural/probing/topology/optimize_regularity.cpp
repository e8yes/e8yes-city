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

#include "procedural/probing/topology/optimize_regularity.hpp"
#include "procedural/probing/topology/definition.hpp"
#include "procedural/probing/topology/edge_set.hpp"
#include "procedural/probing/topology/mutation_regularity.hpp"
#include "procedural/probing/topology/objective_regularity.hpp"
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/log/trivial.hpp>
#include <cmath>
#include <iostream>
#include <random>

namespace e8 {
namespace procedural {
namespace {

unsigned const kMutationCount = 3;

void ReportProgress(unsigned i, unsigned iteration_count, float score,
                    unsigned edge_count) {
  unsigned last_percentage = static_cast<int>(static_cast<float>(i - 1) /
                                              (iteration_count - 1) * 10.f);
  unsigned percentage =
      static_cast<int>(static_cast<float>(i) / (iteration_count - 1) * 10.f);
  if (i != 0 && percentage == last_percentage) {
    return;
  }

  BOOST_LOG_TRIVIAL(info) << "OptimizeRegularity() " << percentage * 10
                          << " % current score " << score << ", edge count "
                          << edge_count;
}

} // namespace

OptimizeRegularityResult
OptimizeRegularity(Topology const &topology, unsigned iteration_count,
                   std::default_random_engine *random_engine) {
  EdgeSetState edge_set_state = CreateEdgeSetStateFor(topology, random_engine);
  RegularityScoreMap score_map = CreateRegularityScoreMapFor(topology);

  float best_score = EvaluateRegularityObjective(score_map);
  Topology best_result = topology;

  for (unsigned i = 0; i < iteration_count; ++i) {
    unsigned operation_count = kMutationCount;
    ReportProgress(i, iteration_count,
                   best_score / boost::num_vertices(topology),
                   boost::num_edges(best_result));

    Mutation mutation = edge_set_state.Mutate(operation_count);
    RevertibleRegularityMutation revertible(std::move(mutation), score_map,
                                            best_score);
    float new_score = ApplyMutation(revertible, &best_result, &score_map);
    if (new_score >= best_score) {
      best_score = new_score;
      continue;
    }

    RevertMutation(revertible, &best_result, &score_map);
    edge_set_state.Revert();
  }

  return OptimizeRegularityResult{
      .topology = best_result,
      .score = best_score,
  };
}

} // namespace procedural
} // namespace e8
