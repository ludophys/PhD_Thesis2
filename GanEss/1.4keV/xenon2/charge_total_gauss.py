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

run_nb = ['2661', '2662', '2663', '2664']
intervals = ['0-200']#, '101-301', '101-201', '202-302']

#Definition of binnings for histo


enebins = np.linspace(0, 10000, 200)
enebinskev = np.linspace(0, 10, 80)
timebins = np.linspace(0, 4, 80)

Q_cum = []
Q_init_cum = []
t03 = []
t07 = []
fluct = []
wf = []
sigma_noise_bs = []
Q_raw_tot = []
Q_den_tot = []
single_pulse_charges = []
wd_func = 'coif'

wfplot = False
save = True
folder = 'data_08-09'

for i in range(len(run_nb)):
    for j in range(len(intervals)):
        try:
            # Raw charge after first selection
            Q_init_file = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/"+folder+"/Q_Xe_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_"+wd_func+".npy")
            Q_init_cum.extend(Q_init_file)

            # Denoised charge after first selection
            Q_file = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/"+folder+"/Q_den_Xe_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_"+wd_func+".npy")
            Q_cum.extend(Q_file)

            t03_file = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/"+folder+"/t03_Xe_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_"+wd_func+".npy")
            t03.extend(t03_file)

            t07_file = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/"+folder+"/t07_Xe_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_"+wd_func+".npy")
            t07.extend(t07_file)

            #sigma_noise_bs_file = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/"+folder+"/sigma_noise_bs_Xe_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_"+wd_func+".npy")
            #sigma_noise_bs.extend(sigma_noise_bs_file)

            fluct_file = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/"+folder+"/fluct_Xe_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_"+wd_func+".npy")
            fluct.extend(fluct_file)

            if wfplot==True:
                wf_file = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/"+folder+"/wf_Xe_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_"+wd_func+".npy")
                #print(np.shape(wf_file))
                
                wf.extend(wf_file)
        except:
            continue

print('len Q_init_cum is :', len(Q_init_cum))
print('len Q_cum is :', len(Q_cum))
print('len t03 is :', len(t03))
print('len t07 is :', len(t07))
print('len fluct is :', len(fluct))
print('len wf is :', len(wf))

#Find the 5.9 keV peak and do the conversion from ADC counts to keV
wf = np.array(wf)
print('len wf at start is :', wf)
Q_init_cum = np.array(Q_init_cum)
Q_cum = np.array(Q_cum)
mask_to_kev = (Q_cum>=2000)
counts, bins, __ = plt.hist(Q_cum[mask_to_kev], bins=enebins)
plt.show()
bin_centers = (bins[:-1] + bins[1:]) / 2

to_kev = bin_centers[np.argmax(counts)]

t03 = np.array(t03)
t07 = np.array(t07)
diff = (t07 - t03) * 40/5000

fluct = np.array(fluct)

plt.hist(diff, bins=timebins)
plt.xlabel("Time difference ($t_{07}-t_{03}$) ($\mu$s)")
plt.ylabel("Counts")
plt.yscale('log')
plt.show()


fluctbins = np.arange(np.min(fluct), 1e-6, (1e-8))

plt.hist(fluct, bins=fluctbins)
plt.xlabel("Cs fluctuations") 
plt.xscale('log')  
plt.ylabel("Counts")
plt.yscale('log')
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
plt.xlabel("Time difference ($t_{07}-t_{03}$) ($\mu$s)")
plt.ylabel("Counts")
plt.yscale('log')
plt.legend()
plt.show()


# 2d histo diff variable

plt.figure(figsize=(5,4))

h = plt.hist2d(diff, Q_cum * 5.9/to_kev,
               bins=[timebins, enebinskev],
               cmap='viridis',
               norm=LogNorm())

plt.xlabel(r'Time difference ($t_{07}-t_{03}$) ($\mu$s)', fontsize=13)
plt.ylabel('Energy (keV)', fontsize=13)

plt.axvline(mu - 3*sigma, color='red', linestyle='-', linewidth=1.5,
            label=r'$\mu - 3\sigma$')
plt.axvline(mu + 3*sigma, color='red', linestyle='--', linewidth=1.5,
            label=r'$\mu + 3\sigma$')

