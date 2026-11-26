# This script take the G4 output and convert it into a csv table
# with only the relevant informations (it saves a lot of memory)

import pandas as pd
import numpy  as np
import sys

pieces = sys.argv[1:]

print("pieces is : ", pieces)

particles_all = pd.DataFrame()


filepath = '/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/GanESS_simulation/GanESSG4/'+str(pieces[0])+'.h5'
#filepath = '/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/GanESS_simulation/GanESSG4/geantinos.h5'

particles = pd.read_hdf(filepath, "/MC/particles")
print("len particles", len(particles))

particles = particles[(particles['initial_volume'] != "VESSEL_GAS")
                        & (particles['initial_volume'] != "VESSEL")]

particles_convert = particles[["initial_x", "initial_y", "initial_z", "initial_momentum_x", "initial_momentum_y", "initial_momentum_z", "initial_volume"]]
particles_convert = particles_convert.rename(columns={"initial_x": "x"})
particles_convert = particles_convert.rename(columns={"initial_y": "y"})
particles_convert = particles_convert.rename(columns={"initial_z": "z"})
particles_convert = particles_convert.rename(columns={"initial_momentum_x": "px"})
particles_convert = particles_convert.rename(columns={"initial_momentum_y": "py"})
particles_convert = particles_convert.rename(columns={"initial_momentum_z": "pz"})
particles_convert = particles_convert.rename(columns={"initial_volume": "volume"})

# Store in a .csv file to read it later
particles_convert.to_csv("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/GanESS_simulation/GanESSG4/G4_outputs/"+str(pieces[0])+".csv", index=False)




