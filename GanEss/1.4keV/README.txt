Here we have two sets of analysis for noise reduction depending on the gas. Everything is very similar.

The script noise_analysis.sh will launch automatically the scripts. They are separated since we threat a large amount of data, we don't want to threat everything at the same time to not crash.

denoised.py is used for wf calibration, baseline sub, denoising with wavelet decomposition, saving of the different variables

fluct_study.py will callculate the cumulative sum ratio and save it

time_generation.py will generate the variables t0x and t0y that are the times at x% and y% of the cumsum ratio, mainly for rising time characterization (the times can be changed depending on what is optimized)

charge_study.py make all the plots, apply cuts and produced the energy spectrum. It is also possible the plot some wf distribution to see how the signal looks like. 

All the steps are for one sample of events since we cannot execute it for entire datasets. When it has been executed for several sample of the dataset, it is possible the group everything and make the final energy pectrum using charge_total.py. It loads the datasets generated previously and group everything.
