#### In this script we generate plots and save values usefull to make final plots with accumulated data

print('We are in charge_study.py')
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

#Reading of input param from source code moise_analysis.sh
import sys

run_nb = [sys.argv[1]]
event_min = int(sys.argv[2])

wd_func = str(sys.argv[3])
gas = str(sys.argv[4])
nf = int(sys.argv[5])
event_max = event_min + nf

#loading of data
charge = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/Q_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy")
wf = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/wf_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy")
t03 = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/t03_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy")
t07 = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/t07_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy")
fluct = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/fluct_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy")

#Definition of binnings for histo
fluctbins = np.linspace(8e-8, 8e-7, 200)
enebins = np.linspace(0, 10000, 200)
enebinskev = np.linspace(0, 8, 60)
timebins = np.linspace(0, 6, 50)

#Find the 5.9 keV peak and do the conversion from ADC counts to keV

mask_to_kev = (charge>=2000)
counts, bins, __ = plt.hist(charge[mask_to_kev], bins=enebins)
bin_centers = (bins[:-1] + bins[1:]) / 2


to_kev = bin_centers[np.argmax(counts)]
print('to kev is :', to_kev)

#Rising time
diff = (t07-t03) * 40/5000

#### First plot 
c1, b1, _ = plt.hist(diff, bins=timebins)
b1_centers = 0.5 * (b1[:-1] + b1[1:])
histo1 = np.column_stack((c1, b1_centers))

# We cannot process all the data at the same time because of memory so we need to accumulate the histo counts latter
# IMPORTANT : the binnings have to stay fixed for a same set of data, if not we cannot accumulate
np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/diff1D_histo_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", histo1)

plt.xlabel('diff')
plt.ylabel('Entries (log)')
plt.yscale('log')
plt.legend()
plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/diff1D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf", dpi=300, bbox_inches="tight")
print("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/diff1D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf")
plt.show()

### 2nd plot

# Gaussian fitting
def gauss(x, A, mu, sigma):
    return A * np.exp(-(x - mu)**2 / (2 * sigma**2))
counts, bins = np.histogram(diff, bins=timebins)
x = (bins[:-1] + bins[1:]) / 2
xmin = 0.8 #Min value of the window for the gaussian fitting
xmax = 2 #Max value of the window for the gaussian fitting
mask = (x >= xmin) & (x <= xmax)
x_fit = x[mask]
y_fit = counts[mask]

#Input parameters
A0 = np.max(y_fit)
mu0 = x_fit[np.argmax(y_fit)]
sigma0 = 1
p0 = [A0, mu0, sigma0]

#optimised parameters
popt, pcov = curve_fit(gauss, x_fit, y_fit, p0=p0)
A, mu, sigma = popt
fwhm = 2.355 * sigma
sigma = np.abs(sigma)

print("=== Résultats du fit ===")
print(f"Amplitude = {A:.2f}")
print(f"Mu        = {mu:.3f}")
print(f"Sigma     = {sigma:.3f}")
print(f"FWHM      = {fwhm:.3f}")

x_model = np.linspace(xmin, xmax, 500)
y_model = gauss(x_model, *popt)

#We plot everything, the gaussian fitting will need to be used on the final diff data so we don't store anything here
plt.figure(figsize=(8,5))
plt.hist(diff, bins=timebins, alpha=0.6, label="Data")
plt.plot(x_model, y_model, 'r-', lw=2, label=f"Gaussian fit\nAmplitude = {A:.2f}\nMu        = {mu:.3f}\nSigma     = {sigma:.3f}\nFWHM      = {fwhm:.3f}")
plt.axvline(xmin, color='k', ls='--')
plt.axvline(xmax, color='k', ls='--')
plt.xlabel("Valeur")
plt.ylabel("Counts")
#plt.yscale('log')
plt.legend()
plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/diff_fit1D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf", dpi=300, bbox_inches="tight")
print("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/diff_fit1D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf")
plt.show()

### 3rd plot 
# Here again, we don't need to save anything, we will just ahve to plot it in the final step

