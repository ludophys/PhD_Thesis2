# Here we make comparison studies between wavelet functions

import pandas as pd
import matplotlib as mpl
import glob
import numpy as np
import matplotlib.pyplot as plt

mpl.rcParams.update({# Figure
    "figure.figsize": (6.2, 6.2),
    "figure.dpi": 150,
    "savefig.dpi": 300,
    "savefig.bbox": "tight",

    # Font
    "font.family": "serif",
    "font.size": 14,

    # Math
    "mathtext.fontset": "stix",
    "mathtext.rm": "STIXGeneral",

    # Axes
    "axes.labelsize": 16,
    "axes.titlesize": 16,
    "axes.linewidth": 1.2,

    # Ticks
    "xtick.labelsize": 13,
    "ytick.labelsize": 13,
    "xtick.direction": "in",
    "ytick.direction": "in",
    "xtick.top": True,
    "ytick.right": True,
    "xtick.major.size": 6,
    "ytick.major.size": 6,
    "xtick.minor.size": 3,
    "ytick.minor.size": 3,

    # Lines
    "lines.linewidth": 2.0,
    "lines.markersize": 6,

    # Legend
    "legend.fontsize": 12,
    "legend.frameon": False,

    # Grid
    "axes.grid": False,
})

#Reading of input param from source code noise_analysis.sh
import sys

print("Arguments received:")
for i, arg in enumerate(sys.argv):
    print(i, repr(arg))

run_nb = sys.argv[1].split(",")
run_nb = [int(x) for x in run_nb]
wd_func = sys.argv[2].split(",")
wd_func = [str(x) for x in wd_func]
dec_level = int(sys.argv[3])
gas = str(sys.argv[4])

nf = 200
main_path="/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV"
event_min = 0


date_today = "2026-09-30"

event_max = int(event_min + nf)


#wd_func = ['haar', 'coif', 'db', 'sym']

fig, ax = plt.subplots()
ax2 = ax.twinx() 

mean_Q = []
mean_noise = []

for k in range(len(wd_func)):

    Q_cons = []
    sigma_noise_bs = []

    for i in range(len(run_nb)):
        print("run_nb is :", run_nb[i])
        print("wd_func is :", wd_func[k])

        post_path = str(gas)+"_"+str(run_nb[i])+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func[k]+str(dec_level)+".npy"
        print("post_path is :", post_path)
        sigma_noise_bs_file = np.loadtxt(main_path+"/"+str(gas)+"/data_"+str(date_today)+"/sigma_noise_bs_"+post_path)
        sigma_noise_bs.extend(sigma_noise_bs_file)

        Q_cons_file = np.loadtxt(main_path+"/"+str(gas)+"/data_"+str(date_today)+"/Q_cons_"+post_path)
        Q_cons.extend(Q_cons_file)

    mean_Q.append(np.mean(Q_cons))
    mean_noise.append(np.mean(sigma_noise_bs))


ax.plot(wd_func, mean_Q, ls='', marker='x', color='blue', label='Charge error')
ax2.plot(wd_func, mean_noise, ls='', marker='+', color='red', label='Noise level')

ax.set_title('Wavelet function comparisons (dec level = '+str(dec_level)+')')
ax.set_xlabel('Wavelet function')

ax.set_ylabel('Charge error', color='blue')
#ax.set_ylim(0.1e-3, 5e-3)
ax.tick_params(axis='y', colors='blue')
ax.ticklabel_format(axis='y', style='sci', scilimits=(0,0))

ax2.set_ylabel('Noise level (ADC)', color='red')
ax2.tick_params(axis='y', colors='red')
ax2.ticklabel_format(axis='y', style='sci', scilimits=(0,0))

plt.xticks(rotation=45)
plt.tight_layout()
plt.show()


