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

from numpy import array
from numpy import ndarray
from numpy.linalg import norm
from sympy import Circle
from typing import Dict
from typing import List
from intermediate_representation.catmul_rom import CatmulRomArcLength2T
from intermediate_representation.catmul_rom import CatmulRomDomain
from intermediate_representation.curve_pb2 import ArcInterval
from intermediate_representation.curve_pb2 import CatmulRomCurve3
from intermediate_representation.space_pb2 import Point3
from intermediate_representation.street_pb2 import MarkedCurve
from intermediate_representation.street_pb2 import Street
from intermediate_representation.street_pb2 import TrafficLane
from intermediate_representation.street_pb2 import TrafficMarking
from intermediate_representation.street_pb2 import TrafficWay
from procedural.probing.flow import ProbeConnectionFlow
from procedural.probing.population import PopulationProbe
from procedural.probing.topology import ProbeConnection
from procedural.street.constants import LANE_WIDTH
from procedural.street.constants import SOLID_LINE_LENGTH_BEFORE_INTERSECTION
from procedural.street.curve import ComputeStreetCurves


def _CentralNormal(curve: CatmulRomCurve3) -> ndarray:
    assert len(curve.control_points) == 4

    dx = curve.control_points[2].x - curve.control_points[1].x
    dy = curve.control_points[2].y - curve.control_points[1].y
    dz = curve.control_points[2].z - curve.control_points[1].z

    v = array([dy, -dx, dz])
    return v/norm(v)


def _TranslateCurve(
        curve: CatmulRomCurve3,
        dir: ndarray,
        distance: float) -> CatmulRomCurve3:
    assert len(curve.control_points) == 4

    ps = list()
    for i in range(len(curve.control_points)):
        p = Point3(
            x=curve.control_points[i].x + distance*dir[0],
            y=curve.control_points[i].y + distance*dir[1],
            z=curve.control_points[i].z + distance*dir[2])
        ps.append(p)

    return CatmulRomCurve3(control_points=ps)


def _StandardMarkings(
        curve: CatmulRomCurve3,
        curve_index: int,
        lane_count: int) -> List[TrafficMarking]:
    t1, t2 = CatmulRomDomain(curve=curve)

    if curve_index == 0:
        return [
            TrafficMarking(
                arc_interval=ArcInterval(start=t1, stop=t2),
                marking_type=TrafficMarking.SOLID_YELLOW)
        ]
    elif curve_index == lane_count:
        return [
            TrafficMarking(
                arc_interval=ArcInterval(start=t1, stop=t2),
                marking_type=TrafficMarking.SOLID_WHITE)
        ]
    else:
        t_dash_start = CatmulRomArcLength2T(
            curve=curve, s=SOLID_LINE_LENGTH_BEFORE_INTERSECTION)
        t_dash_end = t2 - (t_dash_start - t1)

        return [
            TrafficMarking(
                arc_interval=ArcInterval(start=t1, stop=t_dash_start),
                marking_type=TrafficMarking.SOLID_WHITE),
            TrafficMarking(
                arc_interval=ArcInterval(start=t_dash_start, stop=t_dash_end),
                marking_type=TrafficMarking.DASHED_WHITE),
            TrafficMarking(
                arc_interval=ArcInterval(start=t_dash_end, stop=t2),
                marking_type=TrafficMarking.SOLID_WHITE),
        ]


def _GenerateStandardMarkedCurves(
        curve: CatmulRomCurve3, lane_count: int) -> List[MarkedCurve]:
    result = list()

    central_normal = _CentralNormal(curve)
    for i in range(lane_count + 1):
        curve = _TranslateCurve(curve=curve,
                                dir=central_normal,
                                distance=i*LANE_WIDTH)
        markings = _StandardMarkings(
            curve=curve, curve_index=i, lane_count=lane_count)

        marked_curve = MarkedCurve(curve=curve, markings=markings)
        result.append(marked_curve)

    return result


def _GenerateStandardRootLanes(
        lane_count: int,
        standard_marked_curves: List[MarkedCurve]) -> List[TrafficLane]:
    root_lanes = list()

    for i in range(lane_count):
        lane = TrafficLane(
            left_curve_index=i,
            right_curve_index=i + 1)
        root_lanes.append(lane)

    for i in range(lane_count):
        if i > 0:
            assert len(standard_marked_curves[i].markings) == 3
            assert standard_marked_curves[i].markings[1].marking_type ==    \
                TrafficMarking.DASHED_WHITE

            window = standard_marked_curves[i].markings[1].arc_interval
            root_lanes[i].left_lane_change_windows.append(window)
            root_lanes[i].left_adjacent_lanes.append(root_lanes[i - 1])

        if i < lane_count - 1:
            assert len(standard_marked_curves[i + 1].markings) == 3
            assert standard_marked_curves[i + 1].markings[1].marking_type == \
                TrafficMarking.DASHED_WHITE

            window = standard_marked_curves[i + 1].markings[1].arc_interval
            root_lanes[i].right_lane_change_windows.append(window)
            root_lanes[i].right_adjacent_lanes.append(root_lanes[i + 1])

    return root_lanes


def _GenerateTrafficWay(curve: CatmulRomCurve3,
                        lane_count: int,
                        flow_percentile: float) -> TrafficWay:
    standard_marked_curves = _GenerateStandardMarkedCurves(
        curve=curve, lane_count=lane_count)
    root_lanes = _GenerateStandardRootLanes(
        lane_count=lane_count, standard_marked_curves=standard_marked_curves)

    return TrafficWay(
        marked_curves=standard_marked_curves,
        root_lanes=root_lanes,
        flow_percentile=flow_percentile)


def GenerateStreets(probes: List[PopulationProbe],
                    intersection_areas: List[Circle],
                    connection_flows: List[ProbeConnectionFlow]) -> \
        Dict[ProbeConnection, Street]:
    """Generates the street intermediate representation based on the geometry
    of the intersection at each probe together with connection flow.

    Args:
        probes (List[PopulationProbe]): A list of population probes connected
            by the flows.
        intersection_areas (List[Circle]): The approximate geometry of the
            intersection at each probe.
        connection_flows (List[ProbeConnectionFlow]): A list of traffic flows
            between pairs of probes.

    Returns:
        Dict[ProbeConnection, Street]: A dictionary mapping probe connections
            (pairs of probes) to their corresponding street intermediate
            representations.
    """
    curves = ComputeStreetCurves(
        probes=probes,
        intersection_areas=intersection_areas,
        connection_flows=connection_flows)

    result: Dict[ProbeConnection, Street] = dict()
    for connection_flow, curve in curves.items():
        traffic_way = _GenerateTrafficWay(
            curve=curve,
            lane_count=connection_flow.lane_count,
            flow_percentile=connection_flow.flow)

        reverse_conn = ProbeConnection(
            src_probe_index=connection_flow.dst_probe_index,
            dst_probe_index=connection_flow.src_probe_index)
        if reverse_conn not in result:
            # First encounter.
            conn = ProbeConnection(
                src_probe_index=connection_flow.src_probe_index,
                dst_probe_index=connection_flow.dst_probe_index)
            street = Street(
                center_curve=curve,
                traffic_ways=[traffic_way])

            result[conn] = street
        else:
            # Second encounter in the reverse direction.
            result[conn].traffic_ways.append(traffic_way)

    return result
