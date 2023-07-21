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

#include "procedural/probing/topology/optimizer.hpp"
#include "procedural/probing/topology/edge_set.hpp"
#include "procedural/probing/topology/mutation.hpp"
#include "procedural/probing/topology/objective.hpp"
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/log/trivial.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <utility>

namespace e8 {
namespace procedural {
namespace {

float const kSourceSampleRatio = 0.2f;
float const kInitialMutationRatio = 0.1f;
float const kMutationScheduleFactor = -4.0f;
float const kTemperatureScheduleFactor = -1000.0f;

unsigned SourceSampleCount(Topology const &topology) {
  return std::max(1U, static_cast<unsigned>(boost::num_vertices(topology) *
                                            kSourceSampleRatio));
}

unsigned MutationOperationCountAt(unsigned i, unsigned iteration_count,
                                  CostMap const &cost_map) {
  if (i == 0) {
    return 0.2f * boost::num_edges(cost_map);
  }
  float t = static_cast<float>(i) / (iteration_count - 1);
  float mutation_ratio =
      kInitialMutationRatio * std::exp(kMutationScheduleFactor * t);
  return std::max(
      1U, static_cast<unsigned>(boost::num_edges(cost_map) * mutation_ratio));
}

bool AcceptMutation(float score, float best_score, unsigned i,
                    unsigned iteration_count,
                    std::default_random_engine *random_engine) {
  if (score >= best_score) {
    return true;
  }

  float t = static_cast<float>(i) / (iteration_count - 1);
  if (t > .5f) {
    return false;
  }
  float relative_regression = (best_score - score) / best_score;
  float p = std::exp(kTemperatureScheduleFactor * relative_regression * t);
  return std::uniform_real_distribution<float>(0, 1)(*random_engine) < p;
}

void ReportProgress(unsigned i, unsigned iteration_count, float score,
                    unsigned mutation_operation_count, unsigned edge_count) {
  unsigned last_percentage = static_cast<int>(static_cast<float>(i - 1) /
                                              (iteration_count - 1) * 10.f);
  unsigned percentage =
      static_cast<int>(static_cast<float>(i) / (iteration_count - 1) * 10.f);
  if (i != 0 && percentage == last_percentage) {
    return;
  }

  BOOST_LOG_TRIVIAL(info) << "OptimizeTopology() " << percentage * 10
                          << " % current score " << score
                          << ", mutation operation count "
                          << mutation_operation_count << ", edge count "
                          << edge_count;
}

Topology ToResultTopology(CostMap const &cost_map, Topology const &original) {
  assert(boost::num_vertices(cost_map) == boost::num_vertices(original));

  Topology result(boost::num_vertices(cost_map));
  for (unsigned i = 0; i < boost::num_vertices(original); ++i) {
    result[i] = original[i];
  }

  auto [current, end] = boost::edges(cost_map);
  for (; current != end; ++current) {
    auto [original_edge_desc, existence] =
        boost::edge(current->m_source, current->m_target, original);
    assert(existence);
    float static_cost =
        boost::get(boost::edge_weight_t(), original, original_edge_desc);

    assert(!boost::edge(current->m_source, current->m_target, result).second);
    boost::add_edge(current->m_source, current->m_target, static_cost, result);
  }

  return result;
}

} // namespace

OptimizationResult OptimizeTopology(Topology const &topology,
                                    unsigned iteration_count,
                                    std::default_random_engine *random_engine) {
  CostMap cost_map = CreateCostMapForTopology(topology);
  EdgeSetState edge_set_state = CreateEdgeSetStateFor(cost_map, random_engine);
  SourcePopulationSampler source_population(topology);

  CostMap best_cost_map = cost_map;
  float best_score = EvaluateObjective(topology, cost_map, source_population);

  for (unsigned i = 0; i < iteration_count; ++i) {
    unsigned operation_count =
        MutationOperationCountAt(i, iteration_count, cost_map);
    float prob_add = i == 0 ? .0f : .5f;
    Mutation mutation = edge_set_state.Mutate(operation_count, prob_add);
    RevertibleMutation revertible(std::move(mutation), cost_map);
    ApplyMutation(revertible, topology, &cost_map);

    float score = EvaluateObjective(topology, cost_map, source_population);
    ReportProgress(i, iteration_count, score, operation_count,
                   boost::num_edges(cost_map));

    if (!AcceptMutation(score, best_score, i, iteration_count, random_engine)) {
      RevertMutation(revertible, &cost_map);
      edge_set_state.Revert();
      continue;
    }

    if (score <= best_score) {
      continue;
    }

    best_cost_map = cost_map;
    best_score = score;
  }

  return OptimizationResult{
      .topology = ToResultTopology(best_cost_map, topology),
      .score = EvaluateObjective(topology, best_cost_map, source_population),
  };
}

} // namespace procedural
} // namespace e8
