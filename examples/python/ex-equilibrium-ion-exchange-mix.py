# Reaktoro is a unified framework for modeling chemically reactive systems.
#
# Copyright © 2014-2021 Allan Leal
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library. If not, see <http://www.gnu.org/licenses/>.

# -----------------------------------------------------------------------------
# 👏 Acknowledgements 👏
# -----------------------------------------------------------------------------
# This example was originally authored by:
#   • Svetlana Kyas (23 November 2021)
#
# and since revised by:
#   • Allan Leal (28 August 2023)
#     - Using ActivityModelPhreeqc instead of ActivityModelHKF for aqueous phase.
# -----------------------------------------------------------------------------


from reaktoro import *

db = PhreeqcDatabase("phreeqc.dat")

# Define an aqueous phase
solution = AqueousPhase("H2O Na+ Cl- H+ OH- K+ Ca+2")
solution.set(ActivityModelPhreeqc(db))

# Define an ion exchange phase
exchange = IonExchangePhase("CaX2 KX NaX")
exchange.set(ActivityModelIonExchangeVanselow())

# Create chemical system
system = ChemicalSystem(db, solution, exchange)

T = 25.0 # temperature in celsius
P = 1.0  # pressure in bar

# Define initial equilibrium state
state = ChemicalState(system)
state.temperature(T, "celsius")
state.pressure(P, "atm")

# To match the PHREEQC script:
# ---------------------------------------------------------------
# EXCHANGE 1
# CaX2 0.4065 # exchangeable Ca and K in mol
# KX 0.1871
# SOLUTION 1
# Na 1.2; Cl 1.2 charge # Na in solution, exchanges for K and Ca
# MIX 1; 1 1e-9 # ...Take 1e-9 of solution 1 = 1 μg water
# END
# ---------------------------------------------------------------

mix_scale = 1e-9
state.set("H2O" , 1.00 * mix_scale, "kg")
state.set("Na+" , 1.20 * mix_scale, "mmol")
state.set("Cl-" , 1.20 * mix_scale, "mmol")
# Define exchange species
state.set("KX"  , 0.1870, "mol")
state.set("CaX2", 0.4065, "mol")

# Define equilibrium solver and equilibrate given initial state with input conditions
solver = EquilibriumSolver(system)
solver.solve(state)

# Calculate aqueous and exchange properties
aqprops = AqueousProps(state)
exprops = IonExchangeProps(state)

print(aqprops)
print(exprops)

print("I  = %f mol/kgw" % float(aqprops.ionicStrength()))
print("pH = %f"         % float(aqprops.pH()))
print("pE = %f\n"       % float(aqprops.pE()))

print("n(CaX2) = %e mole"   % float(exprops.speciesAmount("CaX2")))
print("n(KX)   = %e mole"   % float(exprops.speciesAmount("KX")))
print("n(NaX)  = %e mole\n" % float(exprops.speciesAmount("NaX")))

print("m(Ca+2) = %e molal"   % float(aqprops.speciesMolality("Ca+2")))
print("m(Cl-)  = %e molal"   % float(aqprops.speciesMolality("Cl-")))
print("m(K+)   = %e molal"   % float(aqprops.speciesMolality("K+")))
print("m(Na+)  = %e molal\n" % float(aqprops.speciesMolality("Na+")))
