"""
e8City
Copyright (C) 2024 e8yes

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
from sympy import Circle
from sympy import Point2D
from procedural.probing.flow import ProbeConnectionFlow
from procedural.probing.population import PopulationProbe
from procedural.street.curve import ComputeStreetCurves
from procedural.street.ir_traffic_way import GenerateTrafficWays
from procedural.street.ir_intersection import GenerateIntersections


class IrIntersectionTest(unittest.TestCase):
    def test_EmptyIntersectionConnectivities(self):
        intersections = GenerateIntersections(
            probes=[], intersection_areas=[], traffic_ways={})
        self.assertEqual(len(intersections), 0)

    def test_DeadendIntersectionConnectivities(self):
        probe0 = PopulationProbe(location=array(
            [0, 0, 0]), population_grid_200=100)
        probe1 = PopulationProbe(location=array(
            [1000, 0, 0]), population_grid_200=100)

        intersection0 = Circle(Point2D((0, 0)), 7)
        intersection1 = Circle(Point2D((1000, 0)), 7)

        flow0 = ProbeConnectionFlow(
            src_probe_index=0, dst_probe_index=1, flow=10, lane_count=1)
        flow1 = ProbeConnectionFlow(
            src_probe_index=1, dst_probe_index=0, flow=40, lane_count=2)

        street_curves = ComputeStreetCurves(
            probes=[probe0, probe1],
            intersection_areas=[intersection0, intersection1],
            connection_flows=[flow0, flow1])
        traffic_ways = GenerateTrafficWays(street_curves)

        intersections = GenerateIntersections(
            probes=[probe0, probe1],
            intersection_areas=[intersection0, intersection1],
            traffic_ways=traffic_ways)

        self.assertEqual(len(intersections), 2)
        self.assertEqual(intersections[0].standard.center.x, 0)
        self.assertEqual(intersections[0].standard.center.y, 0)
        self.assertEqual(intersections[0].standard.center.z, 0)
        self.assertEqual(intersections[1].standard.center.x, 1000)
        self.assertEqual(intersections[1].standard.center.y, 0)
        self.assertEqual(intersections[1].standard.center.z, 0)

        self.assertEqual(len(intersections[0].standard.connections), 1)
        self.assertEqual(len(intersections[1].standard.connections), 1)

        self.assertEqual(
            intersections[0].standard.connections[0].inbound_traffic_way_id,
            "1_0")
        self.assertEqual(
            intersections[0].standard.connections[0].inbound_lane_number, 0)
        self.assertEqual(
            intersections[0].standard.connections[0].outbound_traffic_way_id,
            "0_1")

        self.assertEqual(
            intersections[1].standard.connections[0].inbound_traffic_way_id,
            "0_1")
        self.assertEqual(
            intersections[1].standard.connections[0].inbound_lane_number, 0)
        self.assertEqual(
            intersections[1].standard.connections[0].outbound_traffic_way_id,
            "1_0")

    def test_TwoWayIntersectionConnectivities(self):
        probe0 = PopulationProbe(location=array(
            [0, 0, 0]), population_grid_200=100)
        probe1 = PopulationProbe(location=array(
            [1000, 1, 0]), population_grid_200=100)
        probe2 = PopulationProbe(location=array(
            [-1000, -1, 0]), population_grid_200=100)

        intersection0 = Circle(Point2D((0, 0)), 7)
        intersection1 = Circle(Point2D((1000, 1)), 7)
        intersection2 = Circle(Point2D((-1000, -1)), 7)

        flow01 = ProbeConnectionFlow(
            src_probe_index=0, dst_probe_index=1, flow=10, lane_count=1)
        flow10 = ProbeConnectionFlow(
            src_probe_index=1, dst_probe_index=0, flow=40, lane_count=2)

        flow02 = ProbeConnectionFlow(
            src_probe_index=0, dst_probe_index=2, flow=80, lane_count=3)
        flow20 = ProbeConnectionFlow(
            src_probe_index=2, dst_probe_index=0, flow=40, lane_count=2)

        street_curves = ComputeStreetCurves(
            probes=[probe0, probe1, probe2],
            intersection_areas=[intersection0, intersection1, intersection2],
            connection_flows=[flow01, flow10, flow02, flow20])
        traffic_ways = GenerateTrafficWays(street_curves)

        intersections = GenerateIntersections(
            probes=[probe0, probe1, probe2],
            intersection_areas=[intersection0, intersection1, intersection2],
            traffic_ways=traffic_ways)
        self.assertEqual(len(intersections), 3)

        self.assertEqual(intersections[0].standard.center.x, 0)
        self.assertEqual(intersections[0].standard.center.y, 0)
        self.assertEqual(intersections[0].standard.center.z, 0)
        self.assertEqual(intersections[1].standard.center.x, 1000)
        self.assertEqual(intersections[1].standard.center.y, 1)
        self.assertEqual(intersections[1].standard.center.z, 0)
        self.assertEqual(intersections[2].standard.center.x, -1000)
        self.assertEqual(intersections[2].standard.center.y, -1)
        self.assertEqual(intersections[2].standard.center.z, 0)

        self.assertEqual(len(intersections[0].standard.connections), 6)
        self.assertEqual(
            intersections[0].standard.connections[0].inbound_traffic_way_id,
            "2_0")
        self.assertEqual(
            intersections[0].standard.connections[0].inbound_lane_number,
            0)
        self.assertEqual(
            intersections[0].standard.connections[0].outbound_traffic_way_id,
            "0_2")

        self.assertEqual(
            intersections[0].standard.connections[1].inbound_traffic_way_id,
            "2_0")
        self.assertEqual(
            intersections[0].standard.connections[1].inbound_lane_number,
            0)
        self.assertEqual(
            intersections[0].standard.connections[1].outbound_traffic_way_id,
            "0_1")

        self.assertEqual(
            intersections[0].standard.connections[2].inbound_traffic_way_id,
            "2_0")
        self.assertEqual(
            intersections[0].standard.connections[2].inbound_lane_number,
            1)
        self.assertEqual(
            intersections[0].standard.connections[2].outbound_traffic_way_id,
            "0_1")

        self.assertEqual(
            intersections[0].standard.connections[3].inbound_traffic_way_id,
            "1_0")
        self.assertEqual(
            intersections[0].standard.connections[3].inbound_lane_number,
            0)
        self.assertEqual(
            intersections[0].standard.connections[3].outbound_traffic_way_id,
            "0_1")

        self.assertEqual(
            intersections[0].standard.connections[4].inbound_traffic_way_id,
            "1_0")
        self.assertEqual(
            intersections[0].standard.connections[4].inbound_lane_number,
            0)
        self.assertEqual(
            intersections[0].standard.connections[4].outbound_traffic_way_id,
            "0_2")

        self.assertEqual(
            intersections[0].standard.connections[5].inbound_traffic_way_id,
            "1_0")
        self.assertEqual(
            intersections[0].standard.connections[5].inbound_lane_number,
            1)
        self.assertEqual(
            intersections[0].standard.connections[5].outbound_traffic_way_id,
            "0_2")

    def test_ThreeWayIntersectionConnectivities(self):
        # TODO.
        pass

    def test_FourWayIntersectionConnectivities(self):
        # TODO.
        pass


if __name__ == '__main__':
    unittest.main()
