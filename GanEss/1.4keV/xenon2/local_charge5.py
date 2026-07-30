import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm

# ---------------------------------------------------------
# General parameters
# ---------------------------------------------------------
np.random.seed(42)

#Reading of input param from source code moise_analysis.sh
import sys

run_nb = [sys.argv[1]]
event_min = int(sys.argv[2])

wd_func = str(sys.argv[3])
gas = str(sys.argv[4])
nf = int(sys.argv[5])
event_max = event_min + nf

USE_FAKE_DATA = False
REAL_DATA_PATH = "/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/wf_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy"
#REAL_DATA_PATH = "/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/Argon/wf_Ar_['3131']_evts_[0-200]_coif.npy"

MAX_ROWS = 30000
ENTIRE_DATA = False

N_EVENTS      = 2000
N_SAMPLES     = 500
DT            = 1.0
NOISE_SIGMA   = 0.05
PULSE_SIGMA   = 4.0
AMP_MIN, AMP_MAX = 0.5, 5.0
MIN_SEP       = 15

# ----- Primary detection parameters (baseline + Nsigma) -----
BASELINE_N_BINS = 500
N_SIGMA          = 3.0
MIN_DURATION     = 250
MIN_GAP_BELOW_PRIMARY = 5   # min number of consecutive points below the PRIMARY threshold
                             # required to consider the pulse truly over.
                             # If the curve dips below the primary threshold for less
                             # time than this, it is considered a simple fluctuation
                             # and merged with the next segment.

# ----- Double-peak detection parameters -----
SUBPEAK_FRACTION       = 0.7   # secondary threshold = 70% of the pulse height
MIN_VALLEY_DURATION    = 10    # min number of consecutive points below the secondary threshold
                               # required to be considered a real valley (not just noise)
VALLEY_AREA_FRACTION   = 0.2   # the valley area must be > X% of the first peak's area
                               # to validate the split into 2 pulses
SECOND_PEAK_MARGIN_FRACTION = 0.1  # after the valley, the curve must rise back above
                                    # the secondary threshold by at least this fraction
                                    # (x peak height) to validate the existence of a 2nd peak

N_EXAMPLES_TO_PLOT = 6

# ---------------------------------------------------------
# Choice of events to display
# ---------------------------------------------------------
FIRST_EVENT_TO_PLOT = 401     # first event to display
LAST_EVENT_TO_PLOT = 450    # last event to display (inclusive)

# ---------------------------------------------------------
# 2D histogram parameters (waveform "persistence" plots)
# ---------------------------------------------------------
HIST2D_TIME_BINS = 200      # number of bins along the time axis
HIST2D_AMP_BINS  = 200      # number of bins along the amplitude axis

# ---------------------------------------------------------
# Energy window used to filter waveforms for the dedicated 2D histogram
# ---------------------------------------------------------
ENERGY_FILTER_MIN_KEV = 1.3   # lower bound (inclusive) of the energy window, in keV
ENERGY_FILTER_MAX_KEV = 1.6   # upper bound (inclusive) of the energy window, in keV


# ---------------------------------------------------------
# Fake data generation
# ---------------------------------------------------------
def generate_pulse(t, t0, amplitude, sigma):
    return amplitude * np.exp(-0.5 * ((t - t0) / sigma) ** 2)


def generate_event(n_samples, dt, noise_sigma, pulse_sigma, min_sep):
    t = np.arange(n_samples) * dt
    waveform = np.random.normal(0, noise_sigma, n_samples)

    n_pulses = np.random.randint(0, 6)

    positions = []
    amplitudes = []
    attempts = 0
    while len(positions) < n_pulses and attempts < 100:
        pos = np.random.uniform(20, n_samples - 20)
        if all(abs(pos - p) > min_sep for p in positions):
            positions.append(pos)
            amplitudes.append(np.random.uniform(AMP_MIN, AMP_MAX))
        attempts += 1

    for pos, amp in zip(positions, amplitudes):
        waveform += generate_pulse(t, pos, amp, pulse_sigma)

    return waveform


