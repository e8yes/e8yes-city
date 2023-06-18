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

from numpy import float32
from numpy import expand_dims
from numpy import ndarray
from numpy import inner
from numpy import zeros
from numpy.linalg import norm
from pykdtree.kdtree import KDTree
from typing import Tuple


def _Integrate(points: ndarray) -> ndarray:
    """Returns an array where the element a[i] is the total length covered
    before points[i].
    """
    length_integration = zeros(shape=(points.shape[0],), dtype=float32)

    for i in range(1, points.shape[0]):
        segment_length = norm(x=points[i] - points[i - 1])
        length_integration[i] = length_integration[i - 1] + segment_length

    return length_integration


class LanePiece:
    """Represents a partition of the entire lane as a list of continuous line
    segments with constant width.
    """

    def __init__(self,
                 width: float,
                 points: ndarray) -> None:
        """Constructs a lane piece.

        Args:
            width (float): The width of the lane piece.
            points (ndarray): a dense ordered array of center points in this
            lane piece, with shape being (num_points, 3). The points are
            approximately equidistant, and the distance is shorter than
            width/2. In addition, the cloest gap between any pair of segment
            is wider than width*(sqrt(2) - 1)/2
        """
        assert width is not None and width > 0
        assert points is not None and   \
            points.shape[0] >= 2 and    \
            points.shape[1] == 3

        self._width = width
        self._points = points
        self._length_integration = _Integrate(points=points)
        self._kd_tree = KDTree(points)

    def Width(self) -> float:
        """Returns the width of this lane piece.
        """
        return self._width

    def Points(self) -> ndarray:
        """Returns a dense ordered array of center points in this lane piece.
        shape=(num_points, 3)
        """
        return self._points

    def Length(self) -> float:
        """Returns the total length (in meter) of this lane piece.
        """
        return self._length_integration[-1]

    def LengthFromStartToLocation(self, location: ndarray) -> float:
        """Returns the total length (in meter) covered from the beginning of
        this lane piece to the specified location, assuming the location is
        within the lane. The returned length is guaranteed to be in
        [0, Length()].
        """
        start_idx, end_idx, proj, _ = self._FindSegment(location)

        if start_idx is None:
            return 0
        if end_idx is None:
            return self.Length()

        return self._length_integration[start_idx] + proj

    def LengthFromLocationToEnd(self, location: ndarray) -> float:
        """Returns the total length (in meter) remaining from the specified
        location to the ending of this lane piece, assuming the location is
        within the lane. The returned length is guaranteed to be in
        [0, Length()].
        """
        return self.Length() - self.LengthFromStartToLocation(location)

    def DistanceToLocation(self, location: ndarray) -> float:
        """Returns the closest distance from the lane to the specified
        location (can be arbitrary).
        """
        start_idx, end_idx, proj, dir = self._FindSegment(location)

        if start_idx is None:
            return norm(location - self._points[0])
        if end_idx is None:
            return norm(location - self._points[-1])

        projected_point = self._points[start_idx] + proj*dir
        return norm(location - projected_point)

    def _FindSegment(self,
                     location: ndarray) -> Tuple[int, int, float, ndarray]:
        _, idxs = self._kd_tree.query(expand_dims(location, axis=0), k=1)
        idx = idxs[0]

        if idx == self._points.shape[0] - 1:
            proj, dir = self._Projection(idx - 1, idx, location)
            if proj > self._SegmentLength(idx - 1, idx):
                # Lands after the last segment.
                return idx, None, None, None
            # Lands in the last segment.
            return idx - 1, idx, proj, dir

        proj, dir = self._Projection(idx, idx + 1, location)
        if proj >= 0:
            # Lands in the segment (idx, idx + 1).
            return idx, idx + 1, proj, dir

        if idx == 0:
            # Lands before the first segment.
            return None, idx, None, None

        # Lands before the segment (idx, idx + 1).
        proj, dir = self._Projection(idx - 1, idx, location)
        return idx - 1, idx, proj, dir

    def _SegmentLength(self, start_idx: int, end_idx: int) -> float:
        return self._length_integration[end_idx] - \
            self._length_integration[start_idx]

    def _Projection(self,
                    start_idx: int,
                    end_idx: int,
                    location: ndarray) -> Tuple[float, ndarray]:
        segment = self._points[end_idx] - self._points[start_idx]
        dir = segment/self._SegmentLength(start_idx, end_idx)
        query = location - self._points[start_idx]
        return inner(query, dir), dir
