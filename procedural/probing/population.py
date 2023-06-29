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

from dataclasses import dataclass
from numpy import concatenate
from numpy import cos
from numpy import exp
from numpy import expand_dims
from numpy import full
from numpy import linspace
from numpy import log
from numpy import matmul
from numpy import meshgrid
from numpy import ndarray
from numpy import pi
from numpy import sin
from numpy import sqrt
from numpy import squeeze
from numpy import sum
from numpy import unique
from numpy import zeros
from numpy.linalg import norm
from numpy.random import choice
from numpy.random import seed
from numpy.random import uniform
from typing import List


_SQUARE_METERS_PER_CORE = 7*10**6
_PROBES_PER_SQUARE_METER = 30/10**6
_PROBE_GRID_SIZE_METER = 200
_PERSONS_PER_PROBE = 6.7
_BASE_POPULATION_PER_SQUARE_METER = 4.6/10**3


def _CartesianToLocation2d(xs: ndarray, ys: ndarray) -> ndarray:
    """Converts points from Cartesian coordinate to a location array of shape
    (N, 2).
    """
    xs = expand_dims(a=xs, axis=1)
    ys = expand_dims(a=ys, axis=1)
    return concatenate((xs, ys), axis=1)


def _PolarToLocation2d(radii: ndarray, thetas: ndarray) -> ndarray:
    """Converts points from polar coordinate to a location (Cartesian) array
    of shape (N, 2)
    """
    return _CartesianToLocation2d(xs=radii*cos(thetas),
                                  ys=radii*sin(thetas))


def _RadianToDirection2d(thetas: ndarray) -> ndarray:
    """Converts unit vectors defined in polar coordinate to an array
    (Cartesian) of shape (N, 2)
    """
    return _PolarToLocation2d(radii=1, thetas=thetas)


def _CreateBases2d(directions: ndarray) -> ndarray:
    """Creates orthogonal bases in R2 from the specified directions.
    """
    orthogonals = zeros(shape=directions.shape)
    orthogonals[:, 0] = directions[:, 1]
    orthogonals[:, 1] = -directions[:, 0]

    directions = expand_dims(a=directions, axis=1)
    orthogonals = expand_dims(a=orthogonals, axis=1)
    return concatenate((directions, orthogonals), axis=1)


class _CityCores:
    """Stores information associated with core areas in the city.
    """

    def __init__(self,
                 locations: ndarray,
                 bases: ndarray,
                 expected_radii: ndarray,
                 pr_important: ndarray) -> None:
        assert locations.shape[0] == bases.shape[0]
        assert locations.shape[0] == pr_important.shape[0]

        self.core_count = locations.shape[0]

        self.locations = locations
        self.bases = bases
        self.expected_radii = expected_radii
        self.pr_important = pr_important


def _GenerateCityCores(city_size: float) -> _CityCores:
    core_count = int(city_size*city_size / _SQUARE_METERS_PER_CORE)

    xs = uniform(low=-city_size/2, high=city_size/2, size=core_count)
    ys = uniform(low=-city_size/2, high=city_size/2, size=core_count)
    core_locations = _CartesianToLocation2d(xs=xs, ys=ys)

    core_radii = full(shape=core_locations.shape[0],
                      fill_value=sqrt(_SQUARE_METERS_PER_CORE/pi))

    thetas = uniform(low=0, high=2*pi, size=core_count)
    core_directions = _RadianToDirection2d(thetas=thetas)
    core_bases = _CreateBases2d(directions=core_directions)

    importance_scores = uniform(low=0, high=1, size=core_count)
    pr_important = importance_scores/sum(importance_scores)

    return _CityCores(locations=core_locations,
                      bases=core_bases,
                      expected_radii=core_radii,
                      pr_important=pr_important)


def _SnapToLocalGrid(locations: ndarray):
    """Moves/discretizes the locations to their closes grid point. A grid is a
    uniform mesh composed of squares of width _PROBE_GRID_DiZE_METER
    """
    grid_corners = locations//_PROBE_GRID_SIZE_METER
    return grid_corners*_PROBE_GRID_SIZE_METER


def _SampleFromIstropicBivariateExponentialDistribution(expected_radius: float,
                                                        size: int) -> ndarray:
    """TODO: A bivariate exponential distribution with correlation could model
    the population more realistically.
    """
    qs = uniform(low=0, high=1, size=size)
    radii = -expected_radius*log(1 - sqrt(qs))
    thetas = uniform(low=0, high=2*pi, size=size)
    return _PolarToLocation2d(radii=radii, thetas=thetas)


