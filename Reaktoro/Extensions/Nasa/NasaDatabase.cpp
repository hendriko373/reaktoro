// Reaktoro is a unified framework for modeling chemically reactive systems.
//
// Copyright (C) 2014-2020 Allan Leal
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

#include "NasaDatabase.hpp"

// C++ includes
#include <fstream>
#include <sstream>

// CMakeRC includes
#include <cmrc/cmrc.hpp>

CMRC_DECLARE(ReaktoroDatabases);

// Reaktoro includes
#include <Reaktoro/Extensions/Nasa/NasaDatabaseParseUtils.hpp>
#include <Reaktoro/Extensions/Nasa/NasaSpeciesUtils.hpp>

namespace Reaktoro {

/// Return the contents of the embedded NASA database with given name (or empty)
auto getNasaDatabaseContent(String name) -> String
{
    error(!oneof(name, "cea", "cea-improved", "burcat"),
        "Could not load embedded NASA database file with name `", name, "`. ",
        "The currently supported names are: \n"
        "    - cea    \n",
        "    - cea-improved \n",
        "    - burcat \n",
        "");
    auto fs = cmrc::ReaktoroDatabases::get_filesystem();
    auto contents = fs.open("databases/nasa/" + name + ".dat");
    return String(contents.begin(), contents.end());
}

NasaDatabase::NasaDatabase()
: Database()
{}

NasaDatabase::NasaDatabase(String name)
: NasaDatabase(NasaDatabase::withName(name))
{}

auto NasaDatabase::withName(String name) -> NasaDatabase
{
    const String content = getNasaDatabaseContent(name);
    std::istringstream stream(content);
    return fromStream(stream);
}

auto NasaDatabase::fromFile(String path) -> NasaDatabase
{
    std::ifstream stream(path);
    error(!stream.is_open(), "Could not open NASA database file with given path: ", path);
    return fromStream(stream);
}

auto NasaDatabase::fromStream(std::istream& stream) -> NasaDatabase
{
    const auto lines = NasaUtils::createTextLines(stream);
    const auto specieslist = NasaUtils::createNasaSpeciesVector(lines);

    NasaDatabase db;

    for(auto const& species : specieslist)
        db.addSpecies(NasaUtils::convertSpecies(species));

    return db;
}

} // namespace Reaktoro