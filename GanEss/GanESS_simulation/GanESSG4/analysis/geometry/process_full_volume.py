import pandas as pd
import numpy  as np


particles_all = pd.DataFrame()

filepath = '/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/GanESS_simulation/GanESSG4/G4_outputs/geantinos_pmt.csv'

particles = pd.read_csv(filepath)
print("len particles", len(particles))

# Store in a .csv file to read it later and make plots
particles.to_csv("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/GanESS_simulation/GanESSG4/analysis/new_version/hits.csv", index=False)


