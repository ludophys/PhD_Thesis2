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

run_nb = ['3131']#, '3132', '3133', '3134']
intervals = ['0-200']#, '101-301', '101-201', '202-302']
#intervals = [ '202-302']

#Definition of binnings for histo

fluctbins = np.linspace(8e-8, 8e-7, 200)
enebins = np.linspace(0, 10000, 200)
enebinskev = np.linspace(0, 8, 60)
timebins = np.linspace(0, 4, 80)

Q_cum = []
t03 = []
t07 = []
fluct = []
wd_func = 'coif'

Q_init_cum = []
wf = []

wfplot = True

main_path = '/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon'


for i in range(len(run_nb)):
    #main_path = '/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/'+run_nb[i]

    for j in range(len(intervals)):
        #try:
        Q_init_file = np.loadtxt(main_path+"/Q_Ar_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_"+wd_func+".npy")
        Q_init_cum.extend(Q_init_file)
        Q_file = np.loadtxt(main_path+"/Q_den_Ar_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_"+wd_func+".npy")
        Q_cum.extend(Q_file)
        print(len(Q_file))
        t03_file = np.loadtxt(main_path+"/t03_Ar_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_"+wd_func+".npy")

        t03.extend(t03_file)
        t07_file = np.loadtxt(main_path+"/t07_Ar_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_"+wd_func+".npy")
        t07.extend(t07_file)

        if wfplot==True:
            wf_file = np.loadtxt(main_path+"/wf_Ar_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_"+wd_func+".npy")
            print('Path wf : ', main_path+"/wf_Ar_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_"+wd_func+".npy")
            wf.extend(wf_file)

        fluct_file = np.loadtxt(main_path+"/fluct_Ar_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_"+wd_func+".npy")
        fluct.extend(fluct_file)
        #except:
            #continue

#Find the 5.9 keV peak and do the conversion from ADC counts to keV

Q_init_cum = np.array(Q_init_cum)
wf = np.array(wf)
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


plt.legend(title='Argon', frameon=True, fontsize=11, framealpha=0.9)

plt.tight_layout()
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

plt.figure(figsize=(5,4))

h = plt.hist2d(fluct, Q_cum * 5.9/to_kev,
               bins=[fluctbins, enebinskev],
               cmap='viridis',
               norm=LogNorm())

cutmax_fluct = mu2 + 7*sigma2
cutmin_fluct = mu2 - 7*sigma2
plt.axvline(cutmax_fluct, color='red', linestyle='-', linewidth=1.5,
            label=r'$\mu + 7\sigma$')

plt.xlabel('Cs fluctuation', fontsize=13)
plt.ylabel('Energy (keV)', fontsize=13)

plt.xscale('log')

plt.tick_params(axis='both', which='major', direction='in',
                top=True, right=True, labelsize=11)

cbar = plt.colorbar()
cbar.set_label('Counts', fontsize=12)

plt.legend(title='Argon', frameon=True, fontsize=11, framealpha=0.9)

plt.tight_layout()
plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/TOT_fluct2D_"+wd_func+".pdf", dpi=300, bbox_inches="tight")
#print("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/fluct2D_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf")
plt.show()


#m1 = (fluct >= mu2 - 3*sigma2)
m2 = (fluct <= cutmax_fluct)
m3 = m2 & (diff >= mu - 3*sigma)
m4 = m3 & (diff <= mu + 3*sigma)

#m5 = m4 & (np.max(wf) >= 3)

#mask_charge = (fluct >= mu2 - 3*sigma2) & (fluct <= mu2 + 3*sigma2) & (diff >= mu - 3*sigma) & (diff <= mu + 3*sigma) #& (np.max(wf) >= 3)
mask_charge = (fluct <= cutmax_fluct) & (diff >= mu - 3*sigma) & (diff <= mu + 3*sigma) #& (np.max(wf) >= 3)


Q_tot = len(Q_init_cum)
print('Number of events : ', Q_tot)
plt.figure(figsize=(7,5))

plt.hist(Q_init_cum * 5.9/to_kev, enebinskev,
         histtype='step', linewidth=2, color='k',
         label='Before WD')

plt.hist(Q_cum * 5.9/to_kev, enebinskev,
         histtype='step', linewidth=2, color='C0',
         label='After WD')

plt.hist(Q_cum[m2] * 5.9/to_kev, enebinskev,
         histtype='step', linewidth=2, color='C1',
         label=f'Cut 1 ({100*len(Q_cum[~m2])/Q_tot:.1f}% removed)')

plt.hist(Q_cum[m3] * 5.9/to_kev, enebinskev,
         histtype='step', linewidth=2, color='C2',
         label=f'Cut 2 ({100*len(Q_cum[~m3])/Q_tot:.1f}% removed)')

plt.hist(Q_cum[m4] * 5.9/to_kev, enebinskev,
         histtype='step', linewidth=2, color='C3',
         label=f'Cut 3 ({100*len(Q_cum[~m4])/Q_tot:.1f}% removed)')

plt.axvline(1.49, color='0.3', linestyle='--', linewidth=1.5)
plt.axvline(2.90, color='0.3', linestyle='--', linewidth=1.5)
plt.axvline(5.90, color='0.3', linestyle='--', linewidth=1.5)

plt.text(1.49, 3e2, '1.49 keV', rotation=90, fontsize=10,
         ha='right', va='center')
plt.text(2.9, 3e2, '2.90 keV', rotation=90, fontsize=10,
         ha='right', va='center')
plt.text(5.90, 3e2, '5.90 keV', rotation=90, fontsize=10,
         ha='right', va='center')

plt.xlabel('Energy spectrum (keV)', fontsize=15)
plt.ylabel('Entries', fontsize=15)

plt.yscale('log')

plt.xticks(fontsize=13)
plt.yticks(fontsize=13)

plt.tick_params(direction='in', which='both',
                top=True, right=True,
                length=6, width=1.2)

plt.minorticks_on()

plt.grid(axis='y', which='both', alpha=0.25)

plt.legend(title='Argon', frameon=True, fontsize=11, loc='lower right', framealpha=0.9)

plt.tight_layout()

np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/TOT_charge_"+wd_func+".npy", Q_cum[m4] * 5.9/to_kev)

plt.savefig("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/TOT_charge_"+wd_func+".pdf", dpi=300, bbox_inches="tight")
#print("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/charge_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".pdf")
plt.show()

if wfplot==True:
    size_plot = 8
    row = 1
    col = 2
    fig, axes = plt.subplots(row, col, figsize=(size_plot * col ,size_plot * row), constrained_layout=True)
    axes = axes.flatten()

    mask_ene = ((Q_cum * 5.9/to_kev) >= 1.3) & ((Q_cum * 5.9/to_kev) <= 1.6)
    t = np.linspace(0, 40, 5000)

    print('len wf is :', len(wf))

    c3 = m4 & mask_ene #& (t[np.argmax(wf, axis=1)]>=20)
    wfplot3 = wf[c3][:50000]
    t_broadcast = np.broadcast_to(t[:, np.newaxis], wfplot3.T.shape)
    axes[1].hist2d(t_broadcast.flatten(), wfplot3.T.flatten(), bins=[200, 200], cmap='viridis', norm=LogNorm())

    axes[1].set_xlabel("Time ($\mu$s)", fontsize = 25)
    axes[1].set_ylabel("Charge (pes)", fontsize = 25)
    axes[1].tick_params(axis='both', labelsize=20)

    plt.show()



