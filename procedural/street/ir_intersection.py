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

from dataclasses import dataclass
from numpy import arctan2
from sympy import Circle
from typing import Dict
from typing import List

from intermediate_representation.intersection_pb2 import Intersection
from intermediate_representation.intersection_pb2 import StandardIntersection
from intermediate_representation.intersection_pb2 import TrafficConnection
from intermediate_representation.space_pb2 import Point3
from intermediate_representation.traffic_way_pb2 import TrafficWay
from procedural.probing.population import PopulationProbe
from procedural.probing.topology import ProbeConnection


@dataclass
class _TrafficWayIO:
    inbound: TrafficWay
    outbound: TrafficWay


def _ComputeTrafficWayIOs(
    probes: List[PopulationProbe],
    traffic_ways: Dict[ProbeConnection, TrafficWay]) -> \
        List[List[_TrafficWayIO]]:
    # IOs are a collection of _TrafficWayIOs keyed by their external probe
    # index. These traffic ways, either inbound or outbound, are connected to
    # a common probe.
    probe_to_ios: List[Dict[int, _TrafficWayIO]] = list()
    for _ in range(len(probes)):
        probe_to_ios.append(dict())

    for connection, traffic_way in traffic_ways.items():
        if connection.dst_probe_index not in \
                probe_to_ios[connection.src_probe_index]:
            probe_to_ios[connection.src_probe_index][
                connection.dst_probe_index] = _TrafficWayIO(
                inbound=None,
                outbound=None)
        if connection.src_probe_index not in \
                probe_to_ios[connection.dst_probe_index]:
            probe_to_ios[connection.dst_probe_index][
                connection.src_probe_index] = _TrafficWayIO(
                inbound=None,
                outbound=None)

        probe_to_ios[connection.src_probe_index][
            connection.dst_probe_index].outbound = traffic_way
        probe_to_ios[connection.dst_probe_index][
            connection.src_probe_index].inbound = traffic_way

    # Sorts the _TrafficWayIO in counter-clockwise order.
    result: List[List[_TrafficWayIO]] = list()
    for i in range(len(probes)):
        theta_and_ios = list()

        for external_probe_index, traffic_way_io in probe_to_ios[i].items():
            dir = probes[external_probe_index].location - probes[i].location
            theta = arctan2(dir[1], dir[0])
            theta_and_ios.append((theta, traffic_way_io))

        theta_and_ios = sorted(theta_and_ios, key=lambda t: t[0])

        ios = list()
        for _, io in theta_and_ios:
            ios.append(io)

        result.append(ios)

    return result


def _CollectOutbounds(inbound_idx,
                      traffic_ios: List[_TrafficWayIO]) -> List[TrafficWay]:
    result = list()

    for k in range(1, len(traffic_ios)):
        idx = (inbound_idx + len(traffic_ios) - k) % len(traffic_ios)
        if traffic_ios[idx].outbound is not None:
            result.append(traffic_ios[idx].outbound)

    return result


def _AssignConnectivities(traffic_ios: List[_TrafficWayIO],
                          intersection: Intersection) -> None:
    for j in range(len(traffic_ios)):
        if traffic_ios[j].inbound is None:
            continue

        exit_lane_count = len(traffic_ios[j].inbound.exit_lane_indices)
        assert exit_lane_count > 0

        if traffic_ios[j].outbound is not None:
            # U turn.
            intersection.standard.connections.append(TrafficConnection(
                inbound_traffic_way_id=traffic_ios[j].inbound.id,
                inbound_lane_number=0,
                outbound_traffic_way_id=traffic_ios[j].outbound.id))

        outbounds = _CollectOutbounds(j, traffic_ios)
        if len(outbounds) == 0:
            # j is a Deadend.
            continue

        # Loop over lane or outbound assignments, depending which one is
        # greater.
        outbound_increment = None
        if len(outbounds) >= exit_lane_count:
            outbound_increment = 1.0
        else:
            assert exit_lane_count > 1
            outbound_increment = (len(outbounds) - 1)/(exit_lane_count - 1)

        lane_increment = None
        if exit_lane_count >= len(outbounds):
            lane_increment = 1.0
        else:
            assert len(outbounds) > 1
            lane_increment = (exit_lane_count - 1)/(len(outbounds) - 1)

        num_assignments = max(exit_lane_count, len(outbounds))

        for k in range(num_assignments):
            inbound_lane_number = int(k*lane_increment + 0.5)
            outbound = outbounds[int(k*outbound_increment + 0.5)]

            intersection.standard.connections.append(TrafficConnection(
                inbound_traffic_way_id=traffic_ios[j].inbound.id,
                inbound_lane_number=inbound_lane_number,
                outbound_traffic_way_id=outbound.id))


def GenerateIntersections(
        probes: List[PopulationProbe],
        intersection_areas: List[Circle],
        traffic_ways: Dict[ProbeConnection, TrafficWay]) -> List[Intersection]:
    """Generates intersection intermediate representations based on the
    geometry of the intersections and traffic ways.

    Args:
        probes (List[PopulationProbe]): The population probes connected by the
            traffic ways.
        intersection_areas (List[Circle]): The 2D (projected) approximate
            geometry of intersection at each population probe.
        traffic_ways (Dict[ProbeConnection, TrafficWay]):  dictionary mapping
            probe connections (pairs of probes) to their corresponding traffic
            way intermediate representations.

    Returns:
        List[Intersection]: A list of intersection intermediate
            representations with the same size as the population probes.
    """
    result: List[Intersection] = list()

    # 1. Assigns the center of the intersections.
    for probe in probes:
        intersection = StandardIntersection(
            center=Point3(x=probe.location[0],
                          y=probe.location[1],
                          z=probe.location[2]))
        result.append(Intersection(standard=intersection))

    # TODO: 2. Assigns street cuts.
    probe_to_traffic_ios = _ComputeTrafficWayIOs(probes, traffic_ways)

    # TODO: 3. Assigns boundaries.

    # 4. Assigns connectivities.
    for i in range(len(probes)):
        _AssignConnectivities(probe_to_traffic_ios[i], result[i])

    return result
