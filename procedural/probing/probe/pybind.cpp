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

#include "procedural/probing/probe/probe.hpp"
#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>

namespace e8 {
namespace procedural {

void RegisterPopulationProbe(pybind11::module *m) {
  pybind11::class_<PopulationProbe>(*m, "PopulationProbe")
      .def(pybind11::init<Eigen::Vector3f, float>());
}

} // namespace procedural
} // namespace e8
