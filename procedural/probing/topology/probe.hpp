// e8City
// Copyright (C) 2023 e8yes
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <eigen3/Eigen/Core>

namespace e8 {
namespace procedural {

// A population probe is a stratified sample point in the city which gives an
// estimate of the population density in its surrounding area.
struct PopulationProbe {
  PopulationProbe(Eigen::Vector3f const &location, float population_grid_200)
      : location(location), population_grid_200(population_grid_200) {}

  // The spatial Cartesian location of the probe.
  Eigen::Vector3f location;

  // The expected number of people living in the 200m*200m grid, centered at the
  // location.
  float population_grid_200;
};

} // namespace procedural
} // namespace e8
