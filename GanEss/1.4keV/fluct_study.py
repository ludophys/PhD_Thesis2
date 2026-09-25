# here we calculate the variable fluct that calculate how smooth the ratio curve is

print('We are in fluct_study.py')
import pandas as pd
import glob
import pywt
import numpy as np
import matplotlib.pyplot as plt
import sys

run_nb = [sys.argv[1]]
event_min = int(sys.argv[2])
wd_func = str(sys.argv[3])
gas = str(sys.argv[4])
nf = int(sys.argv[5])
main_path = str(sys.argv[6])

from datetime import date
date_today = date.today().strftime("%Y-%m-%d")

event_max = event_min + nf

wf = np.loadtxt(main_path+"/"+str(gas)+"/data_"+str(date_today)+"/wf_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy")
print('wf loaded')

WF_save = np.array(wf)
fluct = []

for i in range(len(WF_save)):
    print('process : ', i/len(WF_save) * 100, '%')
    cumsum_ratio = np.cumsum(WF_save[i])/np.sum(WF_save[i])

    fluct.append(np.var(np.diff(cumsum_ratio)))

np.savetxt(main_path+"/"+str(gas)+"/data_"+str(date_today)+"/fluct_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", fluct)
