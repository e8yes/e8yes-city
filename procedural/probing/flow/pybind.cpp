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

#include "procedural/probing/flow/flow.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace e8 {
namespace procedural {

void RegisterFlowTopology(pybind11::module *m) {
  pybind11::class_<ProbeConnectionFlow>(*m, "ProbeConnectionFlow")
      .def(pybind11::init<unsigned, unsigned, float, unsigned>())
      .def_readonly("src_probe_index", &ProbeConnectionFlow::src_probe_index,
                    pybind11::return_value_policy::copy)
      .def_readonly("dst_probe_index", &ProbeConnectionFlow::dst_probe_index,
                    pybind11::return_value_policy::copy)
      .def_readonly("flow", &ProbeConnectionFlow::flow,
                    pybind11::return_value_policy::copy)
      .def_readonly("lane_count", &ProbeConnectionFlow::lane_count,
                    pybind11::return_value_policy::copy);

  // Function.
  m->def("EstimateProbeTopologyFlow", &EstimateProbeTopologyFlow,
         pybind11::return_value_policy::copy);
}

} // namespace procedural
} // namespace e8