plt.tick_params(axis='both', which='major', direction='in',
                top=True, right=True, labelsize=11)

cbar = plt.colorbar()
cbar.set_label('Counts', fontsize=12)

plt.legend(title='Xenon',frameon=True, fontsize=11, framealpha=0.9)

plt.tight_layout()
if save == True:
    plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/TOT_diff2D_"+wd_func+".pdf",
                dpi=600, bbox_inches="tight")

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
plt.xlabel("Cs fluctuations")   
plt.ylabel("Counts")
plt.yscale('log')
plt.xscale('log')

plt.legend()
#plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/fluct_fit1D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf", dpi=300, bbox_inches="tight")
#print("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/fluct_fit1D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf")
plt.show()

#2d histo fluct variable

### 6th plot (We don't save anything)

plt.figure(figsize=(5,4))

h = plt.hist2d(fluct, Q_cum * 5.9/to_kev,
               bins=[fluctbins, enebinskev],
               cmap='viridis',
               norm=LogNorm())

cutmax_fluct = mu2 + 7*sigma2
cutmin_fluct = mu2 - 7*sigma2
#plt.axvline(cutmin_fluct, color='red', linestyle='-', linewidth=1.5,
 #           label=r'$\mu - 7\sigma$')
plt.axvline(cutmax_fluct, color='red', linestyle='-', linewidth=1.5,
            label=r'$\mu + 7\sigma$')


plt.xlabel('Cs fluctuation', fontsize=13)
plt.ylabel('Energy (keV)', fontsize=13)

plt.xscale('log')

plt.tick_params(axis='both', which='major', direction='in',
                top=True, right=True, labelsize=11)

cbar = plt.colorbar()
cbar.set_label('Counts', fontsize=12)

plt.legend(title='Xenon',frameon=True, fontsize=11, framealpha=0.9)

plt.tight_layout()
if save == True:
    plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/TOT_fluct2D_"+wd_func+".pdf",
                dpi=600, bbox_inches="tight")

plt.show()

m1 = (fluct >= cutmin_fluct)
m2 = m1 & (fluct <= cutmax_fluct)
m3 = m2 & (diff >= mu - 3*sigma)
m4 = m3 & (diff <= mu + 3*sigma)

mask_charge = (fluct >= cutmin_fluct) & (fluct <= cutmax_fluct) & (diff >= mu - 3*sigma) & (diff <= mu + 3*sigma) #& (np.max(wf) >= 3)

Q_tot = len(Q_init_cum)
print('Number of events : ', Q_tot)

import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

# ============================================================
# PARAMETRES DU DOUBLE FIT 5.9 / 6.5 keV
# ============================================================

centers_1 = np.array([5.9, 6.5])
int_ratio_1 = 1299 / 11142

# ============================================================
# FONCTIONS DE FIT
# ============================================================

def double_gaussian_fixed_ratio(x, A1, mu1, sigma1, sigma2, center2, ratio):
    A2 = A1 * ratio * sigma1 / sigma2
    g1 = A1 * np.exp(-0.5 * ((x - mu1) / sigma1)**2)
    g2 = A2 * np.exp(-0.5 * ((x - center2) / sigma2)**2)
    return g1 + g2

def gaussian_fixed_center(x, A, sigma, center=2.9):
    return A * np.exp(-0.5 * ((x - center) / sigma)**2)

# ============================================================
# PREPARATION DU SPECTRE Q_cum[m4]
# ============================================================

energy_m4 = Q_cum[m4] * 5.9 / to_kev
energy_m4 = energy_m4[np.isfinite(energy_m4)]

# ============================================================
# HISTOGRAMME
# ============================================================

plt.figure(figsize=(7, 7))
use_charge = [Q_init_cum, Q_cum, Q_cum[m1], Q_cum[m2], Q_cum[m4]]

for i in range(len(use_charge)):
    if i == 0:
        label = 'Raw WFs'
    elif i == 1:
        label = 'Denoised WFs'
    else:
        label = f'Cut {i-2} ({100*len(use_charge[i])/Q_tot:.1f}% kept)'
    plt.hist(use_charge[i] * 5.9 / to_kev, enebinskev, histtype='step', linewidth=2, color=f'C{i}', label=label, alpha=0.7)