h = plt.hist2d(diff, charge * 5.9/to_kev, bins=[timebins, enebinskev], cmap='viridis', norm=LogNorm())
plt.xlabel('diff (t07-t03 in $\mu$s)')
plt.ylabel('Energy (keV)')
plt.axvline(mu + 3*sigma, color='orange', linestyle='--', label='$\mu$ + 3$\sigma$')
plt.axvline(mu - 3*sigma, color='red', linestyle='--', label='$\mu$ - 3$\sigma$')
plt.legend()
plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/diff2D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf", dpi=300, bbox_inches="tight")
print("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/diff2D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf")
plt.show()

### 4th plot
# Here we need to save fluc the same way as before

c2, b2, _ = plt.hist(fluct, bins=fluctbins)
b2_centers = 0.5 * (b2[:-1] + b2[1:])
histo2 = np.column_stack((c2, b2_centers))
np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/fluct1D_histo_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", histo2)

plt.xlabel('fluct')
plt.ylabel('Entries (log)')
plt.legend()
plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/fluct1D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf", dpi=300, bbox_inches="tight")
print("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/fluct1D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf")
plt.show()

### 5th plot (We don't save anything)
# Gaussian fitting
counts, bins = np.histogram(fluct, bins=fluctbins)
x = (bins[:-1] + bins[1:]) / 2
xmin = 0.23e-6 #Min value of the window for the gaussian fitting
xmax = 0.45e-6 #Max value of the window for the gaussian fitting
mask = (x >= xmin) & (x <= xmax)
x_fit = x[mask]
y_fit = counts[mask]
A0 = np.max(y_fit)
mu0 = x_fit[np.argmax(y_fit)]
sigma0 = 0.2e-6
p0 = [A0, mu0, sigma0]
popt, pcov = curve_fit(gauss, x_fit, y_fit, p0=p0)
A2, mu2, sigma2 = popt
fwhm2 = 2.355 * sigma2
sigma2 = np.abs(sigma2)

print("=== Résultats du fit ===")
print(f"Amplitude = {A2:.2f}")
print(f"Mu        = {mu2:.3f}")
print(f"Sigma     = {sigma2:.3f}")
print(f"FWHM      = {fwhm2:.3f}")
x_model = np.linspace(xmin, xmax, 500)
y_model = gauss(x_model, *popt)
plt.figure(figsize=(8,5))
plt.hist(fluct, bins=fluctbins, alpha=0.6, label="Data")
plt.plot(x_model, y_model, 'r-', lw=2, label=f"Gaussian fit\nAmplitude = {A2:.2f}\nMu        = {mu2:.3f}\nSigma     = {sigma2:.3f}\nFWHM      = {fwhm2:.3f}")
plt.axvline(xmin, color='k', ls='--')
plt.axvline(xmax, color='k', ls='--')
plt.xlabel("Valeur")
plt.ylabel("Counts")

plt.legend()
plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/fluct_fit1D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf", dpi=300, bbox_inches="tight")
print("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/fluct_fit1D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf")
plt.show()

### 6th plot (We don't save anything)

h = plt.hist2d(fluct, charge * 5.9/to_kev, bins=[fluctbins, enebinskev], cmap='viridis', norm=LogNorm())
plt.xlabel('fluct')
plt.axvline(mu2 - 3*sigma2, color='orange', linestyle='--', label='$\mu$ - 3$\sigma$')
plt.axvline(mu2 + 3*sigma2, color='red', linestyle='--', label='$\mu$ + 3$\sigma$')
plt.ylabel('Energy (keV)')
plt.xscale('log')
plt.legend()
plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/fluct2D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf", dpi=300, bbox_inches="tight")
print("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/fluct2D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf")
plt.show()

#Here we apply all the cuts determined from the several histograms before
# we apply them one by on to see what is the ratio of events rejected by each and what are the effects of each on the final energy spectrum

m1 = (fluct >= mu2 - 3*sigma2)
m2 = m1 & (fluct <= mu2 + 3*sigma2)
m3 = m2 & (diff >= mu - 3*sigma)
m4 = m3 & (diff <= mu + 3*sigma)
m5 = m4 & (np.max(wf) >= 3)

mask_charge = (fluct >= mu2 - 3*sigma2) & (fluct <= mu2 + 3*sigma2) & (diff >= mu - 3*sigma) & (diff <= mu + 3*sigma) & (np.max(wf) >= 3)

