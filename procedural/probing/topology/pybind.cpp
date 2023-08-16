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
#include "procedural/probing/topology/topology.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace e8 {
namespace procedural {

void RegisterProbeTopology(pybind11::module *m) {
  // Output struct.
  pybind11::class_<ProbeConnection>(*m, "ProbeConnection")
      .def(pybind11::init<unsigned, unsigned>())
      .def_readonly("src_probe_index", &ProbeConnection::src_probe_index,
                    pybind11::return_value_policy::copy)
      .def_readonly("dst_probe_index", &ProbeConnection::dst_probe_index,
                    pybind11::return_value_policy::copy);

  pybind11::class_<ProbeTopologyResult>(*m, "ProbeTopologyResult")
      .def(pybind11::init<>())
      .def_readonly("connections", &ProbeTopologyResult::connections,
                    pybind11::return_value_policy::copy)
      .def_readonly("score", &ProbeTopologyResult::score,
                    pybind11::return_value_policy::copy);

  // Function.
  m->def("ComputeProbeTopology", &ComputeProbeTopology,
         pybind11::return_value_policy::copy);
}

} // namespace procedural
} // namespace e8
