### In this script, we denoise wf and we store integral and signal

print('We are in denoised.py')
#1.4keV focus
import pandas as pd
import tables as tb
import glob
import pywt
import numpy as np

import matplotlib        as mpl
import matplotlib.pyplot as plt
import matplotlib.colors as colors
from datetime import date

import sys

run_nb = [sys.argv[1]]

event_min = int(sys.argv[2])
wd_func = str(sys.argv[3])
gas = str(sys.argv[4])
nf = int(sys.argv[5])
main_path = str(sys.argv[6])

date_today = date.today().strftime("%Y-%m-%d")

import gres.database.load_db as db
data_pmt = db.DataPMT('gap', int(run_nb[0]))
print(data_pmt['adc_to_pes'])

calib = np.array(data_pmt['adc_to_pes'].to_list())

nbr_evpr = 1000
t = np.linspace(0, 5000, 5000) #5000 bins of 8ns = 40ms

# True for plotting few wfs
plot = False

print(sys.argv[0])

ene_range_min = [0]
ene_range_max = [10000]

# definition of arrays that store variables

charge = []
WF_save = []
amp_WF = []
denoised_save = []
wf_denoised_save = []

Q_cons = []
sigma_noise = []
sigma_noise_baseline = []

def threshold_cross(y, threshold, min_distance=100):
    y = np.asarray(y)
    diff = y - threshold
    crossings = np.where(np.diff(np.sign(diff)) != 0)[0]

    if len(crossings) == 0:
        return 0

    filtered = [crossings[0]]
    for c in crossings[1:]:
        if c - filtered[-1] >= min_distance:
            filtered.append(c)

    return len(filtered)