# ============================================================
# DONNEES DU FIT
# ============================================================

counts, bin_edges = np.histogram(energy_m4, bins=enebinskev)
bin_centers = 0.5 * (bin_edges[:-1] + bin_edges[1:])

# ============================================================
# FIT 1 : DOUBLE GAUSSIENNE 5.9 + 6.5 keV
# ============================================================

mask_1 = (bin_centers > 5.0) & (bin_centers < 7.2)
x_fit_1, y_fit_1 = bin_centers[mask_1], counts[mask_1]

p0_1 = [1000, 5.9, 0.15, 0.15]
bounds_1 = ([0, 5.7, 0.01, 0.01], [np.inf, 6.1, 1.0, 1.0])

popt_1, pcov_1 = curve_fit(
    lambda x, A1, mu1, sigma1, sigma2:
        double_gaussian_fixed_ratio(x, A1, mu1, sigma1, sigma2, centers_1[1], int_ratio_1),
    x_fit_1, y_fit_1, p0=p0_1, bounds=bounds_1, maxfev=50000
)

A1, mu1, sigma1, sigma2 = popt_1
A2 = A1 * int_ratio_1 * sigma1 / sigma2

# ============================================================
# FIT 2 : GAUSSIENNE UNIQUE CENTREE A 1.5 keV
# ============================================================

center_2 = 1.5
mask_2 = (bin_centers > 0.5) & (bin_centers < 1.7)
x_fit_2, y_fit_2 = bin_centers[mask_2], counts[mask_2]

p0_2 = [500, 0.15]
bounds_2 = ([0, 0.01], [np.inf, 1.0])

popt_2, pcov_2 = curve_fit(
    lambda x, A, sigma: gaussian_fixed_center(x, A, sigma, center_2),
    x_fit_2, y_fit_2, p0=p0_2, bounds=bounds_2, maxfev=50000
)

A3, sigma3 = popt_2

# ============================================================
# FIT 3 : GAUSSIENNE UNIQUE CENTREE A 4.5 keV
# ============================================================

center_3 = 4.3
mask_3 = (bin_centers > 3.9) & (bin_centers < 4.5)
x_fit_3, y_fit_3 = bin_centers[mask_3], counts[mask_3]

p0_3 = [500, 0.15]
bounds_3 = ([0, 0.01], [np.inf, 1.0])

popt_3, pcov_3 = curve_fit(
    lambda x, A, sigma: gaussian_fixed_center(x, A, sigma, center_3),
    x_fit_3, y_fit_3, p0=p0_3, bounds=bounds_3, maxfev=50000
)

A4, sigma4 = popt_3

# ============================================================
# COURBES DES FITS
# ============================================================

x_plot_1 = np.linspace(3, 10, 1000)
g1 = A1 * np.exp(-0.5 * ((x_plot_1 - mu1) / sigma1)**2)
g2 = A2 * np.exp(-0.5 * ((x_plot_1 - centers_1[1]) / sigma2)**2)
g_total_1 = g1 + g2

x_plot_2 = np.linspace(0, 5, 1000)
g3 = A3 * np.exp(-0.5 * ((x_plot_2 - center_2) / sigma3)**2)

x_plot_3 = np.linspace(2, 8, 1000)
g4 = A4 * np.exp(-0.5 * ((x_plot_3 - center_3) / sigma4)**2)

plt.plot(x_plot_1, g_total_1, 'k--', linewidth=2.5, label='Double Gaussian 5.9/6.5 keV')
plt.plot(x_plot_1, g1, '--', color='orange', linewidth=1.5, alpha=0.7)
plt.plot(x_plot_1, g2, '--', color='green', linewidth=1.5, alpha=0.7)
plt.plot(x_plot_3, g4, 'm--', linewidth=2.5, label='Gaussian 4.5 keV')

plt.plot(x_plot_2, g3, 'b--', linewidth=2.5, label='Gaussian 1.5 keV')


# ============================================================
# LIGNES DES ENERGIES
# ============================================================

for energy in [1.50, 4.30, 5.90, 6.49]:
    plt.axvline(energy, color='0.3', linestyle='--', linewidth=1.2)

