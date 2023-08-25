"""
 e8City
 Copyright (C) 2023 e8yes

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 """

import e8citydll
from procedural.probing.population import PopulationProbe
from procedural.probing.topology import ProbeConnection
from dataclasses import dataclass
from matplotlib.figure import Figure
from typing import List


def _ToInternalProbes(
        probes: List[PopulationProbe]) -> List[e8citydll.PopulationProbe]:
    internal_probes = list()
    for probe in probes:
        internal_probes.append(e8citydll.PopulationProbe(
            probe.location, probe.population_grid_200))
    return internal_probes


def _ToInternalConnections(
        connections: List[ProbeConnection]) -> List[e8citydll.ProbeConnection]:
    internal_connections = list()
    for connection in connections:
        internal_connections.append(e8citydll.ProbeConnection(
            connection.src_probe_index, connection.dst_probe_index))
    return internal_connections


@dataclass
class ProbeConnectionFlow:
    """Represents the measured flow on a directed connection.
    """

    # Indices to the probe array specifying a directed connection.
    src_probe_index: int
    dst_probe_index: int

    # The total number of people transported through the connection.
    flow: float

    # The suitable number of lanes for the flow.
    lane_count: int


def _ToProbeConnectionFlow(
    internal_connection_flows: e8citydll.ProbeTopologyResult) -> \
        List[ProbeConnectionFlow]:
    connection_flows = list()
    for internal_connection_flow in internal_connection_flows:
        connection_flows.append(ProbeConnectionFlow(
            src_probe_index=internal_connection_flow.src_probe_index,
            dst_probe_index=internal_connection_flow.dst_probe_index,
            flow=internal_connection_flow.flow,
            lane_count=internal_connection_flow.lane_count))

    return connection_flows


def EstimateProbeTopologyFlow(
        probes: List[PopulationProbe],
        connections: List[ProbeConnection],
        iteration_count: int) -> List[ProbeConnectionFlow]:
    """It estimates the directed transportation flow over the connections. It
    uses an iterative algorithm to simulate and measure the flow. The estimate
    converges as more iterations are run.

    Args:
        probes (List[PopulationProbe]): _description_
        connections (List[ProbeConnection]): _description_
        iteration_count (int): _description_

    Returns:
        List[ProbeConnectionFlow]: _description_
    """
    internal_probes = _ToInternalProbes(probes)
    internal_connections = _ToInternalConnections(connections)
    internal_connection_flows = e8citydll.EstimateProbeTopologyFlow(
        internal_probes, internal_connections, iteration_count)
    return _ToProbeConnectionFlow(internal_connection_flows)


def VisualizeProbeTopologyFlow(probes: List[PopulationProbe],
                               flows: List[ProbeConnectionFlow],
                               figure: Figure,
                               axis: int) -> None:
    """_summary_

    Args:
        probes (List[PopulationProbe]): _description_
        flows (List[ProbeConnectionFlow]): _description_
        figure (Figure): _description_
        axis (int): _description_
    """
    for flow in flows:
        figure.axes[axis].plot(
            [probes[flow.src_probe_index].location[0],
             probes[flow.dst_probe_index].location[0]],
            [probes[flow.src_probe_index].location[1],
             probes[flow.dst_probe_index].location[1]],
            color="grey", linewidth=0.4*flow.lane_count)
