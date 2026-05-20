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
event_max = event_min + 10
wd_func = str(sys.argv[3])
gas = str(sys.argv[4])

wf = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/wf_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy")
print('wf loaded')
t = np.linspace(0, 5000, 5000)

WF_save = np.array(wf)

t03 = []
t07 = []

for i in range(len(WF_save)):
    print('process : ', i/len(WF_save) * 100, '%')
    cumsum_ratio = np.cumsum(WF_save[i])/np.sum(WF_save[i])
    t03.append(t[np.searchsorted(cumsum_ratio, 0.3)])
    t07.append(t[np.searchsorted(cumsum_ratio, 0.7)])

np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/t03_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", t03)
np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/t07_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", t07)


