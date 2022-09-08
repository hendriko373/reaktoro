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

#include "ReactionRateModelPalandriKharaka.hpp"

// Reaktoro includes
#include <Reaktoro/Common/Algorithms.hpp>
#include <Reaktoro/Common/Constants.hpp>
#include <Reaktoro/Common/Exception.hpp>
#include <Reaktoro/Serialization/Models.YAML.hpp>

namespace Reaktoro {
namespace detail {

using Catalyst = ReactionRateModelParamsPalandriKharaka::Catalyst;
using Mechanism = ReactionRateModelParamsPalandriKharaka::Mechanism;

/// Construct a function that computes the activity-based contribution of a catalyst in the mineral reaction rate.
auto mineralCatalystFnActivity(Catalyst const& catalyst, PhaseList const& phases) -> Fn<real(ChemicalProps const&)>
{
    auto const& formula = catalyst.formula;
    auto const& power = catalyst.power;

    auto const iaqueousPhase = phases.indexWithAggregateState(AggregateState::Aqueous);
    auto const iaqueousSpecies = phases[iaqueousPhase].species().indexWithFormula(formula);
    auto const ispecies = phases.numSpeciesUntilPhase(iaqueousPhase) + iaqueousSpecies;

    auto fn = [=](ChemicalProps const& props)
    {
        auto const& ai = props.speciesActivity(ispecies);
        return pow(ai, power);
    };

    return fn;
}

/// Construct a function that computes the partial-pressure-based contribution of a catalyst in the mineral reaction rate.
auto mineralCatalystFnPartialPressure(Catalyst const& catalyst, PhaseList const& phases) -> Fn<real(ChemicalProps const&)>
{
    auto const& formula = catalyst.formula;
    auto const& power = catalyst.power;

    auto const igaseousPhase = phases.indexWithAggregateState(AggregateState::Gas);
    auto const igaseousSpecies = phases[igaseousPhase].species().indexWithFormula(formula);
    auto const ispecies = phases.numSpeciesUntilPhase(igaseousPhase) + igaseousSpecies;

    auto fn = [=](ChemicalProps const& props)
    {
        auto const P  = props.pressure(); // pressure in Pa
        auto const xi = props.speciesMoleFraction(ispecies);
        auto const Pi = xi * P * 1e-5; // partial pressure in bar!
        return pow(Pi, power);
    };

    return fn;
}

/// Construct a function that computes the contribution of a catalyst in the mineral reaction rate.
auto mineralCatalystFn(Catalyst const& catalyst, PhaseList const& phases) -> Fn<real(ChemicalProps const&)>
{
    if(catalyst.property == "a")
        return mineralCatalystFnActivity(catalyst, phases);
    if(catalyst.property == "P")
        return mineralCatalystFnPartialPressure(catalyst, phases);
    errorif(true, "Expecting mineral catalyst property symbol to be either `a` or `P`, but got `", catalyst.property, "` instead.");
}

struct Foo {};

auto mineralMechanismFn(Mechanism const& mechanism, PhaseList const& phases) -> Fn<real(MineralReactionRateArgs)>
{
    // The universal gas constant (in kJ/(mol*K))
    const auto R = universalGasConstant * 1e-3;

    // Create the mineral catalyst functions
    Vec<Fn<real(ChemicalProps const&)>> catalyst_fns;
    for(auto&& catalyst : mechanism.catalysts)
        catalyst_fns.push_back(mineralCatalystFn(catalyst, phases));

    // Define the mineral mechanism function
    auto fn = [=](MineralReactionRateArgs args)
    {
        const auto& lgk = mechanism.lgk.value();
        const auto& E = mechanism.E.value();
        const auto& p = mechanism.p.value();
        const auto& q = mechanism.q.value();

        const auto T = args.props.temperature();
        const auto k0 = pow(lgk, 10.0);
        const auto k = k0 * exp(-E/R * (1.0/T - 1.0/298.15));

        const auto Omega  = args.Omega;
        const auto pOmega = p != 1.0 ? pow(Omega, p) : Omega;
        const auto qOmega = q != 1.0 ? pow(1 - pOmega, q) : 1 - pOmega;

        real g = 1.0;
        for(auto&& catalystfn : catalyst_fns)
            g *= catalystfn(args.props);

        return k * qOmega * g;
    };

    return fn;
}

} // namespace detail

auto ReactionRateModelPalandriKharaka(ReactionRateModelParamsPalandriKharaka const& params) -> MineralReactionRateModelGenerator
{
    MineralReactionRateModelGenerator model = [=](String const& mineral, PhaseList const& phases)
    {
        Vec<Fn<real(MineralReactionRateArgs)>> mechanism_fns;
        for(auto const& mechanism : params.mechanisms)
            mechanism_fns.push_back(detail::mineralMechanismFn(mechanism, phases));

        MineralReactionRateModel fn = [=](MineralReactionRateArgs args) -> Rate
        {
            const auto area = args.area;
            real sum = 0.0;
            for(auto&& mechanismfn : mechanism_fns)
                sum += mechanismfn(args);
            return area * sum;
        };

        return fn;
    };

    return model;
}

auto ReactionRateModelPalandriKharaka(Vec<ReactionRateModelParamsPalandriKharaka> const& paramsvec) -> MineralReactionRateModelGenerator
{
    MineralReactionRateModelGenerator model = [=](String const& mineral, PhaseList const& phases)
    {
        const auto idx = indexfn(paramsvec, RKT_LAMBDA(x, contains(x.names, mineral)));
        errorif(idx >= paramsvec.size(), "Could not find a mineral with name `", mineral, "` in the provided set of Palandri-Kharaka parameters.");
        const auto params = paramsvec[idx];
        return ReactionRateModelPalandriKharaka(params)(mineral, phases);
    };

    return model;
}

} // namespace Reaktoro