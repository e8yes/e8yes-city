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

// Creates a cost map from the specified topology. The static edge cost of the topology needs not be initialized.
CostMap CreateCostMapForTopology(Topology const &topology);

// The full objective is computed as follow,
// L_{topology} = \frac{1}{|V|} \sum_{s,t \in V} C(s) f_X(t)
// p_r(travel|T_{min}(s,t))
//
// V:                   The set of vertices in the topology map.
// s:                   A source vertex.
// t:                   A target vertex.
// T_{min}(s,t):        The minimum number of seconds to go from vertex s to
//                      vertex t. This function is symmetric.
// P_r(travel|time):    Estimates the proportion of the population is willing to
//                      spend the specified amount of time in commute.
// f_X(t):              The importance of vertex t. Note, \sum_{t \in V} f_X(t) = 1.
// C(s):                The local population count at vertex s.
//
// The objective measures the average number of resident transported. The higher
// the number gets, the better the topology is. The time complexity of the
// objective is O(|V||E|log(|E|)), where E is the number of edges in the
// topology. Sometimes, it could be cost prohibitive to compute the full
// objective. The source_sampler allows this function to be evaluated on samples
// of sources, bringing the complexity down to O(|S||E|log(|E|)), where S is the
// sample set. Note, the sampled objective is an unbiased estimate, therefore
// the values amongst different sample sets can be compared.
float EvaluateObjective(Topology const &topology, CostMap const &cost_map,
                        SourceSamplerInterface const &source_sampler);

} // namespace probing
} // namespace procedural
} // namespace e8
