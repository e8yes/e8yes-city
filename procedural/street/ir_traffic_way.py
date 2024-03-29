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
from typing import Dict
from typing import List

from intermediate_representation.catmul_rom import CatmulRomArcLength2T
from intermediate_representation.catmul_rom import CatmulRomDomain
from intermediate_representation.curve_pb2 import ArcInterval
from intermediate_representation.curve_pb2 import CatmulRomCurve3
from intermediate_representation.space_pb2 import Point3
from intermediate_representation.traffic_way_pb2 import MarkedCurve
from intermediate_representation.traffic_way_pb2 import TrafficLane
from intermediate_representation.traffic_way_pb2 import TrafficMarking
from intermediate_representation.traffic_way_pb2 import TrafficWay
from procedural.probing.flow import ProbeConnectionFlow
from procedural.probing.topology import ProbeConnection
from procedural.street.constants import LANE_WIDTH
from procedural.street.constants import SOLID_LINE_LENGTH_BEFORE_INTERSECTION


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


def _GenerateStandardEntranceLanes(
        lane_count: int,
        standard_marked_curves: List[MarkedCurve]) -> List[TrafficLane]:
    assert lane_count > 0
    assert len(standard_marked_curves) == lane_count + 1

    result = list()

    for i in range(lane_count):
        lane = TrafficLane(
            left_curve_index=i,
            right_curve_index=i + 1)
        result.append(lane)

    for i in range(lane_count):
        if i > 0:
            assert len(standard_marked_curves[i].markings) == 3
            assert standard_marked_curves[i].markings[1].marking_type ==    \
                TrafficMarking.DASHED_WHITE

            window = standard_marked_curves[i].markings[1].arc_interval
            result[i].left_lane_change_windows.append(window)
            result[i].left_adjacent_lane_indices.append(i - 1)

        if i < lane_count - 1:
            assert len(standard_marked_curves[i + 1].markings) == 3
            assert standard_marked_curves[i + 1].markings[1].marking_type == \
                TrafficMarking.DASHED_WHITE

            window = standard_marked_curves[i + 1].markings[1].arc_interval
            result[i].right_lane_change_windows.append(window)
            result[i].right_adjacent_lane_indices.append(i + 1)

    return result


def _GenerateTrafficWay(src_probe_index: int,
                        dst_probe_index: int,
                        curve: CatmulRomCurve3,
                        lane_count: int,
                        flow_percentile: float) -> TrafficWay:
    standard_marked_curves = _GenerateStandardMarkedCurves(
        curve=curve, lane_count=lane_count)
    entrance_lanes = _GenerateStandardEntranceLanes(
        lane_count=lane_count, standard_marked_curves=standard_marked_curves)

    return TrafficWay(
        id=f"{src_probe_index}_{dst_probe_index}",
        leftmost_curve=curve,
        marked_curves=standard_marked_curves,
        lanes=entrance_lanes,
        entrance_lane_indices=[i for i in range(len(entrance_lanes))],
        exit_lane_indices=[i for i in range(len(entrance_lanes))],
        flow_percentile=flow_percentile)


def GenerateTrafficWays(
    street_curves: Dict[ProbeConnectionFlow, CatmulRomCurve3]) -> \
        Dict[ProbeConnection, TrafficWay]:
    """Generates the traffic way intermediate representation based on the
    geometry of the street center curves together with the flow statistics.

    Args:
        street_curves (Dict[ProbeConnectionFlow, CatmulRomCurve3]): The flow
            statistics and the geometry of the street center curves.

    Returns:
        Dict[ProbeConnection, Street]: A dictionary mapping probe connections
            (pairs of probes) to their corresponding traffic way intermediate
            representations.
    """
    result: Dict[ProbeConnection, TrafficWay] = dict()
    for connection_flow, curve in street_curves.items():
        traffic_way = _GenerateTrafficWay(
            connection_flow.src_probe_index,
            connection_flow.dst_probe_index,
            curve=curve,
            lane_count=connection_flow.lane_count,
            flow_percentile=connection_flow.flow)

        conn = ProbeConnection(
            src_probe_index=connection_flow.src_probe_index,
            dst_probe_index=connection_flow.dst_probe_index)
        result[conn] = traffic_way

    return result
