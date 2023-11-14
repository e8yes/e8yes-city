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

import unittest
from numpy import array
from numpy import array_equal
from numpy import linspace
from intermediate_representation.curve_pb2 import CatmulRomCurve3
from intermediate_representation.catmul_rom import CatmulRomArcLength2T
from intermediate_representation.catmul_rom import CatmulRomDomain
from intermediate_representation.catmul_rom import CatmulRomFTs
from intermediate_representation.catmul_rom import CatmulRomT2ArcLength
from intermediate_representation.space_pb2 import Point3


class CatmulRomTest(unittest.TestCase):
    def test_FtRechesControlPoints(self):
        p0 = Point3(x=-15, y=-20, z=0)
        p1 = Point3(x=-10, y=-10, z=0)
        p2 = Point3(x=10, y=10, z=0)
        p3 = Point3(x=15, y=20, z=0)
        curve = CatmulRomCurve3(control_points=[
            p0, p1, p2, p3])

        t1, t2 = CatmulRomDomain(curve=curve)
        ts = linspace(start=t1, stop=t2, num=10)
        pts = CatmulRomFTs(curve=curve, ts=ts)

        self.assertEqual(10, pts.shape[0])
        self.assertTrue(array_equal(array([-10, -10, 0]), pts[0]))
        self.assertTrue(array_equal(array([10, 10, 0]), pts[9]))

    def test_CurveParameterToArcLength(self):
        p0 = Point3(x=-15, y=-20, z=0)
        p1 = Point3(x=-10, y=-10, z=0)
        p2 = Point3(x=10, y=10, z=0)
        p3 = Point3(x=15, y=20, z=0)
        curve = CatmulRomCurve3(control_points=[
            p0, p1, p2, p3])

        t1, t2 = CatmulRomDomain(curve=curve)

        s = CatmulRomT2ArcLength(curve=curve, t=t1)
        self.assertAlmostEqual(0, s)

        s = CatmulRomT2ArcLength(curve=curve, t=t2)
        self.assertAlmostEqual(28.3, s, places=1)

        s = CatmulRomT2ArcLength(curve=curve, t=(t1 + t2)/2)
        self.assertAlmostEqual(14.2, s, places=1)

    def test_ArcLengthToCurveParameter(self):
        p0 = Point3(x=-15, y=-20, z=0)
        p1 = Point3(x=-10, y=-10, z=0)
        p2 = Point3(x=10, y=10, z=0)
        p3 = Point3(x=15, y=20, z=0)
        curve = CatmulRomCurve3(control_points=[
            p0, p1, p2, p3])

        t1, t2 = CatmulRomDomain(curve=curve)

        t = CatmulRomArcLength2T(curve=curve, s=0.0)
        self.assertAlmostEqual(t1, t, places=1)

        t = CatmulRomArcLength2T(curve=curve, s=28.3)
        self.assertAlmostEqual(t2, t, places=1)

        t = CatmulRomArcLength2T(curve=curve, s=14.2)
        self.assertAlmostEqual((t1 + t2)/2, t, places=1)


if __name__ == '__main__':
    unittest.main()
