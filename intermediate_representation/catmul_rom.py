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
from numpy import clip
from numpy import linspace
from numpy import ndarray
from numpy import sqrt
from numpy.linalg import norm
from typing import Tuple
from intermediate_representation.curve_pb2 import CatmulRomCurve3


def CatmulRomDomain(curve: CatmulRomCurve3) -> Tuple[float, float]:
    """Calculates the domain of a centripetal Catmul-Rom curve. The domain
    allows evaluate of the curve from the second to the third control points.
    """
    assert len(curve.control_points) == 4

    p0 = array([[curve.control_points[0].x,
                 curve.control_points[0].y,
                 curve.control_points[0].z]])
    p1 = array([[curve.control_points[1].x,
                curve.control_points[1].y,
                curve.control_points[1].z]])
    p2 = array([[curve.control_points[2].x,
                curve.control_points[2].y,
                curve.control_points[2].z]])

    t1 = sqrt(norm(p1 - p0))
    t2 = t1 + sqrt(norm(p2 - p1))

    return t1, t2


def CatmulRomFTs(curve: CatmulRomCurve3, ts: ndarray) -> ndarray:
    """Evaluate the curve at the specified curve parameters.

    Args:
        curve (CatmulRomCurve3): The curve to be evaluated.
        ts (ndarray): The array of curve parameters to be evaluated upon.

    Returns:
        ndarray: The evaluated points on the curve, of shape (n, 3)
    """
    assert len(curve.control_points) == 4

    p0 = array([[curve.control_points[0].x,
                 curve.control_points[0].y,
                 curve.control_points[0].z]])
    p1 = array([[curve.control_points[1].x,
                curve.control_points[1].y,
                curve.control_points[1].z]])
    p2 = array([[curve.control_points[2].x,
                curve.control_points[2].y,
                curve.control_points[2].z]])
    p3 = array([[curve.control_points[3].x,
                curve.control_points[3].y,
                curve.control_points[3].z]])

    t0 = 0
    t1 = t0 + sqrt(norm(p1 - p0))
    t2 = t1 + sqrt(norm(p2 - p1))
    t3 = t2 + sqrt(norm(p3 - p2))

    ts = ts.reshape((-1, 1)).repeat([3], axis=1)

    A1 = (t1 - ts) / (t1 - t0) * p0 + (ts - t0) / (t1 - t0) * p1
    A2 = (t2 - ts) / (t2 - t1) * p1 + (ts - t1) / (t2 - t1) * p2
    A3 = (t3 - ts) / (t3 - t2) * p2 + (ts - t2) / (t3 - t2) * p3
    B1 = (t2 - ts) / (t2 - t0) * A1 + (ts - t0) / (t2 - t0) * A2
    B2 = (t3 - ts) / (t3 - t1) * A2 + (ts - t1) / (t3 - t1) * A3
    ps = (t2 - ts) / (t2 - t1) * B1 + (ts - t1) / (t2 - t1) * B2

    return ps


def CatmulRomDTs(curve: CatmulRomCurve3, ts: ndarray) -> ndarray:
    """Compute the unit tangent vectors at the specified curve parameters.

    Args:
        curve (CatmulRomCurve3): The curve to be evaluated.
        ts (ndarray): The array of curve parameters to be evaluated upon.

    Returns:
        ndarray: The evaluated unit tangent vectors on the curve, of shape
            (n, 3)
    """
    DT = 1e-3

    ps = CatmulRomFTs(curve, ts)
    p_nexts = CatmulRomFTs(curve, ts + DT)

    tangent = (p_nexts - ps)/DT
    return tangent/norm(tangent)


def CatmulRomT2ArcLength(
        curve: CatmulRomCurve3,
        t: float,
        sample_count: int = 10) -> float:
    """Computes the arc length at the specified curve parameter.

    Args:
        curve (CatmulRomCurve3): The curve whose arc length is to be
            calculated.
        t (float): The curve parameter. It has to be in the domain.
        sample_count (int, optional): It controls the precision of the result.
            High sample count yields more accurate precision. Defaults to 10.

    Returns:
        float: The calculated arc length.
    """
    assert sample_count >= 2

    t1, t2 = CatmulRomDomain(curve)
    assert t >= t1 and t <= t2
    ts = linspace(t1, t2, num=sample_count)
    ps = CatmulRomFTs(curve=curve, ts=ts)
    assert ps.shape[0] >= 2

    prev_s = 0.0
    curr_s = norm(ps[1] - ps[0])
    curr_i = 1

    while ts[curr_i] < t:
        assert curr_i + 1 < ps.shape[0]

        prev_s = curr_s
        curr_s += norm(ps[curr_i + 1] - ps[curr_i])
        curr_i += 1

    alpha = (t - ts[curr_i - 1])/(ts[curr_i] - ts[curr_i - 1])
    return (1 - alpha)*prev_s + alpha*curr_s


def CatmulRomArcLength2T(
        curve: CatmulRomCurve3,
        s: float,
        sample_count: int = 10) -> float:
    """Computes the curve parameter at the specified arc length.

    Args:
        curve (CatmulRomCurve3): The curve whose curve parameter is to be
            calculated.
        s (float): The arc length. The result will be clamped, if the
            corresponding curve parameter is out of domain.
        sample_count (int): It controls the precision of the result.
            High sample count yields more accurate precision. Defaults to 10.

    Returns:
        float: The calculated curve parameter.
    """
    assert sample_count >= 2

    t1, t2 = CatmulRomDomain(curve)
    ts = linspace(t1, t2, num=sample_count)
    ps = CatmulRomFTs(curve=curve, ts=ts)
    assert ps.shape[0] >= 2

    prev_s = 0.0
    curr_s = norm(ps[1] - ps[0])
    curr_i = 1

    while curr_s < s and curr_i + 1 < ps.shape[0]:
        prev_s = curr_s
        curr_s += norm(ps[curr_i + 1] - ps[curr_i])
        curr_i += 1

    alpha = (s - prev_s)/(curr_s - prev_s)
    t = (1 - alpha)*ts[curr_i - 1] + alpha*ts[curr_i]

    return clip(t, t1, t2)