def _GenerateProbePoints(city_size: float,
                         city_cores: _CityCores) -> ndarray:
    probe_count = int(city_size*city_size*_PROBES_PER_SQUARE_METER)

    # Samples from a mixture of exponential distributions.
    core_choices = choice(
        a=city_cores.core_count, size=probe_count, p=city_cores.pr_important)

    local_locations = _SampleFromIstropicBivariateExponentialDistribution(
        expected_radius=city_cores.expected_radii[core_choices],
        size=probe_count)
    local_probes = _SnapToLocalGrid(locations=local_locations)
    local_probes = expand_dims(local_probes, axis=2)

    global_probes = city_cores.locations[core_choices] +                    \
        squeeze(matmul(city_cores.bases[core_choices], local_probes), axis=2)
    global_probes = unique(ar=global_probes, axis=0)

    # TODO: should use a height field instead of hard coding it to a fixed
    # value.
    height = full(shape=(global_probes.shape[0], 1), fill_value=0.0)
    return concatenate((global_probes, height), axis=1)


def _EstimatePopulationSize(city_size: float, probe_count: int) -> float:
    return probe_count*_PERSONS_PER_PROBE + \
        _BASE_POPULATION_PER_SQUARE_METER*city_size**2


def _EvaluatePopulationDensityAt(locations: ndarray,
                                 city_cores: _CityCores):
    densities = zeros(shape=locations.shape[0])

    for i in range(city_cores.core_count):
        e = city_cores.expected_radii[i]
        lamb = 1/e
        radius = norm(locations - city_cores.locations[i], axis=1)
        core_densities = lamb/(2*pi*(1 + e))*exp(-lamb*radius)

        densities += city_cores.pr_important[i]*core_densities

    return densities


def _ComputePopulationEstimates(location: ndarray,
                                population_size: float,
                                city_cores: _CityCores,
                                area_width: float,
                                patch_width: int) -> float:
    x = location[0] + linspace(start=-area_width/2,
                               stop=area_width/2,
                               num=area_width//patch_width)
    y = location[1] + linspace(start=-area_width/2,
                               stop=area_width/2,
                               num=area_width//patch_width)
    xv, yv = meshgrid(x, y)
    loc_x = expand_dims(xv.flatten(), axis=1)
    loc_y = expand_dims(yv.flatten(), axis=1)
    evaluation_locations = concatenate((loc_x, loc_y), axis=1)

    densities = _EvaluatePopulationDensityAt(evaluation_locations, city_cores)
    area_densities = patch_width**2*densities
    return population_size*sum(area_densities)


@dataclass
class PopulationProbe:
    """A population probe is a stratified sample point in the city which gives
    an estimate of the population density in its surrounding area.
    """

    # A 3d location in Cartesian coordinate of shape (N, 3).
    location: ndarray

    # The expected number of people living in the 200m*200m grid, centered at
    # the location.
    population_grid_200: float

    # The expected number of people living in the 1km*1km grid, centered at
    # the location.
    population_grid_1000: float


def GeneratePopulationProbes(city_size: float,
                             seed_number: int = 13) -> List[PopulationProbe]:
    """Generates population probes randomly given the city size. The
    generation follows an exponentially distributed population model at
    multiple core areas. TODO: Should take the terrain (potentially
    represented by a height field) into consideration.

    Args:
        city_size (float): The magnitude of the city, in meter.
        seed_number (int, optional): The random seed used for the generation.
            Defaults to 13.

    Returns:
        List[PopulationProbe]: A list of generated population probes. It could
            return an empty array when the city is too small in size.
    """
    seed(seed_number)

    city_cores = _GenerateCityCores(city_size=city_size)
    if city_cores.core_count == 0:
        return list()
    probe_points = _GenerateProbePoints(
        city_size=city_size, city_cores=city_cores)
    population_size = _EstimatePopulationSize(
        city_size=city_size, probe_count=probe_points.shape[0])

    result = list()
    for i in range(probe_points.shape[0]):
        location = probe_points[i]
        population_grid_200 = _ComputePopulationEstimates(
            location=location,
            population_size=population_size,
            city_cores=city_cores,
            area_width=200,
            patch_width=50)
        population_grid_1000 = _ComputePopulationEstimates(
            location=location,
            population_size=population_size,
            city_cores=city_cores,
            area_width=1000,
            patch_width=100)

        result.append(PopulationProbe(
            location=location,
            population_grid_200=population_grid_200,
            population_grid_1000=population_grid_1000))

    return result
