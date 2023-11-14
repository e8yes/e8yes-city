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

from matplotlib.figure import Figure
from numpy import ndarray
from numpy import array
from numpy import inner
from numpy import linspace
from numpy import sqrt
from numpy.linalg import norm
from sympy import Point2D
from sympy import Polygon
from sympy import Segment2D
from typing import Dict
from typing import List
from typing import Set
from intermediate_representation.curve_pb2 import CatmulRomCurve3
from intermediate_representation.space_pb2 import Point3
from procedural.probing.flow import ProbeConnectionFlow
from procedural.probing.population import PopulationProbe


def _CollectOutEdges(
        connection_flows: List[ProbeConnectionFlow]) -> Dict[int, Set[int]]:
    out_edges: Dict[int, Set[int]] = dict()

    for connection_flow in connection_flows:
        if connection_flow.src_probe_index not in out_edges:
            out_edges[connection_flow.src_probe_index] = set()

        out_edges[connection_flow.src_probe_index].add(
            connection_flow.dst_probe_index)

    return out_edges


def _Dissimilarity(src_probe_index: int,
                   dst_probe_index1: int,
                   dst_probe_index2: int,
                   probes: List[PopulationProbe]) -> float:
    v1 = probes[dst_probe_index1].location - probes[src_probe_index].location
    v2 = probes[dst_probe_index2].location - probes[src_probe_index].location
    return -inner(v1, v2)/norm(v1)/norm(v2)


def _Compute1WayExternalControlPoints(
        src_probe_index: int,
        dst_probe_indices: Set[int],
        probes: List[PopulationProbe]) -> Dict[int, ndarray]:
    out = next(iter(dst_probe_indices))

    src = probes[src_probe_index].location
    dst = probes[out].location
    dir = dst - src
    control_point = src - dir

    return {
        out: control_point
    }


def _Compute2WaysExternalControlPoints(
        dst_probe_indices: Set[int],
        probes: List[PopulationProbe]) -> Dict[int, ndarray]:
    it = iter(dst_probe_indices)
    out1 = next(it)
    out2 = next(it)
    return {
        out1: probes[out2].location,
        out2: probes[out1].location
    }


def _Compute3WaysExternalControlPoints(
        src_probe_index: int,
        dst_probe_indices: List[int],
        probes: List[PopulationProbe]) -> Dict[int, ndarray]:
    possible_pairings = [
        ((dst_probe_indices[0], dst_probe_indices[1]),
         dst_probe_indices[2]),
        ((dst_probe_indices[0], dst_probe_indices[2]),
         dst_probe_indices[1]),
        ((dst_probe_indices[1], dst_probe_indices[2]),
         dst_probe_indices[0]),
    ]

    best_pairing = None
    best_dissimlarity_score = -float("inf")
    for pa, left_over in possible_pairings:
        a, b = pa

        dissimilarity_score = _Dissimilarity(src_probe_index, a, b, probes)
        if dissimilarity_score < best_dissimlarity_score:
            continue

        best_dissimlarity_score = dissimilarity_score
        best_pairing = (pa, left_over)

    assert best_pairing is not None

    src = probes[src_probe_index].location
    dst = probes[best_pairing[1]].location
    leftover_dir = dst - src
    leftover_control_point = src - leftover_dir

    return {
        best_pairing[0][0]: probes[best_pairing[0][1]].location,
        best_pairing[0][1]: probes[best_pairing[0][0]].location,
        best_pairing[1]: leftover_control_point,
    }


