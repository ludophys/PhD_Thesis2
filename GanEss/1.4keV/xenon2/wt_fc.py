# Here we plot the accumulated data

import pandas as pd
import matplotlib as mpl
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

mpl.rcParams.update({

    # Figure
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

run_nb = ['2661', '2662']#, '2663']
intervals = ['0-200']#, '0-300']#, '101-301', '101-201', '202-302']



wd_func = ['coif', 'db', 'sym']

wfplot = True
Q_cons_dst = pd.DataFrame({'coif', 'db', 'sym'})
sigma_noise_bs_dst = pd.DataFrame({'coif', 'db', 'sym'})
fig, ax = plt.subplots()
ax2 = ax.twinx()      # <-- créé UNE SEULE FOIS

mean_Q = []
mean_noise = []

for k in range(len(wd_func)):

    Q_cons = []
    sigma_noise_bs = []

    for i in range(len(run_nb)):
        for j in range(len(intervals)):

            try:
                sigma_noise_bs_file = np.loadtxt(
                    "/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/"
                    + "sigma_noise_bs_Xe_['" + run_nb[i] + "']_evts_["
                    + intervals[j] + "]_" + wd_func[k] + ".npy"
                )
                sigma_noise_bs.extend(sigma_noise_bs_file)

                Q_cons_file = np.loadtxt(
                    "/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/"
                    + "Q_cons_Xe_['" + run_nb[i] + "']_evts_["
                    + intervals[j] + "]_" + wd_func[k] + ".npy"
                )
                Q_cons.extend(Q_cons_file)

            except:
                continue

    mean_Q.append(np.mean(Q_cons))
    mean_noise.append(np.mean(sigma_noise_bs))

# Tracé UNE SEULE FOIS
#code pour faire le meme plot que en dessous mais dans un format pret pour une publication de physique


ax.plot(wd_func, mean_Q, ls='', marker='o', color='blue', label='Charge error')
ax2.plot(wd_func, mean_noise, ls='', marker='s', color='red', label='Noise level')

# Mise en forme
ax.set_title('Wavelet function comparisons')
ax.set_xlabel('Wavelet function')

ax.set_ylabel('Charge error', color='blue')
ax.set_ylim(0.7e-2, 2.5e-2)
ax.tick_params(axis='y', colors='blue')
ax.ticklabel_format(axis='y', style='sci', scilimits=(0,0))

ax2.set_ylabel('Noise level (ADC)', color='red')
ax2.tick_params(axis='y', colors='red')
ax2.ticklabel_format(axis='y', style='sci', scilimits=(0,0))

plt.xticks(rotation=45)
plt.tight_layout()
plt.show()


