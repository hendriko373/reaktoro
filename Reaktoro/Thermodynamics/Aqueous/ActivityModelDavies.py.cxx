// Reaktoro is a unified framework for modeling chemically reactive systems.
//
// Copyright © 2014-2022 Allan Leal
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.

// pybind11 includes
#include <Reaktoro/pybind11.hxx>

// Reaktoro includes
#include <Reaktoro/Thermodynamics/Aqueous/ActivityModelDavies.hpp>
using namespace Reaktoro;

void exportActivityModelDavies(py::module& m)
{
    py::class_<ActivityModelDaviesParams>(m, "ActivityModelDaviesParams")
        .def(py::init<>())
        .def_readwrite("bions", &ActivityModelDaviesParams::bions)
        .def_readwrite("bneutrals", &ActivityModelDaviesParams::bneutrals)
        ;

    m.def("ActivityModelDavies", py::overload_cast<>(ActivityModelDavies));
    m.def("ActivityModelDavies", py::overload_cast<ActivityModelDaviesParams>(ActivityModelDavies));
}