def _Compute4WaysExternalControlPoints(
        src_probe_index: int,
        dst_probe_indices: List[int],
        probes: List[PopulationProbe]) -> Dict[int, ndarray]:
    possible_pairings = [
        ((dst_probe_indices[0], dst_probe_indices[1]),
         (dst_probe_indices[2], dst_probe_indices[3])),
        ((dst_probe_indices[0], dst_probe_indices[2]),
         (dst_probe_indices[1], dst_probe_indices[3])),
        ((dst_probe_indices[0], dst_probe_indices[3]),
         (dst_probe_indices[1], dst_probe_indices[2])),
    ]

    best_pairing = None
    best_dissimlarity_score = -float("inf")
    for pa, pb in possible_pairings:
        a, b = pa
        c, d = pb

        dissimilarity_score =                                   \
            _Dissimilarity(src_probe_index, a, b, probes) +     \
            _Dissimilarity(src_probe_index, c, d, probes)
        if dissimilarity_score < best_dissimlarity_score:
            continue

        best_dissimlarity_score = dissimilarity_score
        best_pairing = (pa, pb)

    assert best_pairing is not None
    return {
        best_pairing[0][0]: probes[best_pairing[0][1]].location,
        best_pairing[0][1]: probes[best_pairing[0][0]].location,
        best_pairing[1][0]: probes[best_pairing[1][1]].location,
        best_pairing[1][1]: probes[best_pairing[1][0]].location,
    }


def _ComputeNWaysExternalControlPoints(
        src_probe_index: int,
        dst_probe_indices: List[int],
        probes: List[PopulationProbe]) -> Dict[int, ndarray]:
    result = dict()

    for i in range(len(dst_probe_indices)):
        best_pairing = -1
        best_dissimlarity_score = -float("inf")

        for j in range(len(dst_probe_indices)):
            if i == j:
                continue

            dissimilarity_score = _Dissimilarity(src_probe_index,
                                                 i,
                                                 dst_probe_indices[j],
                                                 probes)
            if dissimilarity_score < best_dissimlarity_score:
                continue

            best_dissimlarity_score = dissimilarity_score
            best_pairing = j

        assert best_pairing is not None
        result[dst_probe_indices[i]] =                              \
            probes[dst_probe_indices[best_pairing]].location

    return result


def _ComputeExternalControlPointsFor(
        src_probe_index: int,
        dst_probe_indices: Set[int],
        probes: List[PopulationProbe]) -> Dict[int, ndarray]:
    if len(dst_probe_indices) == 0:
        return dict()
    elif len(dst_probe_indices) == 1:
        return _Compute1WayExternalControlPoints(
            src_probe_index, dst_probe_indices, probes)
    elif len(dst_probe_indices) == 2:
        return _Compute2WaysExternalControlPoints(dst_probe_indices, probes)
    elif len(dst_probe_indices) == 3:
        return _Compute3WaysExternalControlPoints(
            src_probe_index, list(dst_probe_indices), probes)
    elif len(dst_probe_indices) == 4:
        return _Compute4WaysExternalControlPoints(
            src_probe_index, list(dst_probe_indices), probes)
    else:
        return _ComputeNWaysExternalControlPoints(
            src_probe_index, list(dst_probe_indices), probes)


def _ComputeExternalControlPoints(
        out_edges: Dict[int, Set[int]],
        probes: List[PopulationProbe]) -> Dict[int, Dict[int, ndarray]]:
    result: Dict[int, Dict[int, int]] = dict()

    for src_probe_index, dst_probe_indices in out_edges.items():
        result[src_probe_index] = _ComputeExternalControlPointsFor(
            src_probe_index, dst_probe_indices, probes)

    return result


def _ToPoint3(p: ndarray) -> Point3:
    return Point3(x=p[0], y=p[1], z=p[2])