def generate_fake_dataset(n_events, n_samples, dt, noise_sigma, pulse_sigma, min_sep):
    data = np.zeros((n_events, n_samples))
    for i in range(n_events):
        data[i] = generate_event(n_samples, dt, noise_sigma, pulse_sigma, min_sep)
    return data


# ---------------------------------------------------------
# Loading real data
# ---------------------------------------------------------
def load_real_dataset(path):
    """
    Expected shape: (x, y) with y = number of points per event.
    NB: if the file is a real binary .npy, use np.load(path) instead of np.loadtxt.
    """
    if ENTIRE_DATA:
        data = np.loadtxt(path)
    else:
        data = np.loadtxt(path, max_rows=MAX_ROWS)

    return data


# ---------------------------------------------------------
# Threshold based on the baseline (mean + N*sigma)
# ---------------------------------------------------------
def get_baseline_threshold(waveform, n_bins, n_sigma):
    n_bins = min(n_bins, len(waveform))
    baseline = waveform[:n_bins]
    baseline_mean = np.mean(baseline)
    baseline_std = np.std(baseline)
    threshold = baseline_mean + n_sigma * baseline_std
    return threshold, baseline_mean, baseline_std


# ---------------------------------------------------------
# Detect segments above the primary threshold, merging segments
# separated by a dip that is too short (simple fluctuation)
# ---------------------------------------------------------
def find_above_segments(waveform, threshold, min_gap_below):
    """
    Finds continuous runs where waveform > threshold.
    If two consecutive runs are separated by a dip below threshold
    shorter than `min_gap_below` points, they are merged into a single
    segment (considered a fluctuation, not a real end of pulse).
    """
    above = waveform > threshold
    n = len(waveform)

    raw_segments = []
    i = 0
    while i < n:
        if above[i]:
            start = i
            while i < n and above[i]:
                i += 1
            end = i  # exclusive
            raw_segments.append([start, end])
        else:
            i += 1

    if not raw_segments:
        return []

    merged_segments = [raw_segments[0]]
    for seg in raw_segments[1:]:
        prev = merged_segments[-1]
        gap = seg[0] - prev[1]  # number of points below threshold between the 2 runs
        if gap < min_gap_below:
            # fluctuation too short -> merge with previous segment
            prev[1] = seg[1]
        else:
            merged_segments.append(seg)

    return [tuple(s) for s in merged_segments]