plt.text(1.50, 2.5e2, '1.50 keV', rotation=90, fontsize=9, ha='right')
plt.text(4.30, 2.5e2, '4.30 keV', rotation=90, fontsize=9, ha='right')
plt.text(5.90, 2.5e2, '5.90 keV', rotation=90, fontsize=9, ha='right')
plt.text(6.49, 2.5e2, '6.49 keV', rotation=90, fontsize=9, ha='right')

# ============================================================
# MISE EN FORME
# ============================================================

plt.xlabel('Energy spectrum (keV)', fontsize=15)
plt.ylabel('Entries', fontsize=15)
plt.yscale('log')
plt.xticks(fontsize=13)
plt.yticks(fontsize=13)
plt.tick_params(direction='in', which='both', top=True, right=True, length=6, width=1.2)
plt.minorticks_on()
plt.grid(axis='y', which='both', alpha=0.25)
plt.legend(title='Xenon', frameon=True, fontsize=9, loc='lower right', framealpha=0.9)
plt.ylim(1e1, 1e5)
plt.tight_layout()

# ============================================================
# RESULTATS DU DOUBLE FIT 5.9 / 6.5 keV
# ============================================================

print("====================================================")
print("DOUBLE GAUSSIAN FIT - 5.9 / 6.5 keV")
print("====================================================")
print(f"Centre 1 = {mu1:.4f} keV")
print(f"Sigma 1  = {sigma1:.4f} keV")
print(f"FWHM 1   = {2.3548*sigma1:.4f} keV")
print(f"Centre 2 = {centers_1[1]:.4f} keV (fixé)")
print(f"Sigma 2  = {sigma2:.4f} keV")
print(f"FWHM 2   = {2.3548*sigma2:.4f} keV")
print(f"Amplitude 1 = {A1:.2f}")
print(f"Amplitude 2 = {A2:.2f}")
print(f"Ratio intégrales imposé = {int_ratio_1:.6f}")
print(f"Ratio intégrales réel = {(A2*sigma2)/(A1*sigma1):.6f}")

# ============================================================
# RESULTATS DU FIT 1.5 keV
# ============================================================

print("====================================================")
print("GAUSSIAN FIT - 1.5 keV")
print("====================================================")
print(f"Centre = {center_2:.4f} keV (fixé)")
print(f"Sigma  = {sigma3:.4f} keV")
print(f"FWHM   = {2.3548*sigma3:.4f} keV")
print(f"Amplitude = {A3:.2f}")

# ============================================================
# RESULTATS DU FIT 4.5 keV
# ============================================================

print("====================================================")
print("GAUSSIAN FIT - 4.5 keV")
print("====================================================")
print(f"Centre = {center_3:.4f} keV (fixé)")
print(f"Sigma  = {sigma4:.4f} keV")
print(f"FWHM   = {2.3548*sigma4:.4f} keV")
print(f"Amplitude = {A4:.2f}")




if save == True:
    np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/TOT_charge_"+wd_func+".npy", Q_cum[m4] * 5.9/to_kev)

    plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/TOT_charge_"+wd_func+".pdf", dpi=300, bbox_inches="tight")
plt.show()

if wfplot==True:
    size_plot = 8
    row = 1
    col = 2
    fig, axes = plt.subplots(row, col, figsize=(size_plot * col ,size_plot * row), constrained_layout=True)
    axes = axes.flatten()

    #mask_ene = ((Q_cum * 5.9/to_kev) >= 1.3) & ((Q_cum * 5.9/to_kev) <= 1.6)
    mask_ene = ((single_pulse_charges* 5.9/to_kev) >= 1.3) & ((single_pulse_charges* 5.9/to_kev) <= 1.6)
    t = np.linspace(0, 40, 5000)

    print('len wf is :', len(wf))

    #c3 = m4 & mask_ene
    c3 = mask_ene
    wfplot3 = wf[c3][:50000]
    t_broadcast = np.broadcast_to(t[:, np.newaxis], wfplot3.T.shape)
    axes[1].hist2d(t_broadcast.flatten(), wfplot3.T.flatten(), bins=[200, 200], cmap='viridis', norm=LogNorm())

    axes[1].set_xlabel("Time ($\mu$s)", fontsize = 25)
    axes[1].set_ylabel("Charge (pes)", fontsize = 25)
    axes[1].tick_params(axis='both', labelsize=20)
    plt.show()