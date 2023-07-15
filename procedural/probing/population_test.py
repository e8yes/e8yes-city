import unittest
from math import sqrt
from typing import List

from procedural.probing.population import GeneratePopulationProbes
from procedural.probing.population import PopulationProbe


def _Range(probes: List[PopulationProbe]) -> float:
    min_x, max_x = float("inf"), -float("inf")
    min_y, max_y = float("inf"), -float("inf")

    for probe in probes:
        min_x = min(probe.location[0], min_x)
        max_x = max(probe.location[0], max_x)

        min_y = min(probe.location[1], min_y)
        max_y = max(probe.location[1], max_y)

    x_range = max_x - min_x
    y_range = max_y - min_y
    return sqrt(x_range**2 + y_range**2)


class PopulationTest(unittest.TestCase):
    def test_WhenCitySizeIsSmall_ThenExpectEmptyResult(self):
        probes = GeneratePopulationProbes(city_size=100)
        self.assertEqual(0, len(probes))

    def test_WhenCitySizeIsNormal_ThenCheckProbeLocationShape(self):
        probes = GeneratePopulationProbes(city_size=3000)
        self.assertLess(0, len(probes))

        for probe in probes:
            self.assertEqual(1, len(probe.location.shape))
            self.assertEqual(3, probe.location.shape[0])

    def test_WhenCitySizeIsNormal_ThenCheckUniqueness(self):
        probes = GeneratePopulationProbes(city_size=3000)
        self.assertLess(0, len(probes))

        location_set = set()
        for probe in probes:
            location_tuple = (probe.location[0],
                              probe.location[1],
                              probe.location[2])
            self.assertTrue(location_tuple not in location_set)
            location_set.add(location_tuple)

    def test_WhenCitySizeIsNormal_ThenCheckProbeNonUniformity(self):
        # TODO: Apply Kolmogorov Smirnov test here.
        pass

    def test_WhenCitySizeIncreases_ThenCheckProbeRange(self):
        probes_3000 = GeneratePopulationProbes(city_size=3000)
        probes_5000 = GeneratePopulationProbes(city_size=5000)
        probes_7000 = GeneratePopulationProbes(city_size=7000)

        probe_3000_range = _Range(probes_3000)
        probe_5000_range = _Range(probes_5000)
        probe_7000_range = _Range(probes_7000)

        self.assertGreater(probe_5000_range, probe_3000_range)
        self.assertGreater(probe_7000_range, probe_5000_range)

    def test_WhenCitySizeIsNormal_ThenCheckPopulationSize(
            self):
        probes = GeneratePopulationProbes(city_size=10000)

        population_200 = 0.0
        for probe in probes:
            population_200 += probe.population_grid_200

        self.assertAlmostEqual(348*10**3, population_200, delta=5*10**3)


if __name__ == '__main__':
    unittest.main()