# ---------------------------------------------------------
# Split a segment into sub-pulses if a double peak is detected
# via the valley area under a secondary threshold, AND
# check that the curve rises back up enough after the valley
# ---------------------------------------------------------
def split_double_peaks(waveform, start, end, dt,
                        subpeak_fraction, min_valley_duration, valley_area_fraction,
                        second_peak_margin_fraction):
    """
    Analyzes a segment [start:end) already detected as a pulse.
    - secondary threshold = subpeak_fraction * max height of the segment
    - looks for continuous regions where the curve stays below this threshold
      for at least `min_valley_duration` consecutive points (= "valley")
    - for each candidate valley, computes its area (area between the curve
      and the secondary threshold, where waveform < threshold)
    - condition 1: this area must be > valley_area_fraction * area of the preceding peak
    - condition 2: AFTER the valley, the curve must rise back above the secondary
      threshold by at least second_peak_margin_fraction * peak height
      (otherwise it's not a real 2nd peak, just a dip back under the threshold)
    - if both conditions are met -> split at the valley's minimum
    - otherwise, no split
    """
    segment = waveform[start:end]
    n = len(segment)
    peak_val = segment.max()
    subpeak_threshold = subpeak_fraction * peak_val
    second_peak_min_val = subpeak_threshold + second_peak_margin_fraction * peak_val

    below = segment < subpeak_threshold

    # --- detect continuous runs below the secondary threshold ---
    valleys = []
    i = 0
    while i < n:
        if below[i]:
            v_start = i
            while i < n and below[i]:
                i += 1
            v_end = i  # exclusive
            if (v_end - v_start) >= min_valley_duration:
                valleys.append((v_start, v_end))
        else:
            i += 1

    if not valleys:
        # no valley long enough -> single pulse
        peak_idx = start + np.argmax(segment)
        charge = np.sum(segment) * dt
        return [{
            "start": start, "end": end, "charge": charge,
            "peak_idx": peak_idx, "peak_val": waveform[peak_idx],
            "is_double": False, "subpeak_threshold": None,
        }]

    # --- evaluate each candidate valley ---
    boundaries_local = [0]
    prev_boundary = 0
    any_split = False

    for v_i, (v_start, v_end) in enumerate(valleys):
        preceding_segment = segment[prev_boundary:v_start]
        if preceding_segment.size == 0:
            continue

        first_peak_area = np.sum(preceding_segment) * dt

        valley_segment = segment[v_start:v_end]
        # valley area = area between the secondary threshold and the curve
        valley_area = np.sum(subpeak_threshold - valley_segment) * dt

        # segment following the valley, up to the next valley (or the end)
        next_v_start = valleys[v_i + 1][0] if (v_i + 1) < len(valleys) else n
        following_segment = segment[v_end:next_v_start]

        condition_area = (first_peak_area > 0 and
                          valley_area > valley_area_fraction * first_peak_area)
        condition_rebound = (following_segment.size > 0 and
                             following_segment.max() >= second_peak_min_val)

        if condition_area and condition_rebound:
            # valley validated -> split at the valley's local minimum
            split_local = v_start + int(np.argmin(valley_segment))
            boundaries_local.append(split_local)
            prev_boundary = split_local
            any_split = True
        # otherwise: valley not significant, no split here

    boundaries_local.append(n)
    boundaries_local = sorted(set(boundaries_local))

    if not any_split or len(boundaries_local) == 2:
        # no split validated -> single pulse
        peak_idx = start + np.argmax(segment)
        charge = np.sum(segment) * dt
        return [{
            "start": start, "end": end, "charge": charge,
            "peak_idx": peak_idx, "peak_val": waveform[peak_idx],
            "is_double": False, "subpeak_threshold": None,
        }]

    # --- build sub-pulses from the validated boundaries ---
    sub_pulses = []
    for i in range(len(boundaries_local) - 1):
        s_local = boundaries_local[i]
        e_local = boundaries_local[i + 1]
        sub_segment = segment[s_local:e_local]
        if sub_segment.size == 0:
            continue
        sub_charge = np.sum(sub_segment) * dt
        sub_peak_local = int(np.argmax(sub_segment))
        sub_peak_idx = start + s_local + sub_peak_local
        sub_pulses.append({
            "start": start + s_local,
            "end": start + e_local,
            "charge": sub_charge,
            "peak_idx": sub_peak_idx,
            "peak_val": waveform[sub_peak_idx],
            "is_double": True,
            "subpeak_threshold": subpeak_threshold,
        })
    return sub_pulses


# ---------------------------------------------------------
# Detect pulses via threshold + minimum continuous duration + double-peak split
# ---------------------------------------------------------
def detect_pulses_threshold_duration(waveform, threshold, min_duration, min_gap_below, dt,
                                      subpeak_fraction, min_valley_duration,
                                      valley_area_fraction, second_peak_margin_fraction):
    segments = find_above_segments(waveform, threshold, min_gap_below)

    pulses = []
    for (start, end) in segments:
        duration = end - start
        if duration >= min_duration:
            sub_pulses = split_double_peaks(
                waveform, start, end, dt,
                subpeak_fraction, min_valley_duration, valley_area_fraction,
                second_peak_margin_fraction
            )
            pulses.extend(sub_pulses)

    return pulses


# ---------------------------------------------------------
# Load data
# ---------------------------------------------------------
if USE_FAKE_DATA:
    dataset = generate_fake_dataset(
        N_EVENTS, N_SAMPLES, DT, NOISE_SIGMA, PULSE_SIGMA, MIN_SEP
    )