def ComputeStreetCurves(
    probes: List[PopulationProbe],
    intersection_areas: List[Polygon],
    connection_flows: List[ProbeConnectionFlow]) -> \
        Dict[ProbeConnectionFlow, CatmulRomCurve3]:
    """Interpolates the connections amongst the probes using the Catmul-Rom
    cubic spline. Specifically, the geometry of each street depends on the
    preceding and succeeding streets, if they exist. Otherwise, virtual
    streets are prepended or appended to ensure regularity (right-angle
    intersections) of the generated curve.

    Args:
        probes (List[PopulationProbe]): The population probes connected by the
            flows.
        intersection_areas (List[Polygon]): _description_
        connection_flows (List[ProbeConnectionFlow]): Each flow is treated as
            a street, which needs to be assigned a curve.

    Returns:
        Dict[ProbeConnectionFlow, CatmulRomCurve3]: The computed curve for
            each street.
    """
    out_edges = _CollectOutEdges(connection_flows)
    external_control_points = _ComputeExternalControlPoints(out_edges, probes)

    result = dict()
    for connection_flow in connection_flows:
        pre_probe_control_point = external_control_points[
            connection_flow.src_probe_index][connection_flow.dst_probe_index]
        post_probe_control_point = external_control_points[
            connection_flow.dst_probe_index][connection_flow.src_probe_index]

        src_intersection_area = \
            intersection_areas[connection_flow.src_probe_index]
        dst_intersection_area = \
            intersection_areas[connection_flow.dst_probe_index]

        street_segment = Segment2D(
            p1=Point2D((probes[connection_flow.src_probe_index].location[0],
                        probes[connection_flow.src_probe_index].location[1])),
            p2=Point2D((probes[connection_flow.dst_probe_index].location[0],
                        probes[connection_flow.dst_probe_index].location[1])))
        src_intersection = src_intersection_area.intersection(street_segment)
        dst_intersection = dst_intersection_area.intersection(street_segment)
        assert len(src_intersection) == 1
        assert len(dst_intersection) == 1

        src_point = Point3(
            x=src_intersection[0].x.evalf(),
            y=src_intersection[0].y.evalf(),
            z=probes[connection_flow.src_probe_index].location[2])
        dst_point = Point3(
            x=dst_intersection[0].x.evalf(),
            y=dst_intersection[0].y.evalf(),
            z=probes[connection_flow.dst_probe_index].location[2])

        street_curve = CatmulRomCurve3()
        street_curve.control_points.append(_ToPoint3(pre_probe_control_point))
        street_curve.control_points.append(src_point)
        street_curve.control_points.append(dst_point)
        street_curve.control_points.append(_ToPoint3(post_probe_control_point))

        result[connection_flow] = street_curve

    return result


def _VisualizeStreetCurve(street_curve: CatmulRomCurve3,
                          figure: Figure,
                          axis: int) -> None:
    assert len(street_curve.control_points) == 4

    p0 = array([[street_curve.control_points[0].x,
                 street_curve.control_points[0].y,
                 street_curve.control_points[0].z]])
    p1 = array([[street_curve.control_points[1].x,
                street_curve.control_points[1].y,
                street_curve.control_points[1].z]])
    p2 = array([[street_curve.control_points[2].x,
                street_curve.control_points[2].y,
                street_curve.control_points[2].z]])
    p3 = array([[street_curve.control_points[3].x,
                street_curve.control_points[3].y,
                street_curve.control_points[3].z]])

    t0 = 0
    t1 = t0 + sqrt(norm(p1 - p0))
    t2 = t1 + sqrt(norm(p2 - p1))
    t3 = t2 + sqrt(norm(p3 - p2))

    t = linspace(t1, t2, num=10).\
        reshape((-1, 1)).\
        repeat([3], axis=1)

    A1 = (t1 - t) / (t1 - t0) * p0 + (t - t0) / (t1 - t0) * p1
    A2 = (t2 - t) / (t2 - t1) * p1 + (t - t1) / (t2 - t1) * p2
    A3 = (t3 - t) / (t3 - t2) * p2 + (t - t2) / (t3 - t2) * p3
    B1 = (t2 - t) / (t2 - t0) * A1 + (t - t0) / (t2 - t0) * A2
    B2 = (t3 - t) / (t3 - t1) * A2 + (t - t1) / (t3 - t1) * A3
    p = (t2 - t) / (t2 - t1) * B1 + (t - t1) / (t2 - t1) * B2

    for i in range(p.shape[0] - 1):
        figure.axes[axis].plot([p[i, 0], p[i + 1, 0]],
                               [p[i, 1], p[i + 1, 1]],
                               color="grey",
                               linewidth=0.5)


def VisualizeStreetCurves(street_curves: Dict[ProbeConnectionFlow,
                                              CatmulRomCurve3],
                          figure: Figure,
                          axis: int) -> None:
    """Plots the ground projected street curves onto a matplotlib figure.

    Args:
        street_curves (List[CatmulRomCurve3]): Curves to be visualized.
        figure (Figure): The matplotlib figure to be drawn onto.
        axis (int): The axis of the figure to be drawn onto.
    """
    for _, street_curve in street_curves.items():
        _VisualizeStreetCurve(street_curve, figure, axis)
