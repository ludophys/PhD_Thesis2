import pandas as pd
import numpy  as np
import invisible_cities.io.pmaps_io as pmapio
import invisible_cities.io.dst_io  as dstio

#filepath = '/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/GanESS_simulation/GanESSG4/geantinos.h5'

pieces = ["VESSEL_BODY"]#, "VESSEL_FLANGE1", "VESSEL_FLANGE2", "COVER_FLANGE1", "COVER_FLANGE2", "CYL_ENDCAP1", "CYL_ENDCAP2", "CONE_ENDCAP1", "CONE_ENDCAP2"]
particles_all = pd.DataFrame()
for i in range(len(pieces)):
    filepath = '/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/GanESS_simulation/GanESSG4/'+str(pieces[i])+'.h5'
    particles = pd.read_hdf(filepath, "/MC/particles")
    particles_all = pd.concat([particles_all, particles], ignore_index=True)

particles_convert = particles_all[["initial_x", "initial_y", "initial_z", "initial_volume"]]
particles_convert = particles_convert.rename(columns={"initial_x": "x"})
particles_convert = particles_convert.rename(columns={"initial_y": "y"})
particles_convert = particles_convert.rename(columns={"initial_z": "z"})
particles_convert = particles_convert.rename(columns={"initial_volume": "volume"})

particles_convert.to_csv("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/GanESS_simulation/GanESSG4/analysis/files/hits_g4.csv", index=False)