else:
    dataset = load_real_dataset(REAL_DATA_PATH)

n_events_loaded, n_samples_loaded = dataset.shape
t_axis = np.arange(n_samples_loaded) * DT

print(f"Dataset loaded: {n_events_loaded} events, {n_samples_loaded} points/event")
print(f"Baseline over the first {BASELINE_N_BINS} points, threshold = mean + {N_SIGMA}*sigma")
print(f"Merging segments if dip below primary threshold < {MIN_GAP_BELOW_PRIMARY} points")
print(f"Double-peak detection: secondary threshold = {SUBPEAK_FRACTION*100:.0f}% of the peak, "
      f"min valley {MIN_VALLEY_DURATION} points, valley area > {VALLEY_AREA_FRACTION*100:.0f}% of the 1st peak's area, "
      f"rebound after valley >= {SECOND_PEAK_MARGIN_FRACTION*100:.0f}% of the peak height above the secondary threshold")


# ---------------------------------------------------------
# Process all events
# ---------------------------------------------------------
n_pulses_per_event = []
total_charge_per_event = []
all_pulse_charges = []
single_pulse_individual_charges = []  # charge of the single pulse, for events with exactly 1 pulse
example_events = []
n_double_pulses = 0

for idx in range(n_events_loaded):
    waveform = dataset[idx]

    threshold, baseline_mean, baseline_std = get_baseline_threshold(
        waveform, BASELINE_N_BINS, N_SIGMA
    )

    pulses = detect_pulses_threshold_duration(
        waveform, threshold, MIN_DURATION, MIN_GAP_BELOW_PRIMARY, DT,
        SUBPEAK_FRACTION, MIN_VALLEY_DURATION, VALLEY_AREA_FRACTION,
        SECOND_PEAK_MARGIN_FRACTION
    )
    charges = [p["charge"] for p in pulses]
    n_double_pulses += sum(1 for p in pulses if p["is_double"])

    n_pulses_per_event.append(len(pulses))
    # total_charge_per_event.append(np.sum(charges) if charges else 0.0)
    total_charge_per_event.append(np.sum(waveform))  # raw total charge (for comparison)
    all_pulse_charges.extend(charges)

    if len(pulses) == 1:
        single_pulse_individual_charges.append(charges[0])

    if FIRST_EVENT_TO_PLOT <= idx <= LAST_EVENT_TO_PLOT:
        example_events.append((idx, waveform, pulses, threshold))

n_pulses_per_event = np.array(n_pulses_per_event)
total_charge_per_event = np.array(total_charge_per_event)
all_pulse_charges = np.array(all_pulse_charges)
single_pulse_individual_charges = np.array(single_pulse_individual_charges)

print(f"Total number of pulses detected: {len(all_pulse_charges)}")
print(f"  of which from a double-peak split: {n_double_pulses}")
print(f"Average number of pulses per event: {n_pulses_per_event.mean():.2f}")


# ---------------------------------------------------------
# Detailed visualization of examples: colored windows + charge annotation
# ---------------------------------------------------------
def plot_example_event(ax, t_axis, waveform, pulses, threshold, title=""):
    ax.plot(t_axis, waveform, lw=1, color="black", zorder=1)

    ax.axhline(threshold, color="gray", ls="--", lw=1,
               label=f"threshold = {threshold:.2f}")
    ax.axvspan(t_axis[0], t_axis[BASELINE_N_BINS - 1],
               color="blue", alpha=0.1, label="baseline window")

    colors = plt.cm.tab10.colors
    subpeak_threshold_shown = False
    for k, p in enumerate(pulses):
        color = colors[k % len(colors)]
        ax.axvspan(t_axis[p["start"]], t_axis[p["end"] - 1],
                   color=color, alpha=0.3, zorder=0)

        # secondary threshold line, shown only for double peaks
        if p["is_double"] and p["subpeak_threshold"] is not None:
            ax.hlines(p["subpeak_threshold"], t_axis[p["start"]], t_axis[p["end"] - 1],
                       color=color, ls=":", lw=1.3, zorder=1,
                       label="secondary threshold (70% peak)" if not subpeak_threshold_shown else None)
            subpeak_threshold_shown = True

        marker = "D" if p["is_double"] else "o"  # diamond if from a split
        ax.plot(t_axis[p["peak_idx"]], p["peak_val"], marker,
                color=color, markersize=7, zorder=2)
        label = f"Q={p['charge']:.2f}"
        if p["is_double"]:
            label += " (double)"
        ax.annotate(label,
                    (t_axis[p["peak_idx"]], p["peak_val"]),
                    textcoords="offset points", xytext=(5, 8),
                    fontsize=8, fontweight="bold", color=color)

    ax.set_title(title, fontsize=10)
    ax.set_xlabel("time")
    ax.set_ylabel("amplitude")
    ax.legend(loc="upper right", fontsize=7)