chargetot = len(charge)

print('1 : ', len(charge[~m1])/chargetot)
print('2 : ', len(charge[~m2])/chargetot)
print('3 : ', len(charge[~m3])/chargetot)
print('4 : ', len(charge[~m4])/chargetot)
print('5 : ', len(charge[~m5])/chargetot)

c2, b2, _ = plt.hist(fluct, bins=fluctbins)
b2_centers = 0.5 * (b2[:-1] + b2[1:])
histo2 = np.column_stack((c2, b2_centers))
np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/fluct1D_histo_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", histo2)

plt.hist(charge[m1] * 5.9/to_kev, enebinskev, alpha=0.3, label=f'cut1 ({len(charge[~m1])/chargetot:.3f}%)')
plt.hist(charge[m2] * 5.9/to_kev, enebinskev, alpha=0.3, label=f'cut2 ({len(charge[~m2])/chargetot:.3f}%)')
plt.hist(charge[m3] * 5.9/to_kev, enebinskev, alpha=0.3, label=f'cut3 ({len(charge[~m3])/chargetot:.3f}%)')
plt.hist(charge[m4] * 5.9/to_kev, enebinskev, alpha=0.3, label=f'cut4 ({len(charge[~m4])/chargetot:.3f}%)')
plt.hist(charge[m5] * 5.9/to_kev, enebinskev, alpha=0.3, label=f'cut5 ({len(charge[~m5])/chargetot:.3f}%)')

plt.axvline(1.49, color='red', linestyle='--', label='1.49keV')
plt.axvline(2.9, color='orange', linestyle='--', label='2.9keV')
plt.axvline(5.9, color='green', linestyle='--', label='5.9keV')
plt.xlabel('Energy (keV)')
plt.ylabel('Entries (log)')
plt.yscale('log')
plt.legend()
plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/charge_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf", dpi=300, bbox_inches="tight")
print("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/charge_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf")
plt.show()

# Here the commented code is for plotting wf distrib with the cuts, we might want to just show this plot for a sample but it is heavy to produce each time

size_plot = 8
row = 2
col = 4
fig, axes = plt.subplots(row, col, figsize=(size_plot * col ,size_plot * row), constrained_layout=True)
axes = axes.flatten()

mask_ene = (charge * 5.9/to_kev >= 1.3) & (charge * 5.9/to_kev <= 1.6)

t = np.linspace(0, 40, 5000)
#wfplot0 = wf[mask_ene]
#t_broadcast = np.broadcast_to(t[:, np.newaxis], wfplot0.T.shape)

#axes[0].hist2d(t_broadcast.flatten(), wfplot0.T.flatten(), bins=[200, 200], cmap='viridis', norm=LogNorm())

#axes[0].set_xlabel("Time ($\mu$s)")
#axes[0].set_ylabel("Charge (pes)")

#c1 = mask_ene + m1
#wfplot1 = wf[c1]
#t_broadcast = np.broadcast_to(t[:, np.newaxis], wfplot1.T.shape)
#axes[1].hist2d(t_broadcast.flatten(), wfplot1.T.flatten(), bins=[200, 200], cmap='viridis', norm=LogNorm())

#axes[1].set_xlabel("Time ($\mu$s)")
#axes[1].set_ylabel("Charge (pes)")

#c2 = mask_ene + m2
#wfplot2 = wf[c2]
#t_broadcast = np.broadcast_to(t[:, np.newaxis], wfplot2.T.shape)
#axes[2].hist2d(t_broadcast.flatten(), wfplot2.T.flatten(), bins=[200, 200], cmap='viridis', norm=LogNorm())

#axes[2].set_xlabel("Time ($\mu$s)")
#axes[2].set_ylabel("Charge (pes)")


c3 = mask_ene & m4
wfplot3 = wf[c3]
t_broadcast = np.broadcast_to(t[:, np.newaxis], wfplot3.T.shape)
axes[3].hist2d(t_broadcast.flatten(), wfplot3.T.flatten(), bins=[200, 200], cmap='viridis', norm=LogNorm())

axes[3].set_xlabel("Time ($\mu$s)", fontsize=14)
axes[3].set_ylabel("Charge (pes)", fontsize=14)

plt.show()
