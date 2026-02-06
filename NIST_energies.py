#####################################################################################################################
# This code calculates the deposited energy of 3 different particles of the ionosphere (protons, electrons,
# alpha particles) in a silicon sensor of a certain thickness, given a range of initial energies. It then plots the
# results and saves the data in .csv files
#####################################################################################################################

import numpy as np
import pandas as pd
import uproot
from star import electron
from star import ProtonSTARCalculator, ProtonMaterials
from star import AlphaSTARCalculator, AlphaMaterials
import matplotlib.pyplot as plt


#--------------------------------- DEFINE INITIAL ENERGIES (IONOSPHERE) -----------------------------------#

e_energies = np.logspace(-3, 0, 10) # Electron energies in MeV (1 keV - 1 MeV)

p_energies = np.logspace(-2, 1, 10) # Proton energies in MeV (10 keV - 10 MeV)

a_energies = np.logspace(-1, 1, 10) # Alpha energies in MeV (100 keV - 10 MeV)


#--------------------------------------- COMPUTE STOPPING POWERS ------------------------------------------#

e_silicon = electron.PredefinedMaterials.SILICON
e_stopping = electron.calculate_stopping_power(e_silicon, energy = e_energies)

p_silicon = ProtonMaterials.SILICON
p_calculator = ProtonSTARCalculator(p_silicon)
p_stopping = p_calculator.calculate_total_stopping_powers(p_energies)

a_material = AlphaMaterials.SILICON
a_calculator = AlphaSTARCalculator(a_material)
a_stopping = a_calculator.calculate_total_stopping_powers(a_energies)


#------------------------------------ COMPUTE DEPOSITED ENERGIES ------------------------------------------#

thickness = 0.03 # Wafer thickness
density = 2.33000E+00

delta_e = e_stopping["stopping_power_total"] * thickness * density
delta_p = p_stopping * thickness * density
delta_a = a_stopping * thickness * density

#----------------------------------------------- PLOTS ----------------------------------------------------#

plt.figure(figsize=(8,6))
plt.plot(e_energies, delta_e, "--o", label="Electrons", color="teal")
plt.plot(p_energies, delta_p, "--o", label="Protons", color="crimson")
plt.plot(a_energies, delta_a, "--o", label="Helium ions", color="goldenrod")
plt.xlabel("Energy (MeV)")
plt.xscale("log")
plt.yscale("log")
plt.xlabel("Initial Energy [MeV]")
plt.ylabel("Deposited Energy [MeV]")
plt.title("Deposited Energy vs Initial Energy in Spacepix3 sensor (Ionosphere)")
plt.legend()
plt.grid(True, which="both", ls="--", lw=0.5)
plt.show()

#------------------------------------------ SAVE DATA TO CSV ----------------------------------------------#

df_e = pd.DataFrame({
    "Initial Energy (MeV)": e_energies,
    "Stopping Power (MeV cm^2/g)": e_stopping["stopping_power_total"],
    "Transferred Energy (MeV)": delta_e,
})
df_e.to_csv("ionosphere_electron_stopping.csv", index=False)

df_p = pd.DataFrame({
    "Initial Energy (MeV)": p_energies,
    "Stopping Power (MeV cm^2/g)": p_stopping,
    "Transferred Energy (MeV)": delta_p,
})
df_p.to_csv("ionosphere_proton_stopping.csv", index=False)

df_a = pd.DataFrame({
    "Initial Energy (MeV)": a_energies,
    "Stopping Power (MeV cm^2/g)": a_stopping,
    "Transferred Energy (MeV)": delta_a,
})
df_a.to_csv("ionosphere_alpha_stopping.csv", index=False)

#---------------------------------------- ADD SIMULATION DATA --------------------------------------------#

# Path to your ROOT file
root_file = "MergedOutput.root"

# Name of the TTree inside the ROOT
tree_name = "pixelcharge_flattened"

# Open the ROOT file and get the tree
with uproot.open(root_file) as file:
    tree = file[tree_name]

    # Read the entire tree as a pandas DataFrame
    df = tree.arrays(library='pd')

#Now df is a pandas DataFrame with the tree contents
print(df.head())