n_ex = len(example_events)
ncols = 2
nrows = int(np.ceil(n_ex / ncols))
fig, axes = plt.subplots(nrows, ncols, figsize=(12, 4 * nrows))
axes = np.array(axes).reshape(-1)

for i, (event_idx, waveform, pulses, threshold) in enumerate(example_events):
    plot_example_event(
        axes[i], t_axis, waveform, pulses, threshold,
        title=f"Event {event_idx} — {len(pulses)} pulse(s) detected"
    )

for j in range(n_ex, len(axes)):
    axes[j].axis("off")

fig.suptitle("Detected pulses — diamond = peak from a double-peak split (valley area)")
plt.tight_layout()

# ---------------------------------------------------------
# Plot: number of pulses detected per event
# ---------------------------------------------------------
plt.figure(figsize=(7, 5))
max_n = max(n_pulses_per_event.max(), 5)
bins = np.arange(-0.5, max_n + 1.5, 1)
plt.hist(n_pulses_per_event, bins=bins, edgecolor="black", rwidth=0.8)
plt.xlabel("Number of pulses detected per event")
plt.yscale('log')
plt.ylabel("Number of events")
plt.title("Distribution of the number of pulses per event")

# ---------------------------------------------------------
# Plot: total charge per event
# ---------------------------------------------------------
plt.figure(figsize=(7, 5))
ene_pes_bins = np.linspace(0, 5000, 100)
ene_bins = np.linspace(0, 10, 50)
mask_to_kev = (total_charge_per_event>=2000)
counts, bins = np.histogram(total_charge_per_event[mask_to_kev], bins=ene_pes_bins)
bin_centers = (bins[:-1] + bins[1:]) / 2

to_kev = 5.9/bin_centers[np.argmax(counts)]

plt.hist(total_charge_per_event * to_kev, bins=ene_bins, edgecolor="black")
plt.xlabel("Total charge per event (sum of local charges)")
plt.ylabel("Number of events")
plt.yscale("log")
plt.title("Distribution of the total charge per event")

# ---------------------------------------------------------
# Plot: charge of each individual pulse
# ---------------------------------------------------------
plt.figure(figsize=(7, 5))
if len(all_pulse_charges) > 0:
    plt.hist(all_pulse_charges * to_kev, bins=ene_bins, edgecolor="black", color="tab:orange")
plt.xlabel("Local charge of an individual pulse")
plt.ylabel("Number of pulses")
plt.yscale("log")
plt.title("Distribution of charges of each pulse (independent)")

# ---------------------------------------------------------
# Plot: total charge for events with only 1 pulse detected
# ---------------------------------------------------------
plt.figure(figsize=(7, 5))
mask_single_pulse = n_pulses_per_event == 1
single_pulse_charges = total_charge_per_event[mask_single_pulse]
plt.hist(single_pulse_charges * to_kev, bins=ene_bins, edgecolor="black", color="tab:orange")
plt.xlabel("Total charge for events with 1 pulse detected")
plt.ylabel("Number of events")
plt.yscale("log")
plt.title("Distribution of total charges for events with 1 pulse detected")

