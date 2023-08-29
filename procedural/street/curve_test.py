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
from procedural.probing.flow import ProbeConnectionFlow
from procedural.probing.population import PopulationProbe
from procedural.street.curve import ComputeStreetCurves


class StreetCurveTest(unittest.TestCase):
    def test_WhenIntersectionIsZeroWay_ThenExpectEmptyResult(self):
        probe = PopulationProbe(location=array(
            [0, 0, 0]), population_grid_200=100)
        curves = ComputeStreetCurves(probes=[probe], connection_flows=[])
        self.assertEqual(0, len(curves))

    def test_WhenIntersectionIsOneWay_ThenCheckCurve(self):
        probe0 = PopulationProbe(location=array(
            [0, 0, 0]), population_grid_200=100)
        probe1 = PopulationProbe(location=array(
            [1000, 0, 0]), population_grid_200=100)
        flow0 = ProbeConnectionFlow(
            src_probe_index=0, dst_probe_index=1, flow=10, lane_count=1)
        flow1 = ProbeConnectionFlow(
            src_probe_index=1, dst_probe_index=0, flow=10, lane_count=1)
        curves = ComputeStreetCurves(probes=[probe0, probe1],
                                     connection_flows=[flow0, flow1])
        self.assertEqual(2, len(curves))
        self.assertEqual(4, len(curves[flow0].control_points))
        self.assertEqual(4, len(curves[flow1].control_points))

        self.assertEqual(-1000, curves[flow0].control_points[0].x)
        self.assertEqual(0, curves[flow0].control_points[0].y)
        self.assertEqual(0, curves[flow0].control_points[0].z)

        self.assertEqual(0, curves[flow0].control_points[1].x)
        self.assertEqual(0, curves[flow0].control_points[1].y)
        self.assertEqual(0, curves[flow0].control_points[1].z)

        self.assertEqual(1000, curves[flow0].control_points[2].x)
        self.assertEqual(0, curves[flow0].control_points[2].y)
        self.assertEqual(0, curves[flow0].control_points[2].z)

        self.assertEqual(2000, curves[flow0].control_points[3].x)
        self.assertEqual(0, curves[flow0].control_points[3].y)
        self.assertEqual(0, curves[flow0].control_points[3].z)

        self.assertEqual(2000, curves[flow1].control_points[0].x)
        self.assertEqual(0, curves[flow1].control_points[0].y)
        self.assertEqual(0, curves[flow1].control_points[0].z)

        self.assertEqual(1000, curves[flow1].control_points[1].x)
        self.assertEqual(0, curves[flow1].control_points[1].y)
        self.assertEqual(0, curves[flow1].control_points[1].z)

        self.assertEqual(0, curves[flow1].control_points[2].x)
        self.assertEqual(0, curves[flow1].control_points[2].y)
        self.assertEqual(0, curves[flow1].control_points[2].z)

        self.assertEqual(-1000, curves[flow1].control_points[3].x)
        self.assertEqual(0, curves[flow1].control_points[3].y)
        self.assertEqual(0, curves[flow1].control_points[3].z)


if __name__ == '__main__':
    unittest.main()
