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

run_nb = ['3131', '3132', '3133, 3134']
intervals = ['0-100', '101-301', '101-201', '202-302']

#Definition of binnings for histo

fluctbins = np.linspace(8e-8, 8e-7, 200)
enebins = np.linspace(0, 10000, 200)
enebinskev = np.linspace(0, 8, 60)



wd_func = ['db', 'sym']
Q_cum_db = []
Q_cum_sym = []

for i in range(len(run_nb)):
    for j in range(len(intervals)):
        for k in range(len(wd_func)):
            try:
                Q_file = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/"+run_nb[i]+"/Q_Ar_['"+run_nb[i]+"']_evts_["+intervals[j]+"]_"+wd_func[k]+".npy")
                if wd_func[k] == 'db':               
                    Q_cum_db.extend(Q_file)
                if wd_func[k] == 'sym':
                    Q_cum_sym.extend(Q_file)
            except:
                continue

#Find the 5.9 keV peak and do the conversion from ADC counts to keV
print('len Q db is : ', len(Q_cum_db))
print('len Q sym is : ', len(Q_cum_sym))

Q_cum_db = np.array(Q_cum_db)
Q_cum_sym = np.array(Q_cum_sym)
mask_to_kev = (Q_cum_db>=2000)
counts, bins, __ = plt.hist(Q_cum_db[mask_to_kev], bins=enebins)
plt.show()
bin_centers = (bins[:-1] + bins[1:]) / 2

to_kev = bin_centers[np.argmax(counts)]


#difference between Q with db and symlet

cdb, binsdb, _ = plt.hist(Q_cum_db * 5.9/to_kev, bins = enebinskev)
csym, binssym, _ = plt.hist(Q_cum_sym * 5.9/to_kev, bins = enebinskev)

bin_centers = (binsdb[:-1] + binsdb[1:]) / 2

diff_wd = cdb-csym

plt.plot(bin_centers, diff_wd, label='Qdb - Qsym')
plt.xlabel('Energy (pes)')
plt.ylabel('Entries (db - sym)')
plt.legend()
plt.show()


db_noise = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/3131/sigma_noise_Ar_['3131']_evts_[0-100]_db.npy")
sym_noise = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/3131/sigma_noise_Ar_['3131']_evts_[0-100]_sym.npy")
coif_noise = np.loadtxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/3131/sigma_noise_Ar_['3131']_evts_[0-100]_coif.npy")

noisebins = np.linspace(0.15, 0.35, 50)

cdb, binsdb, _ = plt.hist(db_noise, bins = noisebins, density=True, alpha=0.5, label='db')
csym, binssym, _ = plt.hist(sym_noise, bins = noisebins, density=True, alpha=0.5, label='sym')
ccoif, binscoif, _ = plt.hist(coif_noise, bins = noisebins, density=True, alpha=0.5, label='coif')
plt.xlabel('$\sigma_{cD_{1}}$', fontsize=14)
plt.ticklabel_format(axis='x', style='sci', scilimits=(0,0))
plt.ylabel('Entries', fontsize=14)

bin_centers = (binsdb[:-1] + binsdb[1:]) / 2

diff_wd = cdb-csym
plt.legend()
plt.show()

plt.plot(bin_centers, diff_wd, label='noise_db - noise_sym')
plt.xlabel('Std noise in baseline')
plt.ylabel('Entries (db - sym)')
plt.legend()
plt.show()