# ---------------------------------------------------------
# Plot: independent pulse charge, only for events with exactly 1 pulse detected
# ---------------------------------------------------------
plt.figure(figsize=(7, 5))
if len(single_pulse_individual_charges) > 0:
    plt.hist(single_pulse_individual_charges * to_kev, bins=ene_bins, edgecolor="black", color="tab:green")
plt.xlabel("Individual pulse charge (events with 1 pulse detected)")
plt.ylabel("Number of pulses")
plt.yscale("log")
plt.title("Distribution of individual pulse charges — single-pulse events only")

# ---------------------------------------------------------
# Plot: 2D histograms (waveform "persistence" view)
#   Left  -> all waveforms as loaded from the file
#   Right -> only waveforms belonging to single-pulse events
# ---------------------------------------------------------
fig, axes = plt.subplots(1, 2, figsize=(14, 6))

# --- left panel: all waveforms ---
x_all = np.tile(t_axis, n_events_loaded)
y_all = dataset.flatten()

h0 = axes[0].hist2d(
    x_all, y_all,
    bins=[HIST2D_TIME_BINS, HIST2D_AMP_BINS],
    norm=LogNorm(),
    cmap="viridis",
)
fig.colorbar(h0[3], ax=axes[0], label="counts")
axes[0].set_xlabel("time")
axes[0].set_ylabel("amplitude")
axes[0].set_title(f"All waveforms (N = {n_events_loaded} events)")

# --- right panel: only single-pulse waveforms ---
single_pulse_waveforms = dataset[mask_single_pulse]
np.savetxt("/Users/ldonneger/Desktop/PhD_Thesis2/GanEss/1.4keV/xenon2/single_pulse_wf_"+str(gas)+"_"+str(run_nb)+"_evts_["+str(event_min)+"-"+str(event_max)+"]_"+wd_func+".npy", single_pulse_waveforms)

n_single_pulse_events = single_pulse_waveforms.shape[0]

x_single = np.tile(t_axis, n_single_pulse_events)
y_single = single_pulse_waveforms.flatten()

h1 = axes[1].hist2d(
    x_single, y_single,
    bins=[HIST2D_TIME_BINS, HIST2D_AMP_BINS],
    norm=LogNorm(),
    cmap="viridis",
)
fig.colorbar(h1[3], ax=axes[1], label="counts")
axes[1].set_xlabel("time")
axes[1].set_ylabel("amplitude")
axes[1].set_title(f"Single-pulse waveforms only (N = {n_single_pulse_events} events)")

fig.suptitle("Waveform distribution — all events vs. single-pulse events")
plt.tight_layout()

# ---------------------------------------------------------
# Plot: 2D histogram, only for waveforms whose total charge falls
# within the [ENERGY_FILTER_MIN_KEV, ENERGY_FILTER_MAX_KEV] energy window
# ---------------------------------------------------------
total_charge_kev = total_charge_per_event * to_kev
mask_energy_window = ((total_charge_kev >= ENERGY_FILTER_MIN_KEV) &
                       (total_charge_kev <= ENERGY_FILTER_MAX_KEV))

energy_window_waveforms = dataset[mask_energy_window]
n_energy_window_events = energy_window_waveforms.shape[0]

plt.figure(figsize=(7, 6))
if n_energy_window_events > 0:
    x_energy = np.tile(t_axis, n_energy_window_events)
    y_energy = energy_window_waveforms.flatten()

    h2 = plt.hist2d(
        x_energy, y_energy,
        bins=[HIST2D_TIME_BINS, HIST2D_AMP_BINS],
        norm=LogNorm(),
        cmap="viridis",
    )
    plt.colorbar(h2[3], label="counts")
else:
    print("No waveform found in the requested energy window — skipping this 2D histogram.")

plt.xlabel("time")
plt.ylabel("amplitude")
plt.title(f"Waveforms with total charge in [{ENERGY_FILTER_MIN_KEV}, {ENERGY_FILTER_MAX_KEV}] keV "
          f"(N = {n_energy_window_events} events)")
plt.tight_layout()

plt.show()
