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
from sympy import Point2D
from sympy import Polygon
from procedural.probing.flow import ProbeConnectionFlow
from procedural.probing.population import PopulationProbe
from procedural.probing.topology import ProbeConnection
from procedural.street.ir_street import GenerateStreets


class IrStreetTest(unittest.TestCase):
    def test_GenerateStreets(self):
        probe0 = PopulationProbe(location=array(
            [0, 0, 0]), population_grid_200=100)
        probe1 = PopulationProbe(location=array(
            [1000, 0, 0]), population_grid_200=100)

        intersection0 = Polygon(
            Point2D((7, 7)),
            Point2D((7, -7)),
            Point2D((-7, -7)),
            Point2D((-7, 7)))
        intersection1 = Polygon(
            Point2D((1007, 7)),
            Point2D((1007, -7)),
            Point2D((993, -7)),
            Point2D((993, 7)))

        flow0 = ProbeConnectionFlow(
            src_probe_index=0, dst_probe_index=1, flow=10, lane_count=1)
        flow1 = ProbeConnectionFlow(
            src_probe_index=1, dst_probe_index=0, flow=40, lane_count=2)

        streets = GenerateStreets(
            probes=[probe0, probe1],
            intersection_areas=[intersection0, intersection1],
            connection_flows=[flow0, flow1])

        self.assertTrue(ProbeConnection(
            src_probe_index=0, dst_probe_index=1) in streets)
        self.assertTrue(ProbeConnection(
            src_probe_index=1, dst_probe_index=0) not in streets)


if __name__ == '__main__':
    unittest.main()
