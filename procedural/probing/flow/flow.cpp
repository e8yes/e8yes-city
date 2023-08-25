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

#include "procedural/probing/flow/flow.hpp"
#include "procedural/probing/flow/simulate.hpp"
#include "procedural/probing/flow/topology.hpp"
#include "procedural/probing/flow/update.hpp"
#include "procedural/probing/probe/probe.hpp"
#include "procedural/probing/topology/topology.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/log/trivial.hpp>
#include <cassert>
#include <vector>

namespace e8 {
namespace procedural {
namespace {

TopologyFlow ToFlow(std::vector<PopulationProbe> const &probes,
                    std::vector<ProbeConnection> const &connections) {
  TopologyFlow flow(probes.size());
  for (auto const &connection : connections) {
    assert(connection.src_probe_index < boost::num_vertices(flow));
    assert(connection.dst_probe_index < boost::num_vertices(flow));

    FlowProperties properties(/*flow=*/0, /*lane_count=*/1);
    bool successful1 =
        boost::add_edge(connection.src_probe_index, connection.dst_probe_index,
                        properties, flow)
            .second;
    bool successful2 =
        boost::add_edge(connection.dst_probe_index, connection.src_probe_index,
                        properties, flow)
            .second;
    assert(successful1);
    assert(successful2);
  }
  return flow;
}

std::vector<ProbeConnectionFlow> ToResult(TopologyFlow const &flow) {
  std::vector<ProbeConnectionFlow> result;
  result.reserve(boost::num_edges(flow));

  for (auto [current, end] = boost::edges(flow); current != end; ++current) {
    FlowProperties properties =
        boost::get(boost::edge_bundle_t(), flow, *current);
    result.push_back(ProbeConnectionFlow(current->m_source, current->m_target,
                                         properties.flow,
                                         properties.lane_count));
  }

  return result;
}

} // namespace

std::vector<ProbeConnectionFlow>
EstimateProbeTopologyFlow(std::vector<PopulationProbe> const &probes,
                          std::vector<ProbeConnection> const &connections,
                          unsigned iteration_count) {
  TopologyFlow flow = ToFlow(probes, connections);
  for (unsigned i = 0; i < iteration_count; ++i) {
    TopologyFlow simulated_flow = SimulateFlow(flow, probes);
    FlowStatistics flow_statistics = UpdateFlow(simulated_flow, &flow);

    BOOST_LOG_TRIVIAL(info) << "EstimateProbeTopologyFlow() iteration " << i + 1
                            << ", mean(flow)=" << flow_statistics.mean
                            << ", std(flow)=" << flow_statistics.std
                            << ", min(flow)=" << flow_statistics.min
                            << ", max(flow)=" << flow_statistics.max;
  }
  return ToResult(flow);
}

} // namespace procedural
} // namespace e8
