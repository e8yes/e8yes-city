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
from procedural.street.intersection_area import ComputeIntersectionAreas


class IntersectionAreaTest(unittest.TestCase):
    def test_TwoWayIntersection(self):
        probe0 = PopulationProbe(location=array(
            [0, 0, 0]), population_grid_200=100)
        probe1 = PopulationProbe(location=array(
            [500, 0, 0]), population_grid_200=100)
        probe2 = PopulationProbe(location=array(
            [1000, 0, 0]), population_grid_200=100)

        flow01 = ProbeConnectionFlow(
            src_probe_index=0, dst_probe_index=1, flow=10, lane_count=2)
        flow10 = ProbeConnectionFlow(
            src_probe_index=1, dst_probe_index=0, flow=10, lane_count=1)

        flow12 = ProbeConnectionFlow(
            src_probe_index=1, dst_probe_index=2, flow=10, lane_count=1)
        flow21 = ProbeConnectionFlow(
            src_probe_index=2, dst_probe_index=1, flow=10, lane_count=3)

        areas = ComputeIntersectionAreas(
            probes=[probe0, probe1, probe2],
            connection_flows=[flow01, flow12, flow10, flow21])
        self.assertEqual(len(areas), 3)

        self.assertEqual(areas[0].center.x.evalf(), 0)
        self.assertEqual(areas[0].center.y.evalf(), 0)
        self.assertAlmostEqual(areas[0].radius.evalf(), 5.775)

        self.assertEqual(areas[1].center.x.evalf(), 500)
        self.assertEqual(areas[1].center.y.evalf(), 0)
        self.assertAlmostEqual(areas[1].radius.evalf(), 7.7)

        self.assertEqual(areas[2].center.x.evalf(), 1000)
        self.assertEqual(areas[2].center.y.evalf(), 0)
        self.assertAlmostEqual(areas[2].radius.evalf(), 7.7)

    def test_ThreeWayIntersection(self):
        probe0 = PopulationProbe(location=array(
            [0, 0, 0]), population_grid_200=100)
        probe1 = PopulationProbe(location=array(
            [500, 0, 0]), population_grid_200=100)
        probe2 = PopulationProbe(location=array(
            [1000, 0, 0]), population_grid_200=100)
        probe3 = PopulationProbe(location=array(
            [500, 300, 0]), population_grid_200=100)

        flow01 = ProbeConnectionFlow(
            src_probe_index=0, dst_probe_index=1, flow=10, lane_count=2)
        flow10 = ProbeConnectionFlow(
            src_probe_index=1, dst_probe_index=0, flow=10, lane_count=1)

        flow12 = ProbeConnectionFlow(
            src_probe_index=1, dst_probe_index=2, flow=10, lane_count=1)
        flow21 = ProbeConnectionFlow(
            src_probe_index=2, dst_probe_index=1, flow=10, lane_count=3)

        flow13 = ProbeConnectionFlow(
            src_probe_index=1, dst_probe_index=3, flow=10, lane_count=2)
        flow31 = ProbeConnectionFlow(
            src_probe_index=3, dst_probe_index=1, flow=10, lane_count=3)

        areas = ComputeIntersectionAreas(
            probes=[probe0, probe1, probe2, probe3],
            connection_flows=[flow01, flow13, flow12, flow10, flow21, flow31])
        self.assertEqual(len(areas), 4)

        self.assertEqual(areas[0].center.x.evalf(), 0)
        self.assertEqual(areas[0].center.y.evalf(), 0)
        self.assertAlmostEqual(areas[0].radius.evalf(), 5.775)

        self.assertEqual(areas[1].center.x.evalf(), 500)
        self.assertEqual(areas[1].center.y.evalf(), 0)
        self.assertAlmostEqual(areas[1].radius.evalf(), 9.625)

        self.assertEqual(areas[2].center.x.evalf(), 1000)
        self.assertEqual(areas[2].center.y.evalf(), 0)
        self.assertAlmostEqual(areas[2].radius.evalf(), 7.7)

        self.assertEqual(areas[3].center.x.evalf(), 500)
        self.assertEqual(areas[3].center.y.evalf(), 300)
        self.assertAlmostEqual(areas[3].radius.evalf(), 9.625)


if __name__ == '__main__':
    unittest.main()
