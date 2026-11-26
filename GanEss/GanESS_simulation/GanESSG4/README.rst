simGanESS: Simulation software for GanESS
==============================================

simGanESS is the simulation software of GanESS. It is based on IC (https://github.com/next-exp/nexus).

To use it, make sure to have geant4 and nexus environment variables set-up:
+ Geant4: :code:`source geant4.sh` in :code:`$G4INSTALL/bin`
+ Add nexus bin directory to your PATH variable and lib directory to LD_LIBRARY_PATH.
  
Compile with SCons and execute with :code:`bin/ganess -b macro_file -n numb_of_events`

For the analysis of geometry and generator

1. launch exec_geom_runs.sh with the good parameters to send geantinos in the desired regions
   (links need to be adapted to yours)

2. launch convert_output.py (convert_output_full_volume.py if you just sent geantinos everywhere)

3. in analysis/geometry/ launch geom_gene_analysis.sh with the good parameters to compare distributions with theoretical contours
   and check your generators
