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

#include "procedural/probing/flow/topology.hpp"
#include "procedural/probing/probe/probe.hpp"
#include <boost/graph/adjacency_list.hpp>

namespace e8 {
namespace procedural {

// For storing the time cost required to go from one probe to the other.
using FlowCost = boost::property<boost::edge_weight_t, float>;

// For storing the time cost amongst the connections.
using FlowCostMap = boost::adjacency_list<
    /*OutEdgeListS=*/boost::vecS, /*VertexListS=*/boost::vecS,
    /*DirectedS=*/boost::directedS, /*VertexProperty=*/boost::no_property,
    /*EdgeProperty=*/FlowCost>;

// Computes the time costs based on the specified flow measurement.
FlowCostMap CreateFlowCostMapFrom(TopologyFlow const &flow,
                                  std::vector<PopulationProbe> const &probes);

} // namespace procedural
} // namespace e8