cpt_plot = 0
for run in run_nb:
    folder = '/Users/ldonneger/Desktop/PhD_Thesis2/GapData/R'+str(run)+'/raw/'
    files = glob.glob(folder+"/*.h5")
    n_files = len(files)

    event_max = event_min + nf
    it_max = event_max
    if event_max > n_files:
        it_max = n_files-1
        print('event_max is :', it_max)

    #Loop over the nb of file we want to analyze
    for i in range(event_min, it_max): 
    #for i in range(0, 50):

        wf_file = str(files[i])
        print(f'file number : {i*100/(event_max-event_min)}%')
        #print(wf_file)

        with tb.open_file(wf_file, 'r') as h5in:
            n_filelines = h5in.root.RD.pmtrwf.shape[0]
            if n_filelines < 1000:
                continue
          
            #We want to load each WF one by one
            for j in range(0, nbr_evpr):

                    #Here we load each WF one by one, each PMT is separated
                    wvfs =  h5in.root.RD.pmtrwf[j, :, :]

                    wvfs_calib = [0]*5000 #Definition of an array with the good dimensions

                    # Loop over the 7PMTs in order to apply the calib values to the corresponding WFs
                    for pmt in range(0, 7):

                        # We sum the calibrated WFs to obtain the total energy
                        wvfs_calib = wvfs_calib + (wvfs[pmt]/calib[pmt])

                    #Sum of all the lines (7PMT) + we invert the polarity. 
                    #We obtain the summed WF of all the PMTs for one event 

                    #We invert the polarity
                    pmt_rwf    = -wvfs_calib

                    #baseline sub
                    baseline = (t<=500)
                    pmt_rwf_bs = pmt_rwf - np.mean(pmt_rwf[baseline])
                    
                    #We integrate the full WF and we store for each ev
                    tmin = 0
                    tmax = 5000

                    charge_int = (t >= tmin) & (t <= tmax)

                    
                    wf_charge = [np.sum(pmt_rwf_bs[charge_int])]

                    #We apply different cuts below:
                    #We select only WF in a specific energy range
                    #We select only WF with a maximum above the thresh
                    #We select only events containing 1 WF
                    #We remove events that are not entirely saved because of the boarders of the window

                    if (wf_charge >= ene_range_min) & (wf_charge <= ene_range_max):

                        #We save WF that pass cuts
                        WF_save.append(pmt_rwf_bs)

                        # ==========================================================
                        # WAVELET-BASED NOISE REDUCTION
                        #
                        # The waveform is first converted into a NumPy array and
                        # decomposed using a 4-level discrete wavelet transform:
                        #
                        #     coeffs = [cA4, cD4, cD3, cD2, cD1]
                        #
                        # where cA4 contains the coarse/low-frequency information
                        # and cD4, cD3, cD2, cD1 contain detail information at
                        # progressively finer time scales.
                        #
                        # The noise level is estimated independently for each detail
                        # level using only the baseline region of the original
                        # waveform. Because the wavelet coefficients are downsampled
                        # at each level, the baseline length is converted to the
                        # corresponding coefficient-array length using:
                        #
                        #     scale = 2 ** level_number
                        #
                        # For each detail level, the noise standard deviation is
                        # estimated using the Median Absolute Deviation (MAD):
                        #
                        #     sigma_level = median(|noise_coeffs|) / 0.6745
                        #
                        # A separate threshold is then calculated for each level:
                        #
                        #     threshold_level = 3 * sigma_level
                        #
                        # Each detail coefficient array is subsequently processed
                        # using soft thresholding. Coefficients with magnitudes below
                        # the threshold are set to zero, while coefficients above the
                        # threshold are reduced toward zero.
                        #
                        # The approximation coefficients cA4 are kept unchanged.
                        #
                        # The resulting coefficient list is:
                        #
                        #     [cA4, filtered_cD4, filtered_cD3,
                        #      filtered_cD2, filtered_cD1]
                        # # # # # # # # # # # # # # # # # # # # # # # # 

                        denoised = []

                        signal = np.array(pmt_rwf_bs)

                        coeffs = pywt.wavedec(signal, wd_func + '4', level=4)

                        cD1 = coeffs[-1]

                        sigma_noise.append(
                            np.median(np.abs(cD1)) / 0.6745
                        )

                        coeffs_filtered = []
                        coeffs_filtered.append(coeffs[0])

                        n_levels = 4

                        for i_level, c in enumerate(coeffs[1:], start=1):

                            level_number = n_levels - i_level + 1

                            scale = 2 ** level_number

                            noise_start = 0
                            noise_end = int(np.sum(baseline) / scale)

                            noise_start = max(0, noise_start)
                            noise_end = min(len(c), noise_end)

                            noise_coeffs = c[noise_start:noise_end]

                            sigma_level = np.median(
                                np.abs(noise_coeffs)
                            ) / 0.6745

                            if level_number == 1:
                                sigma_noise[-1] = sigma_level

                            threshold_level = 3.0 * sigma_level

                            filtered = pywt.threshold(
                                c,
                                threshold_level,
                                mode='soft'
                            )

                            coeffs_filtered.append(filtered)

                        # We reconstruct the signal with the filtered coeff
                        denoised = pywt.waverec(coeffs_filtered, wd_func+'4')
                       
                        # We test the charge conservation
                        Q_cons.append(np.abs(np.sum(denoised) - np.sum(pmt_rwf_bs))/np.sum(pmt_rwf_bs))

                        #We test the efficiency to reduce noise
                        sigma_noise_baseline.append(np.mean(denoised[baseline]))
                        
                        baseline_mean = np.mean(denoised[baseline])
                        baseline_std = np.std(denoised[baseline])
                        threshold = baseline_mean + 3 * baseline_std

                        if (np.max(denoised) > threshold) & (np.argmax(denoised) < 4000) & (np.argmax(denoised) > 1000):# & (thesh_cross == 2):
                            time_charge = (t>=1000) & (t<=4000)

                            #amp_WF.append(np.max(pmt_rwf_bs))
                            wf_denoised_save.append(denoised)
                            denoised_save.append(np.sum(denoised))
                            
                            #charge.append(np.sum(pmt_rwf_bs))
                            #charge.append(np.sum(denoised))
                        cpt_plot += 1

                        if (plot == True) & (0<=cpt_plot<=100):
                            plt.style.use('default')
                            plt.style.use("seaborn-colorblind")
                            plt.tick_params(direction='in', which='both', top=True, right=True, length=6, width=1.2)
                            plt.minorticks_on()
                            #plt.grid(axis='y', which='both', alpha=0.25)
                            plt.plot(t, pmt_rwf_bs, label='Raw Waveform', color='C1')
                            plt.plot(t, denoised, label='Denoised Waveform', color='C2')
                            #plt.axhline(np.mean(pmt_rwf_bs[baseline]) + 3 * np.std(pmt_rwf_bs[baseline]), color='red')
                            plt.xlabel("Timebin (8ns)")
                            plt.ylabel("Charge (pes)")
                            plt.legend()
                            plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/Paper1/WF_WD.pdf", bbox_inches='tight')
                            plt.show()

import os
os.makedirs(main_path+"/"+str(gas)+"/data_"+str(date_today), exist_ok=True)

#np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/Q_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", charge)
np.savetxt(main_path+"/"+str(gas)+"/data_"+str(date_today)+"/Q_den_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", denoised_save)
np.savetxt(main_path+"/"+str(gas)+"/data_"+str(date_today)+"/wf_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", wf_denoised_save)
#np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/sigma_noise_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", sigma_noise)
#np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/Q_cons_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", Q_cons)
#np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/sigma_noise_bs_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", sigma_noise_baseline)


