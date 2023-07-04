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

#include "procedural/probing/topology_definition.hpp"
#include "procedural/probing/topology_sampler.hpp"
#include <random>
#include <vector>

namespace e8 {
namespace procedural {
namespace probing {

// Estimates the number of seconds needed to travel from u to v, or in the
// opposite direction, based on the size of the local population at u and v.
// This estimate assumes there is a direct connection between u and v, and is
// invariant to topological change.
float EstimateTravelTimeCost(unsigned u, unsigned v, Topology const &topology);

// Estimates the number of seconds spent in crossing the intersections at u and
// v. It changes along with any topological modification.
float EstimateWaitTimeCost(unsigned u, unsigned v, CostMap const &cost_map);

// The full objective is computed as follow,
// \frac{1}{|V|} \sum_{s,t \in V} P_r(T_{min}(s,t)) C_{area}(s)*C_{local}(t),
// where
// V:               The set of vertices in the topology map.
// s:               A source vertex.
// t:               A target vertex.
// T_{min}(s,t):    The minimum number of seconds to go from vertex s to vertex
//                  t. This function is symmetric.
// P_r(time):       Estimates the proportion of the population is willing to
//                  spend the specified amount of time in commute.
// C_{area}(s):     The area population count at vertex s.
// C_{local}(t):    The local population count at vertex t.
//
// The time complexity of the objective is O(|V||E|log(|E|)), where E is the
// number of edges in the topology. Sometimes, it could be cost prohibitive to
// compute the full objective. The source_sampler allows this function to
// evaluate on samples of sources, bring the complexity down to
// O(|S||E|log(|E|)), where S is the sample set. Note, the sampled objective is
// an unbiased estimate of the full version.
float EvaluateObjective(Topology const &topology, CostMap const &cost_map,
                        SourceSamplerInterface const &source_sampler);

} // namespace probing
} // namespace procedural
} // namespace e8
