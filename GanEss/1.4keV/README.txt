The script noise_analysis.sh will launch automatically the scripts, choose your parameters and everything is automatic then. They are separated since we threat a large amount of data, we don't want to threat everything at the same time to not crash.

denoised.py is used for wf calibration, baseline sub, denoising with wavelet decomposition, saving of the different variables

fluct_study.py will callculate the cumulative sum ratio and save it

time_generation.py will generate the variables t0x and t0y that are the times at x% and y% of the cumsum ratio, mainly for rising time characterization (the times can be changed depending on what is optimized)

full_analysis.py contains all the previous 3 scripts but doesn't save denoised waveform (save a lot of memory)

All the steps are for one sample of events since we cannot execute it for entire datasets. When it has been executed for several sample of the dataset, it is possible to group everything and make the final energy pectrum using charge_total.py. It loads the datasets generated previously and group everything.
