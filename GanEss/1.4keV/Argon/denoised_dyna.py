### In this script, we denoise wf and we store integral and signal

print('We are in denoised.py')
#1.4keV focus
import pandas as pd

import tables as tb
from scipy.integrate import trapz
from scipy.optimize import curve_fit
from scipy.special import erfc
import glob
import pywt
from scipy.stats import gaussian_kde
import numpy as np
from scipy.interpolate import UnivariateSpline
from matplotlib.colors import LogNorm

import matplotlib        as mpl
import matplotlib.pyplot as plt
import matplotlib.colors as colors

import sys

from scipy.signal import find_peaks

# ==========================================
# Parameters
# ==========================================

n_left = 8000 // 8
integral_bin_right = 20000 // 8

# ==========================================
# Dynamic integration window
# ==========================================

def find_dynamic_window(wf):

    roi = wf[n_left:integral_bin_right]

    peaks, props = find_peaks(
        roi,
        height=1.2,
        distance=30,
        prominence=0.8
    )

    if len(peaks) == 0:
        return n_left, integral_bin_right

    peak = peaks[0]

    # ======================
    # START
    # ======================

    threshold = 0.2 * props['peak_heights'][0]

    left = roi[max(0, peak-50):peak]

    under = np.where(left < threshold)[0]

    if len(under) > 0:

        start = (
            under[-1]
            + max(0, peak-50)
            + n_left
        )

    else:

        start = n_left

    # ======================
    # END
    # ======================

    search_end = min(
        integral_bin_right,
        start + 1500
    )

    if peak + n_left >= search_end:
        return start, search_end

    tail = wf[peak+n_left : search_end]

    if len(tail) < 20:
        return start, search_end

    for i in range(len(tail)-20):

        # stop before second peak
        if len(peaks) > 1:

            if i + peak >= peaks[1]:
                break

        if np.mean(tail[i:i+20]) < 0.1:

            end = peak + n_left + i

            return start, end

    # fallback
    return start, search_end




run_nb = [sys.argv[1]]
event_min = int(sys.argv[2])
wd_func = str(sys.argv[3])
gas = str(sys.argv[4])
nf = int(sys.argv[5])


import gres.database.load_db as db
data_pmt = db.DataPMT('gap', 5000)
print(data_pmt['adc_to_pes'])

calib = np.array(data_pmt['adc_to_pes'].to_list())

nbr_evpr = 1000
t = np.linspace(0, 5000, 5000) #5000 bins of 8ns = 40ms

# True for plotting few wfs
plot = False

#run_nb = [2661] #xenon
print(sys.argv[0])
 #argon

#usefull to focus only on WF producing a specific energy region
#ene_range_min = np.arange(0, 1400, 200)
#ene_range_max = np.arange(200, 1600, 200)


# to study large range energy region
ene_range_min = [0]
#ene_range_min = [756.25] #argon
#ene_range_max = [930.77]

ene_range_min = [0]
ene_range_max = [10000]

# definition of arrays that store variables

charge = []
time = []
amplitude_err = []
WF_save = []
amp_WF = []
denoised_save = []
wf_denoised_save = []
denoised_save_dyna = []
crossings = []
time_cumsum = []
crossings = []

sigma_noise = []


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
                    
                    
                    #print(np.shape(wvfs[0]))

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

                    noise_int = (t <= (tmin - 100)) & (t >= (tmax + 100))
                    noise_int_af = (t >= (tmax + 100))

                    
                    wf_charge = [np.sum(pmt_rwf_bs[charge_int])]
                    
                    wf_charge_cumsum = np.cumsum(pmt_rwf_bs[charge_int])                    
                    
                    
                    #print('time saved is :', t[idx])
                    
                    #We apply different cuts below:
                    #We select only WF in a specific energy range
                    #We select only WF with a maximum above the thresh
                    #We select only events containing 1 WF
                    #We remove events that are not entirely saved because of the boarders of the window

                    if (wf_charge >= ene_range_min) & (wf_charge <= ene_range_max):

                        #We save WF that pass cuts
                        WF_save.append(pmt_rwf_bs)

                        denoised = []
                        
                        #We use a wavelet decomposition to reduce noise
                        signal = np.array(pmt_rwf_bs)
                        coeffs = pywt.wavedec(signal, wd_func+'4', level=4) #multi-scale wavelet decomposition. The list of coeffs is [cA4, cD4, cD3, cD2, cD1]
                        
                        cD1 = coeffs[-1]

                        sigma_noise.append(np.median(np.abs(cD1))/0.6745)

                        threshold = np.mean(pmt_rwf_bs[baseline]) + 3 * np.std(pmt_rwf_bs[baseline]) #

                        coeffs_filtered = []
                        coeffs_filtered.append(coeffs[0]) 

                        # We travel through the details coefficients
                        # We apply a thresholding to remove small coeff and keep only big ones (signal)
                        for c in coeffs[1:]:
                            filtered = pywt.threshold(c, threshold, mode='soft')
                            coeffs_filtered.append(filtered)

                        # We reconstruct the signal with the filtered coeff
                        denoised = pywt.waverec(coeffs_filtered, wd_func+'4')
                        
                        #print('start is :',start, 'end is : ', end)
                        #if (np.max(denoised) > threshold) & (np.argmax(denoised) < 4000) & (np.argmax(denoised) > 1000):
                        if (np.max(denoised) > threshold) & (np.argmax(denoised) < 4000) & (np.argmax(denoised) > 1000):   
                            start, end = find_dynamic_window(denoised)
                            time_dyna = (t>=start) & (t<=end)
                            #time_charge = (t>=np.argmax(denoised) - 350) & (t<=np.argmax(denoised) + 2125)
                            time_charge = (t>=1000) & (t<=4000)

                            amp_WF.append(np.max(pmt_rwf_bs))
                            wf_denoised_save.append(denoised)
                            denoised_save.append(np.sum(denoised))
                            denoised_save_dyna.append(np.sum(denoised[time_dyna]))
                            #Integral in an automatic window is saved
                            #charge.append(np.sum(denoised[time_charge]))
                            #charge.append(np.sum(denoised))
                        cpt_plot += 1
                        if (plot == True) & (0<=cpt_plot<=10):
                            
                            plt.plot(t, pmt_rwf_bs)
                            plt.axvline(start, color='red')
                            plt.axvline(end, color='red')
                            #plt.axhline(np.mean(pmt_rwf_bs[baseline]) + 3 * np.std(pmt_rwf_bs[baseline]), color='red')
                            plt.xlabel("Timebin (8ns)")
                            plt.ylabel("Charge (pes)")
                            plt.show()

enebins = np.linspace(0, 4000, 200)
cdyna, bins_dyna, _ = plt.hist(denoised_save_dyna, bins=enebins, label='dyna', alpha=0.5)
cfix, bins_fix, _ = plt.hist(denoised_save, bins=enebins, label='fixed', alpha=0.5)
bin_centers = (bins_dyna[:-1] + bins_dyna[1:]) / 2
plt.yscale('log')
plt.xlabel('Energy (pes)', fontsize=14)
plt.ylabel('Entries (log)', fontsize=14)
plt.legend()
plt.show()

diff = cdyna - cfix
plt.plot(bin_centers, diff)
plt.show()


np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/Q_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", denoised_save_dyna)

np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/wf_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", wf_denoised_save)
np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/sigma_noise_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", sigma_noise)

