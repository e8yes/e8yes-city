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
from matplotlib.figure import Figure
from procedural.probing.population import PopulationProbe
from dataclasses import dataclass
from typing import List


@dataclass(eq=True, frozen=True)
class ProbeConnection:
    """In the order of the specified probe array, this indicate two probes at
    the following indices should be connected bidirectionally.
    """
    src_probe_index: int
    dst_probe_index: int


@dataclass(eq=True, frozen=True)
class ProbeTopology:
    """Returns by ComputePopulationProbeTopology().
    """

    # The unique connections between the population probes.
    connections: List[ProbeConnection]

    # The objective score of the connection. It estimates the average number
    # of residents transported at the probes.
    score: float


def _ToInternal(
        probes: List[PopulationProbe]) -> List[e8citydll.PopulationProbe]:
    internal_probes = list()
    for probe in probes:
        internal_probes.append(e8citydll.PopulationProbe(
            probe.location, probe.population_grid_200))
    return internal_probes


def _ToProbeTopology(
        internal_result: e8citydll.ProbeTopologyResult) -> ProbeTopology:
    connections = list()
    for connection in internal_result.connections:
        connections.append(ProbeConnection(
            connection.src_probe_index, connection.dst_probe_index))

    return ProbeTopology(
        connections=connections, score=internal_result.score)


def ComputePopulationProbeTopology(
        probes: List[PopulationProbe],
        regularity_optimization_steps: int = 20000000,
        efficiency_optimization_steps: int = 0) -> ProbeTopology:
    """It computes the connections amongst the specified population probes
        such that the transportation between any two probes is reasonably
        efficient.

    Args:
        probes (List[PopulationProbe]): The population probes to be connected.
        regularity_optimization_steps (int, optional): The number of
            optimization steps to take to produce the regularized topology.
            Defaults to 20000000.
        efficiency_optimization_steps (int, optional): The number of
            optimization steps to take to optimize the transportation
            efficiency. Defaults to 0.

    Returns:
        ProbeTopology: See the above data class.
    """
    internal_probes = _ToInternal(probes)
    internal_result = e8citydll.ComputeProbeTopology(
        internal_probes,
        regularity_optimization_steps,
        efficiency_optimization_steps)
    return _ToProbeTopology(internal_result)


def VisualizeProbeTopology(probes: List[PopulationProbe],
                           probe_topology: ProbeTopology,
                           figure: Figure,
                           axis: int) -> None:
    """Plots the ground projected probe topology onto a matplotlib figure.

    Args:
        probes (List[PopulationProbe]): Probes which the topology refers to.
        probe_topology (ProbeTopology): The topology to be visualized.
        figure (Figure): The matplotlib figure to be drawn onto.
        axis (int): The axis of the figure to be drawn onto.
    """
    for connection in probe_topology.connections:
        figure.axes[axis].plot(
            [probes[connection.src_probe_index].location[0],
             probes[connection.dst_probe_index].location[0]],
            [probes[connection.src_probe_index].location[1],
             probes[connection.dst_probe_index].location[1]],
            color="grey", linewidth=0.5)
