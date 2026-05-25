# Here we plot the accumulated data

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

#Reading of input param from source code noise_analysis.sh
import sys

run_nb = ['3131', '3132', '3133, 3134']
intervals = ['0-100', '101-301', '101-201', '202-302']

#Definition of binnings for histo

fluctbins = np.linspace(8e-8, 8e-7, 200)
enebins = np.linspace(0, 10000, 200)
enebinskev = np.linspace(0, 8, 60)
timebins = np.linspace(0, 4, 80)

Q_cum = []
t03 = []
t07 = []
fluct = []
wd_func = 'db'


for i in range(len(run_nb)):
    for j in range(len(intervals)):
        try:
            Q_file = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/"+run_nb[i]+"/Q_Ar_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_db.npy")
            Q_cum.extend(Q_file)
            t03_file = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/"+run_nb[i]+"/t03_Ar_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_db.npy")
            print(t03)
            t03.extend(t03_file)
            t07_file = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/"+run_nb[i]+"/t07_Ar_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_db.npy")
            t07.extend(t07_file)

            fluct_file = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/"+run_nb[i]+"/fluct_Ar_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_db.npy")
            fluct.extend(fluct_file)
        except:
            continue

#Find the 5.9 keV peak and do the conversion from ADC counts to keV
Q_cum = np.array(Q_cum)
mask_to_kev = (Q_cum>=2000)
counts, bins, __ = plt.hist(Q_cum[mask_to_kev], bins=enebins)
plt.show()
bin_centers = (bins[:-1] + bins[1:]) / 2

to_kev = bin_centers[np.argmax(counts)]
#print('to kev is :', to_kev)


t03 = np.array(t03)
t07 = np.array(t07)
diff = (t07 - t03) * 40/5000

fluct = np.array(fluct)
#print('len(diff) is :', len(diff))
#print('len(fluct) is :', len(fluct))



plt.hist(diff, bins=timebins)
plt.show()

plt.hist(fluct, bins=fluctbins)
plt.show()

# Gaussian fitting diff variable
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
#plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/diff_fit1D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf", dpi=300, bbox_inches="tight")
#print("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/diff_fit1D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf")
plt.show()


# 2d histo diff variable

h = plt.hist2d(diff, Q_cum * 5.9/to_kev, bins=[timebins, enebinskev], cmap='viridis', norm=LogNorm())
plt.xlabel('diff (t07-t03 in $\mu$s)')
plt.ylabel('Energy (keV)')
plt.axvline(mu + 3*sigma, color='orange', linestyle='--', label='$\mu$ + 3$\sigma$')
plt.axvline(mu - 3*sigma, color='red', linestyle='--', label='$\mu$ - 3$\sigma$')
plt.legend()
plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/TOT_diff2D_"+wd_func+".pdf", dpi=300, bbox_inches="tight")
#print("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/diff2D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf")
plt.show()


# Gaussian fitting fluct variable
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
#plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/fluct_fit1D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf", dpi=300, bbox_inches="tight")
#print("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/fluct_fit1D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf")
plt.show()

#2d histo fluct variable

### 6th plot (We don't save anything)

h = plt.hist2d(fluct, Q_cum * 5.9/to_kev, bins=[fluctbins, enebinskev], cmap='viridis', norm=LogNorm())
plt.xlabel('fluct')
plt.axvline(mu2 - 3*sigma2, color='orange', linestyle='--', label='$\mu$ - 3$\sigma$')
plt.axvline(mu2 + 3*sigma2, color='red', linestyle='--', label='$\mu$ + 3$\sigma$')
plt.ylabel('Energy (keV)')
plt.xscale('log')
plt.legend()
plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/TOT_fluct2D_"+wd_func+".pdf", dpi=300, bbox_inches="tight")
#print("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/fluct2D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf")
plt.show()


m1 = (fluct >= mu2 - 3*sigma2)
m2 = m1 & (fluct <= mu2 + 3*sigma2)
m3 = m2 & (diff >= mu - 3*sigma)
m4 = m3 & (diff <= mu + 3*sigma)
#m5 = m4 & (np.max(wf) >= 3)

mask_charge = (fluct >= mu2 - 3*sigma2) & (fluct <= mu2 + 3*sigma2) & (diff >= mu - 3*sigma) & (diff <= mu + 3*sigma) #& (np.max(wf) >= 3)



plt.hist(Q_cum[mask_charge] * 5.9/to_kev, bins=enebinskev)
Q_tot = len(Q_cum)
plt.hist(Q_cum[m1] * 5.9/to_kev, enebinskev, alpha=0.3, label=f'cut1 ({len(Q_cum[~m1])/Q_tot:.3f}%)')
plt.hist(Q_cum[m2] * 5.9/to_kev, enebinskev, alpha=0.3, label=f'cut2 ({len(Q_cum[~m2])/Q_tot:.3f}%)')
plt.hist(Q_cum[m3] * 5.9/to_kev, enebinskev, alpha=0.3, label=f'cut3 ({len(Q_cum[~m3])/Q_tot:.3f}%)')
plt.hist(Q_cum[m4] * 5.9/to_kev, enebinskev, alpha=0.3, label=f'cut4 ({len(Q_cum[~m4])/Q_tot:.3f}%)')

np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/TOT_charge_"+wd_func+".npy", Q_cum[m4] * 5.9/to_kev)

plt.axvline(1.49, color='red', linestyle='--', label='1.49keV')
plt.axvline(2.9, color='orange', linestyle='--', label='2.9keV')
plt.axvline(5.9, color='green', linestyle='--', label='5.9keV')
plt.xlabel('Energy (keV)')
plt.ylabel('Entries (log)')
plt.yscale('log')
plt.legend()
plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/TOT_charge_"+wd_func+".pdf", dpi=300, bbox_inches="tight")
#print("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/charge_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf")
plt.show()



