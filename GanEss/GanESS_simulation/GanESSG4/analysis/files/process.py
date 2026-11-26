import pandas as pd
import numpy  as np
import invisible_cities.io.pmaps_io as pmapio
import invisible_cities.io.dst_io  as dstio
import sys

pieces = sys.argv[1:]

print("pieces is : ", pieces)

particles_all = pd.DataFrame()

# Loading of generator outputs for each piece
for i in range(len(pieces)):
#for i in range(0,1):
    filepath = '/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/GanESS_simulation/GanESSG4/'+str(pieces[i])+'.h5'
    #filepath = '/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/GanESS_simulation/GanESSG4/geantinos.h5'

    particles = pd.read_hdf(filepath, "/MC/particles")
    print("pieces[i]", pieces[i])
    particles = particles[(particles['initial_volume'] == pieces[i])]

    # Concat of dataset from each piece
    particles_all = pd.concat([particles_all, particles], ignore_index=True)

particles_convert = particles_all[["initial_x", "initial_y", "initial_z", "initial_volume"]]
particles_convert = particles_convert.rename(columns={"initial_x": "x"})
particles_convert = particles_convert.rename(columns={"initial_y": "y"})
particles_convert = particles_convert.rename(columns={"initial_z": "z"})
particles_convert = particles_convert.rename(columns={"initial_volume": "volume"})

# Store in a .csv file to read it later and make plots
#particles_convert.to_csv("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/GanESS_simulation/GanESSG4/analysis/files/hits_g4.csv", index=False)
#particles_convert.to_csv("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/GanESS_simulation/GanESSG4/analysis/files/hits_gas.csv", index=False)
particles_convert.to_csv("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/GanESS_simulation/GanESSG4/analysis/new_version/hits_"+str(pieces[i])+".csv", index=False)


