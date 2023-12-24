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
from numpy import allclose
from numpy import arctan2
# from numpy import array
from numpy import ndarray
# from sympy import Line2D
from sympy import Point2D
from sympy import Circle
from typing import List

from procedural.probing.flow import ProbeConnectionFlow
from procedural.probing.population import PopulationProbe
from procedural.street.constants import INTERSECTION_AREA_SCALING_FACTOR
from procedural.street.constants import LANE_WIDTH


@dataclass
class _TrafficWay:
    dir: ndarray
    lane_count: int


def _CollectIntersectingTrafficWays(
    probes: List[PopulationProbe],
    connection_flows: List[ProbeConnectionFlow]) -> \
        List[List[_TrafficWay]]:
    result: List[List[_TrafficWay]] = list()

    for _ in range(len(probes)):
        result.append(list())

    for flow in connection_flows:
        forward = probes[flow.dst_probe_index].location -    \
            probes[flow.src_probe_index].location
        backward = -forward

        result[flow.src_probe_index].append(
            _TrafficWay(dir=forward, lane_count=flow.lane_count))
        result[flow.dst_probe_index].append(
            _TrafficWay(dir=backward, lane_count=flow.lane_count))

    return result


@dataclass
class _Street:
    dir: ndarray
    lane_count: int


def _FindTrafficWayPair(src: _TrafficWay,
                        start: int,
                        traffic_ways: List[_TrafficWay]) -> int:
    for i in range(start, len(traffic_ways)):
        if allclose(src.dir, traffic_ways[i].dir, rtol=1e-3):
            return i
    return None


def _ToOrderedStreets(traffic_ways: List[_TrafficWay]) -> List[_Street]:
    # Merge traffic ways into streets.
    streets = list()

    i = 0
    while i < len(traffic_ways):
        j = _FindTrafficWayPair(traffic_ways[i], i + 1, traffic_ways)
        if j is None:
            street = _Street(
                dir=traffic_ways[i].dir, lane_count=traffic_ways[i].lane_count)
            streets.append(street)

            i += 1
        else:
            total_lane_count = traffic_ways[i].lane_count +     \
                traffic_ways[j].lane_count
            street = _Street(
                dir=traffic_ways[i].dir,
                lane_count=total_lane_count)
            streets.append(street)

            traffic_ways[i + 1], traffic_ways[j] =              \
                traffic_ways[j], traffic_ways[i + 1]

            i += 2

    # Ordered streets by their polar angle (2D ground projection).
    return sorted(
        streets,
        key=lambda street: arctan2(street.dir[0], street.dir[1]))


def _IntersectionRadius(streets: List[_Street]) -> float:
    max_lane_count = max(
        streets, key=lambda street: street.lane_count).lane_count
    return max_lane_count*LANE_WIDTH/2


def _ComputeIntersectionArea(center: PopulationProbe,
                             ordered_street: List[_Street]) -> Circle:
    radius = _IntersectionRadius(ordered_street)
    return Circle(Point2D((center.location[0],
                           center.location[1])),
                  INTERSECTION_AREA_SCALING_FACTOR*radius)


def ComputeIntersectionAreas(
        probes: List[PopulationProbe],
        connection_flows: List[ProbeConnectionFlow]) -> List[Circle]:
    """Compute an approximate geometry of the intersection at each probe.

    Args:
        probes (List[PopulationProbe]): A list of population probes connected
            by the flows.
        connection_flows (List[ProbeConnectionFlow]): A list of traffic flows
            between pairs of probes.

    Returns:
        List[Circle]: The approximate geometry of the intersection at each
            probe.
    """
    intersecting_traffic_ways = _CollectIntersectingTrafficWays(
        probes, connection_flows)

    result = list()
    for i in range(len(probes)):
        ordered_streets = _ToOrderedStreets(intersecting_traffic_ways[i])
        area = _ComputeIntersectionArea(probes[i], ordered_streets)
        result.append(area)

    return result
