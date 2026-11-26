# This script take the G4 output and convert it into a csv table
# with only the relevant informations (it saves a lot of memory)

import pandas as pd
import numpy  as np
import sys

particles_all = pd.DataFrame()


filepath = '/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/GanESS_simulation/GanESSG4/geantinos.h5'

particles = pd.read_hdf(filepath, "/MC/particles")
print("len particles", len(particles))

particles_convert = particles[["initial_x", "initial_y", "initial_z", "initial_volume"]]
particles_convert = particles_convert.rename(columns={"initial_x": "x"})
particles_convert = particles_convert.rename(columns={"initial_y": "y"})
particles_convert = particles_convert.rename(columns={"initial_z": "z"})
particles_convert = particles_convert.rename(columns={"initial_volume": "volume"})

# Store in a .csv file to read it later
particles_convert.to_csv("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/GanESS_simulation/GanESSG4/G4_outputs/geantinos_pmt.csv", index=False)




