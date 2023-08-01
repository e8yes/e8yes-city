import e8citydll
from procedural.probing.population import PopulationProbe
from dataclasses import dataclass
from typing import List


@dataclass
class ProbeConnection:
    """_summary_
    """

    #
    src_probe_index: int

    #
    dst_probe_index: int


@dataclass
class ProbeTopology:
    """_summary_
    """

    #
    connections: List[ProbeConnection]

    #
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
        optimization_step_count: int) -> ProbeTopology:
    """_summary_

    Args:
        probes (List[PopulationProbe]): _description_
        optimization_step_count (int): _description_

    Returns:
        ProbeTopology: _description_
    """
    internal_probes = _ToInternal(probes)
    internal_result = e8citydll.ComputeProbeTopology(
        internal_probes, optimization_step_count)
    return _ToProbeTopology(internal_result)
