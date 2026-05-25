# here we generate the variable diff that is t07-t03 in order to study the rising time of each saved wf

print('We are in time_generation.py')
import pandas as pd

import tables as tb
from scipy.integrate import trapz
from scipy.optimize import curve_fit
from scipy.special import erfc
import glob
import pywt
from scipy.stats import gaussian_kde
import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import UnivariateSpline
from matplotlib.colors import LogNorm

import sys

run_nb = [sys.argv[1]]
event_min = int(sys.argv[2])
wd_func = str(sys.argv[3])
gas = str(sys.argv[4])
nf = int(sys.argv[5])
event_max = event_min + nf

wf = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/wf_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy")
print('wf loaded')
t = np.linspace(0, 5000, 5000)

WF_save = np.array(wf)

t03 = []
t07 = []
cpt=0

for i in range(len(WF_save)):
    print('process : ', i/len(WF_save) * 100, '%')
    cumsum_ratio = np.cumsum(WF_save[i])/np.sum(WF_save[i])
    t03.append(t[np.searchsorted(cumsum_ratio, 0.3)])
    t07.append(t[np.searchsorted(cumsum_ratio, 0.7)])
    cpt+=1

    fig, ax1 = plt.subplots()

    if cpt < 20:
        
        ax1.plot(t, WF_save[i], label='WF')
        ax1.set_ylabel("Charge (pes)")
        ax1.tick_params(axis='y')
        ax1.axvline(t[np.searchsorted(cumsum_ratio, 0.3)], color='red', linestyle='--', label='t03')
        ax1.axvline(t[np.searchsorted(cumsum_ratio, 0.7)], color='forestgreen', linestyle='--', label='t07')
        ax1.set_xlabel("Timebin 8ns")
        ax1.legend()
        
        ax2 = ax1.twinx()
        ax2.plot(t, cumsum_ratio, color='orange')
        ax2.set_ylabel("CumSum ratio", color='orange')
        ax2.tick_params(axis='y', labelcolor='orange')
    
        plt.show()


np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/t03_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", t03)
np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/t07_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", t07)


