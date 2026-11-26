#!/usr/bin/env python3
"""
3D outline of the theoretical geometry + check that Geant4 hits are contained in it.

Usage:
  python check_geometry_en.py --list
  python check_geometry_en.py VESSEL_GAS                         # outline only
  python check_geometry_en.py VESSEL_GAS --demo                  # synthetic points to test the script
  python check_geometry_en.py VESSEL_GAS --points hits.csv       # Geant4 points (columns x,y,z [,volume])
  python check_geometry_en.py VESSEL_GAS --points hits.csv hist_g4.csv   # compare 2 (up to 3) distributions
  python check_geometry_en.py "GAS_*" --points hits.csv          # wildcards accepted
  python check_geometry_en.py --overlaps                         # overlaps between volumes + volumes sticking out of their mother
  python check_geometry_en.py VESSEL_GAS --points hits.csv --save fig_gas   # save figures (formats/dpi: STYLE["save"]; the Agg backend does not open windows)

All the plot settings (fonts, colours, line widths, sizes, binning, texts...) are in the STYLE and TEXT blocks below.

The geometry file (geometry_theory.csv) can be read with pandas: pd.read_csv("geometry_theory.csv").
Units: mm, degrees. Columns:
  volume, name, copy, mother, role (base/union/subtract), shape (tubs/cons/trap/extruded), material,
  rmin1, rmax1, rmin2, rmax2, hz (half-length), x, y, z, rot, ty1, tx1, tx2, ty2, tx3, tx4, polygon, comment
A volume placed several times has one 'base' row per 'copy'; the volume is the union of its copies.
"""
# HOW THE SCRIPT WORKS
# 1. load_geometry() reads geometry_theory.csv and builds one Volume object per volume name.
# 2. For each hits file, inside_mask() tells which hits are inside the theoretical geometry.
# 3. plot_3d() draws the outline + the hits; plot_distributions() draws x/y/z histograms and (z, r) maps.
# 4. check_overlaps() (option --overlaps) tests the geometry itself, no hits needed.
# Geometry code (Prim / Instance / Volume) answers two questions for any solid: 'is this point inside?' and 'what lines draw it?'.
import argparse
import fnmatch
import numpy as np
import pandas as pd
import matplotlib
# Agg backend = no window is opened; figures are only written to files (--save).
#matplotlib.use("Agg")
import matplotlib.pyplot as plt
from pathlib import Path as FilePath
from matplotlib.path import Path
from matplotlib.colors import LogNorm
from matplotlib.lines import Line2D
from matplotlib.ticker import NullLocator
from mpl_toolkits.mplot3d.art3d import Line3DCollection
# Default geometry file (must be in the folder where you run the script, or use --geom).
GEOM_FILE = "geometry_theory.csv"

# ==============================================================================================
#  STYLE: everything about the look of the figures (edit here)
# ==============================================================================================
STYLE = {
    "font": {                       # sizes in points
        "family": ["Helvetica", "Arial", "DejaVu Sans"],
        "size": 13, "title": 15, "suptitle": 17, "label": 14, "tick": 12, "legend": 11,
    },
    "fig": {"size_3d": (9.5, 8.5), "size_dist": (17, 10), "facecolor": "white"},   # sizes in inches
    "axes": {
        "linewidth": 1.3, "tick_direction": "in", "minor_ticks": True,
        "hide_top_right": True, "grid": True, "grid_alpha": 0.25, "grid_ls": "-",
    },
    "colors": {                     # Okabe-Ito palette (colour-blind friendly, readable in black & white)
        "datasets": ["#0072B2", "#D55E00", "#009E73"],
        "outside": "#C00000",       # points outside the geometry
        "theory": "#1a1a1a",        # outline of the geometry
        "subtract": "#CC79A7",      # subtracted solids (dashed)
    },
    "lines": {"base_lw": 1.5, "union_lw": 1.1, "subtract_lw": 0.9, "subtract_ls": "--"},
    "points": {
        "size_3d": 3, "alpha_3d": 0.45, "max_3d": 20000,        # max_3d: sub-sampling for display only
        "outside_size": 26, "outside_marker": "x", "outside_lw": 1.2,
        "markers": ["o", "^", "s"],
    },
    "legend": {"frameon": True, "framealpha": 0.9, "edgecolor": "none", "loc": "bottom"},
    "hist": {
        "bins": 120, "lw": 2.0, "fill_alpha": 0.08, "logy": True,
        "headroom_log": 1.0, "headroom_lin": 0.35,      # free space at the top for the legend (decades / fraction)
        "limit_color": "black", "limit_ls": "--", "limit_lw": 1.3,
    },
    "rz": {"bins": 120, "cmap": "viridis", "lognorm": True, "envelope_color": "#D55E00", "envelope_lw": 1.8},
    "view3d": {"elev": 22, "azim": -58, "zoom": 1.0, "pane_alpha": 0.0, "pane_edge": "#bbbbbb",
               "grid": True, "show_subtract": True, "nbins": 5},
    "save": {"formats": ["png", "pdf"], "dpi": 1200, "transparent": False, "bbox_tight": True},
}

# ==============================================================================================
#  TEXT: all the texts shown on the figures
# ==============================================================================================
TEXT = {
    "x": "x [mm]", "y": "y [mm]", "z": "z [mm]",
    "r": r"$r=\sqrt{x^2+y^2}$ [mm]",
    "density1d": r"Probability density [mm$^{-1}$]",
    "density2d": r"Probability density [mm$^{-2}$]",
    "theory": "Theoretical geometry",
    "limits": "Theoretical limits",
    "envelope": "Theoretical envelope",
    "points_label": "{label} ({n} hits, {n_out} outside)",
    "title_3d": "{volume} \u2014 3D view",
    "title_dist": "{volume} \u2014 hit distributions vs. theoretical limits",
    "title_rz": "{label}: (z, r) map",
}

# Copies the STYLE values into matplotlib's global settings (rcParams). Called once at the start of main().
def apply_style():
    f, a, S = STYLE["font"], STYLE["axes"], STYLE["save"]
    plt.rcParams.update({
        "font.family": "sans-serif", "font.sans-serif": f["family"], "font.size": f["size"],
        "axes.titlesize": f["title"], "axes.labelsize": f["label"],
        "xtick.labelsize": f["tick"], "ytick.labelsize": f["tick"],
        "legend.fontsize": f["legend"], "figure.titlesize": f["suptitle"],
        "axes.linewidth": a["linewidth"], "xtick.direction": a["tick_direction"],
        "ytick.direction": a["tick_direction"], "xtick.minor.visible": a["minor_ticks"],
        "ytick.minor.visible": a["minor_ticks"], "xtick.major.width": a["linewidth"],
        "ytick.major.width": a["linewidth"], "axes.spines.top": not a["hide_top_right"],
        "axes.spines.right": not a["hide_top_right"], "axes.grid": a["grid"],
        "grid.alpha": a["grid_alpha"], "grid.linestyle": a["grid_ls"],
        "legend.frameon": STYLE["legend"]["frameon"], "legend.framealpha": STYLE["legend"]["framealpha"],
        "legend.edgecolor": STYLE["legend"]["edgecolor"],
        "figure.facecolor": STYLE["fig"]["facecolor"], "axes.facecolor": STYLE["fig"]["facecolor"],
        "savefig.dpi": S["dpi"], "mathtext.default": "regular",
        "pdf.fonttype": 42, "ps.fonttype": 42,          # editable fonts in PDF
    })
# Settings of the orange (z, r) envelope drawn on the maps: it is built by testing a grid of (z, r) points
# against the real volume (see rz_envelope), so a finer grid / more angles = more precise but slower.
RZ_RES_MM = 0.5     # (z, r) envelope: grid resolution [mm]
RZ_N_PHI = 12       # (z, r) envelope: number of azimuthal angles tested

# ----------------------------------------------------------------------------
# Rotations (CLHEP convention: rotateX(a) => R = Rx(a) @ R_current)
# ----------------------------------------------------------------------------
# Rotation matrix around the X, Y or Z axis (angle a in degrees).
def _rot(axis, a):
    c, s = np.cos(np.radians(a)), np.sin(np.radians(a))
    return {"X": np.array([[1, 0, 0], [0, c, -s], [0, s, c]]),
            "Y": np.array([[c, 0, s], [0, 1, 0], [-s, 0, c]]),
            "Z": np.array([[c, -s, 0], [s, c, 0], [0, 0, 1]])}[axis]

# Converts the 'rot' column (e.g. "X90,Y-30") into one matrix. Rotations are chained in the order written,
# like rotateX(90) then rotateY(-30) in Geant4 (each new rotation is applied on top of the previous ones).
def parse_rot(s):
    R = np.eye(3)
    if isinstance(s, str) and s.strip():
        for tok in s.replace(" ", "").split(","):
            R = _rot(tok[0].upper(), float(tok[1:])) @ R
    return R

# Reads a number from the csv; an empty cell (NaN) becomes 'default'.
def _f(v, default=0.0):
    return default if pd.isna(v) else float(v)

# ----------------------------------------------------------------------------
# Primitives
# ----------------------------------------------------------------------------
# One row of the csv = one elementary solid: tubs, cons, trap or extruded polygon.
# It can say whether a point is inside (contains) and how to draw itself (outline).
# Both work in the solid's OWN frame (origin at its position, axes rotated by its rotation).
class Prim:
    def __init__(self, row):
        self.name, self.role, self.shape = row["name"], row["role"], row["shape"]
        # hz = half-length along z. rmin/rmax = inner/outer radius at z=-hz (suffix 1) and at z=+hz (suffix 2):
        # a tubs has the same radii at both ends, a cons has different ones.
        self.hz = _f(row["hz"])
        self.rmin1, self.rmax1 = _f(row["rmin1"]), _f(row["rmax1"])
        self.rmin2 = _f(row["rmin2"], self.rmin1)
        self.rmax2 = _f(row["rmax2"], self.rmax1)
        # trap only: half-widths in y (ty1, ty2) and in x (tx1..tx4) at the two z ends
        self.ty1, self.ty2 = _f(row["ty1"]), _f(row["ty2"])
        self.tx1, self.tx2, self.tx3, self.tx4 = (_f(row[k]) for k in ("tx1", "tx2", "tx3", "tx4"))
        # position (x, y, z) of the solid and its rotation matrix
        self.t = np.array([_f(row["x"]), _f(row["y"]), _f(row["z"])])
        self.R = parse_rot(row["rot"])
        # extruded only: polygon vertices written as 'x:y;x:y;...' in the csv, extruded along z over +-hz
        self.poly = None
        if self.shape == "extruded":
            self.poly = np.array([[float(c) for c in p.split(":")] for p in row["polygon"].split(";")])
    # --- membership (local coordinates, N x 3) ---
    # p = (N,3) points in the solid's own frame -> boolean array (True = inside). 'tol' widens the solid by tol mm.
    def contains(self, p, tol=0.0):
        x, y, z = p.T
        hz = self.hz
        # inside the z range of the solid?
        inz = np.abs(z) <= hz + tol
        # tubs / cons: radius between rmin and rmax, both varying linearly with z (t = 0 at z=-hz, t = 1 at z=+hz)
        if self.shape in ("tubs", "cons"):
            r = np.hypot(x, y)
            t = np.clip((z + hz) / (2 * hz), 0, 1)
            rmin = self.rmin1 + (self.rmin2 - self.rmin1) * t
            rmax = self.rmax1 + (self.rmax2 - self.rmax1) * t
            return inz & (r >= rmin - tol) & (r <= rmax + tol)
        # trap: trapezoid whose half-widths in y and x vary linearly with z (angles theta = phi = alpha = 0)
        if self.shape == "trap":   # theta = phi = alpha = 0
            t = np.clip((z + hz) / (2 * hz), 0, 1)
            h = self.ty1 + (self.ty2 - self.ty1) * t
            xlo = self.tx1 + (self.tx3 - self.tx1) * t
            xhi = self.tx2 + (self.tx4 - self.tx2) * t
            u = np.clip((y + h) / (2 * h), 0, 1)
            hx = xlo + (xhi - xlo) * u
            return inz & (np.abs(y) <= h + tol) & (np.abs(x) <= hx + tol)
        # extruded: point-in-polygon test in the xy plane + z range
        if self.shape == "extruded":
            return inz & Path(self.poly).contains_points(p[:, :2], radius=-tol)
        raise ValueError(f"unknown shape: {self.shape}")
    # --- outline (polylines in local coordinates) ---
    # Lines used to draw the solid (in its own frame): closed curves at both z ends + edges between the two ends.
    def outline(self, n=72):
        hz, lines = self.hz, []
        if self.shape in ("tubs", "cons"):
            # tubs / cons: circles (n segments) at z=-hz and z=+hz, for the inner and the outer radius
            ph = np.linspace(0, 2 * np.pi, n + 1)
            for sgn, rmin, rmax in ((-1, self.rmin1, self.rmax1), (+1, self.rmin2, self.rmax2)):
                for r in (rmin, rmax):
                    if r > 0:
                        lines.append(np.c_[r * np.cos(ph), r * np.sin(ph), np.full_like(ph, sgn * hz)])
            # plus 4 straight lines between the two ends, to give the 3D impression
            for a in np.radians([0, 90, 180, 270]):
                for k in (0, 1):
                    r1, r2 = (self.rmin1, self.rmin2) if k == 0 else (self.rmax1, self.rmax2)
                    if r1 > 0 or r2 > 0:
                        lines.append(np.array([[r1 * np.cos(a), r1 * np.sin(a), -hz],
                                               [r2 * np.cos(a), r2 * np.sin(a), +hz]]))
        # trap: 4 corners at each end (lo = z=-hz end, hi = z=+hz end)
        elif self.shape == "trap":
            c = lambda tx_a, tx_b, ty, z: [(-tx_a, -ty, z), (tx_a, -ty, z), (tx_b, ty, z), (-tx_b, ty, z)]
            lo = np.array(c(self.tx1, self.tx2, self.ty1, -hz))
            hi = np.array(c(self.tx3, self.tx4, self.ty2, +hz))
            lines += [np.vstack([lo, lo[:1]]), np.vstack([hi, hi[:1]])]
            lines += [np.array([lo[i], hi[i]]) for i in range(4)]
        # extruded: the polygon at each end + one edge per vertex
        elif self.shape == "extruded":
            P = self.poly
            lo = np.c_[P, np.full(len(P), -hz)]
            hi = np.c_[P, np.full(len(P), +hz)]
            lines += [np.vstack([lo, lo[:1]]), np.vstack([hi, hi[:1]])]
            lines += [np.array([lo[i], hi[i]]) for i in range(len(P))]
        return lines

# ----------------------------------------------------------------------------
# Instance = one placed copy = base solid + boolean operations
# ----------------------------------------------------------------------------
# One placed copy of a volume = a 'base' solid (first csv row of the copy) followed by 'union' / 'subtract' solids
# that are added to / removed from it. The position and rotation of those solids are relative to the base solid.
class Instance:
    def __init__(self, df):
        # one Prim per csv row of this copy
        prims = [Prim(r) for _, r in df.iterrows()]
        assert prims[0].role == "base", "the first row of each copy must be 'base'"
        # base solid + list of boolean operations (union / subtract)
        self.base, self.ops = prims[0], prims[1:]
        # bounding box of the copy; subtracted solids can only remove material, so they are ignored
        pts = np.vstack([l for l, role in self.outlines() if role != "subtract"])
        self.lo, self.hi = pts.min(0), pts.max(0)        # bounding box (used to skip far-away points quickly)
    # P = (N,3) points in WORLD coordinates -> True where the point is inside this copy.
    def contains(self, P, tol=0.0):
        out = np.zeros(len(P), bool)
        # quick pre-selection: only the points inside the bounding box are tested in detail (matters for volumes with many copies)
        m = np.all((P >= self.lo - abs(tol) - 1e-6) & (P <= self.hi + abs(tol) + 1e-6), axis=1)
        if not m.any():
            return out
        # world -> base frame: p_local = R (p - t) (Geant4 convention)
        pb = (P[m] - self.base.t) @ self.base.R.T              # p_local = R (p - t)
        # start from the points inside the base solid...
        inside = self.base.contains(pb, tol)
        # ...then apply each operation, testing the points in the frame of the operand:
        # 'subtract' removes the points that are inside the operand, 'union' adds them
        for op in self.ops:
            k = op.contains((pb - op.t) @ op.R.T)
            inside = inside & ~k if op.role == "subtract" else inside | k
        out[m] = inside
        return out
    # Lines of this copy in WORLD coordinates, as (line, role) pairs; role = base / union / subtract picks the line style.
    def outlines(self):
        out = [(ln @ self.base.R + self.base.t, "base") for ln in self.base.outline()]
        for op in self.ops:
            for ln in op.outline():
                out.append(((ln @ op.R + op.t) @ self.base.R + self.base.t, op.role))
        return out

# ----------------------------------------------------------------------------
# Volume = union of its copies ('copy' column); 'mother' = mother volume
# ----------------------------------------------------------------------------
# A volume = all the csv rows with the same name in the 'volume' column = the union of its copies ('copy' column).
# 'mother' = volume that contains it in Geant4 (only used by --overlaps).
class Volume:
    def __init__(self, name, df):
        self.name = name
        # mother volume name (None for MOTHER_VOLUME, which has no mother)
        mo = df["mother"].iloc[0] if "mother" in df.columns else np.nan
        self.mother = None if pd.isna(mo) or not str(mo).strip() else str(mo)
        # one Instance per copy number (rows without a copy number are copy 0)
        cp = df["copy"].fillna(0) if "copy" in df.columns else pd.Series(0, index=df.index)
        self.instances = [Instance(g) for _, g in df.groupby(cp, sort=True)]
        self.base, self.ops = self.instances[0].base, self.instances[0].ops
        self.material = df.iloc[0]["material"]
    # a point is inside the volume if it is inside at least one of its copies
    def contains(self, P, tol=0.0):
        inside = np.zeros(len(P), bool)
        for inst in self.instances:
            inside |= inst.contains(P, tol)
        return inside
    def outlines(self):
        """[(global polyline (N,3), role)] for all copies"""
        return [o for inst in self.instances for o in inst.outlines()]
    # used for axis limits and for the theoretical limits drawn on the histograms
    def extent(self):
        """Global (min, max) in x, y, z (base solids + unions, all copies; subtractions ignored)."""
        return (np.min([i.lo for i in self.instances], 0), np.max([i.hi for i in self.instances], 0))

# Reads the csv and returns {volume name: Volume}.
def load_geometry(path=GEOM_FILE):
    df = pd.read_csv(path)
    return {n: Volume(n, g) for n, g in df.groupby("volume", sort=False)}

# Volume names matching the pattern (wildcards allowed, e.g. 'GAS_*'); stops with a message if none matches.
def select(geom, pattern):
    names = [n for n in geom if fnmatch.fnmatch(n, pattern)]
    if not names:
        raise SystemExit(f"No volume matches '{pattern}'. Volumes: {', '.join(geom)}")
    return names

# ----------------------------------------------------------------------------
# Containment
# ----------------------------------------------------------------------------
# With a 'volume' column in the hits file, each hit is tested against ITS OWN volume (useful with wildcards);
# without it, a hit is OK if it is inside at least one of the selected volumes.
def inside_mask(geom, pts, selected, vol_col=None, tol=0.0):
    """True if the point is in its own volume (volume column) or, without that column, in one of the selected volumes."""
    P = pts[["x", "y", "z"]].to_numpy(float)
    ok = np.zeros(len(P), bool)
    if vol_col is not None and vol_col in pts.columns:
        for n, idx in pts.groupby(vol_col).indices.items():
            if n in geom:
                ok[idx] = geom[n].contains(P[idx], tol)
            else:
                print(f"  [!] volume '{n}' not in the geometry file: its points are counted as outside")
    else:
        for n in selected:
            ok |= geom[n].contains(P, tol)
    return ok

# ----------------------------------------------------------------------------
# Plots
# ----------------------------------------------------------------------------
def _role_kw(role):
    """Line style of the outline of a base / union / subtracted solid."""
    L, C = STYLE["lines"], STYLE["colors"]
    if role == "subtract":
        return dict(colors=C["subtract"], linewidths=L["subtract_lw"], linestyles=L["subtract_ls"])
    return dict(colors=C["theory"], linewidths=L["union_lw"] if role == "union" else L["base_lw"], linestyles="-")

def _add_lines_3d(ax, segs, **style):
    """Add 3D lines; 'autolim=False' avoids an error on some matplotlib versions (ignored on older ones)."""
    try:
        ax.add_collection3d(Line3DCollection(segs, **style), autolim=False)
    except TypeError:
        ax.add_collection3d(Line3DCollection(segs, **style))

# Saves one figure in every format of STYLE['save']['formats'] (file name = prefix_tag.format).
def save_fig(fig, prefix, tag):
    S = STYLE["save"]
    for fmt in S["formats"]:
        fig.savefig(f"{prefix}_{tag}.{fmt}", dpi=S["dpi"], transparent=S["transparent"],
                    bbox_inches="tight" if S["bbox_tight"] else None)

# 3D figure: outline of the selected volumes + hits (dots = inside, red crosses = outside).
def plot_3d(geom, selected, datasets=(), title=""):
    """datasets: list of dict(label, pts, ok)"""
    V, P, C = STYLE["view3d"], STYLE["points"], STYLE["colors"]
    fig = plt.figure(figsize=STYLE["fig"]["size_3d"])
    ax = fig.add_subplot(111, projection="3d")
    lo, hi = np.full(3, np.inf), np.full(3, -np.inf)
    # 1) outline of each volume; lo / hi keep the overall limits of what is drawn
    for n in selected:
        v = geom[n]
        for role in ("base", "union", "subtract"):
            if role == "subtract" and not V["show_subtract"]:
                continue
            segs = [l for l, r in v.outlines() if r == role]
            if segs:
                _add_lines_3d(ax, segs, **_role_kw(role))
        a, b = v.extent()
        lo, hi = np.minimum(lo, a), np.maximum(hi, b)
    # 2) hits of each dataset: random sub-sample (max_3d) for speed; k selects the colour and the marker
    for k, d in enumerate(datasets):
        pts, ok = d["pts"], d["ok"]
        if not len(pts):
            continue
        rng = np.random.default_rng(k)
        idx = rng.choice(len(pts), min(P["max_3d"], len(pts)), replace=False)
        Q = pts[["x", "y", "z"]].to_numpy(float)[idx]
        good = ok[idx]
        label = TEXT["points_label"].format(label=d["label"], n=len(pts), n_out=int((~ok).sum()))
        ax.scatter(*Q[good].T, s=P["size_3d"], c=C["datasets"][k], marker=P["markers"][k], alpha=P["alpha_3d"],
                   linewidths=0, rasterized=True, label=label)
        if (~good).any():
            ax.scatter(*Q[~good].T, s=P["outside_size"], c=C["outside"], marker=P["outside_marker"],
                       linewidths=P["outside_lw"])
        lo, hi = np.minimum(lo, Q.min(0)), np.maximum(hi, Q.max(0))
    # 3) axis limits, same scale on the 3 axes (set_box_aspect), viewing angle and cosmetics
    for setter, a, b in zip((ax.set_xlim, ax.set_ylim, ax.set_zlim), lo, hi):
        setter(a, b)
    ax.set_box_aspect(np.maximum(hi - lo, 1), zoom=V["zoom"])
    ax.view_init(elev=V["elev"], azim=V["azim"])
    for axis in (ax.xaxis, ax.yaxis, ax.zaxis):
        axis.pane.set_facecolor((1, 1, 1, V["pane_alpha"]))
        axis.pane.set_edgecolor(V["pane_edge"])
        axis.set_minor_locator(NullLocator())
    for name in "xyz":
        ax.locator_params(axis=name, nbins=V["nbins"])
    ax.grid(V["grid"])
    ax.set_xlabel(TEXT["x"], labelpad=10); ax.set_ylabel(TEXT["y"], labelpad=10); ax.set_zlabel(TEXT["z"], labelpad=10)
    # legend: first entry = theoretical outline (a proxy line), then one entry per dataset
    h, l = ax.get_legend_handles_labels()
    theory = Line2D([0], [0], color=C["theory"], lw=STYLE["lines"]["base_lw"])
    ax.legend([theory] + h, [TEXT["theory"]] + l, loc="upper left", bbox_to_anchor=(0.0, 0.95))
    fig.suptitle(title)
    fig.subplots_adjust(left=0.0, right=1.0, bottom=0.02, top=0.93)
    return fig

# The (z, r) plane mixes all azimuthal angles, so the envelope is built by testing a grid of (z, r) points
# at n_phi angles and keeping the points that are inside the real volume at any of them.
def rz_envelope(vol, zlim, rmax, res, n_phi):
    """Exact (z, r) mask: a (z, r) point is inside if at least one of n_phi azimuthal angles is inside the solid."""
    zs = np.linspace(zlim[0], zlim[1], int(max(50, (zlim[1] - zlim[0]) / res)))
    rs = np.linspace(0, rmax, int(max(50, rmax / res)))
    ZZ, RR = np.meshgrid(zs, rs)
    M = np.zeros(ZZ.shape, bool)
    # for each angle, turn (r, z) into (x, y, z) and test it against the volume
    for phi in np.linspace(0, 2 * np.pi, n_phi, endpoint=False) + 1e-3:
        P = np.column_stack([(RR * np.cos(phi)).ravel(), (RR * np.sin(phi)).ravel(), ZZ.ravel()])
        M |= vol.contains(P).reshape(ZZ.shape)
    return zs, rs, M.astype(float)

# Top row: x, y, z histograms. Bottom row: (z, r) maps (r = distance to the z axis), one per dataset.
def plot_distributions(geom, selected, datasets, title=""):
    """Normalised histograms (density=True) overlaid + one (z, r) map per dataset."""
    H, R, C = STYLE["hist"], STYLE["rz"], STYLE["colors"]
    n = len(datasets)
    fig, axs = plt.subplots(2, 3, figsize=STYLE["fig"]["size_dist"], constrained_layout=True)
    # theoretical limits in x, y, z = extent of the selected volumes
    ext = [geom[k].extent() for k in selected]
    emin = np.min([e[0] for e in ext], axis=0)
    emax = np.max([e[1] for e in ext], axis=0)
    # hits of each dataset as (N,3) arrays
    Ps = [d["pts"][["x", "y", "z"]].to_numpy(float) for d in datasets]
    
    # --- top row: x, y, z histograms with the theoretical limits ---
    # i = 0, 1, 2 for x, y, z. Histogram range = theory + hits, so nothing is cut
    for i, (ax, lab) in enumerate(zip(axs[0], "xyz")):
        lo = min([emin[i]] + [Q[:, i].min() for Q in Ps])
        hi = max([emax[i]] + [Q[:, i].max() for Q in Ps])
        bins = np.linspace(lo, hi, H["bins"] + 1)
        # density=True normalises each histogram to an area of 1, so datasets of different sizes can be compared;
        # each one is drawn twice: a light fill + a solid outline (the outline carries the legend label)
        for k, (d, Q) in enumerate(zip(datasets, Ps)):
            col = C["datasets"][k]
            ax.hist(Q[:, i], bins=bins, density=True, histtype="stepfilled", color=col, alpha=H["fill_alpha"], lw=0)
            ax.hist(Q[:, i], bins=bins, density=True, histtype="step", color=col, lw=H["lw"],
                    label=TEXT["points_label"].format(label=d["label"], n=len(Q), n_out=int((~d["ok"]).sum())))
        # dashed vertical lines = theoretical limits
        for e in (emin[i], emax[i]):
            ax.axvline(e, color=H["limit_color"], ls=H["limit_ls"], lw=H["limit_lw"])
        # empty plot, only to create a legend entry for the dashed lines
        ax.plot([], [], color=H["limit_color"], ls=H["limit_ls"], lw=H["limit_lw"],
                label=f"{TEXT['limits']}: [{emin[i]:.1f}, {emax[i]:.1f}] mm")
        ax.set_xlabel(TEXT[lab]); ax.set_ylabel(TEXT["density1d"])
        if H["logy"]:
            ax.set_yscale("log")
        # leave free space above the curves so that the legend does not hide them
        y0, y1 = ax.get_ylim()
        #ax.set_xlim(0, 500)
        #ax.set_ylim(y0, y1 * 10 ** H["headroom_log"] if H["logy"] else y1 * (1 + H["headroom_lin"]))
        ax.legend(loc="lower center")
    # --- bottom row: (z, r) maps, same bins and same colour scale for all datasets ---
    # (z, r) maps: all datasets share the same bins and the same colour scale so they can be compared
        zr = [(Q[:, 2], np.hypot(Q[:, 0], Q[:, 1])) for Q in Ps]
    zlo = min([emin[2]] + [z.min() for z, _ in zr]); zhi = max([emax[2]] + [z.max() for z, _ in zr])
    rmax = max([np.hypot(emax[0], emax[1])] + [r.max() for _, r in zr])

    # theoretical envelope of each selected volume (computed first: it gives the r range of the geometry)
    envs = [rz_envelope(geom[name], (zlo, zhi), rmax, RZ_RES_MM, RZ_N_PHI) for name in selected]
    # r range = theory (rows of the mask containing at least one inside point) + hits, so nothing is cut
    th_r = [rs_[M_.any(axis=1)] for _, rs_, M_ in envs if M_.any()]
    rlo = min([r.min() for r in th_r] + [r.min() for _, r in zr]) if th_r else 0.0
    rhi = max([r.max() for r in th_r] + [r.max() for _, r in zr]) if th_r else rmax
    # margin around the data: 10 % of the r span, but never less than 2 % of rmax (avoids an extreme zoom on thin volumes)
    rpad = max(0.10 * (rhi - rlo), 0.02 * rmax, 1.0)
    rlo_p, rhi_p = max(0.0, rlo - rpad), rhi + rpad

    # bins cover only the displayed r range: the resolution adapts to the volume and bins stay small
    zb, rb = np.linspace(zlo, zhi, R["bins"] + 1), np.linspace(rlo_p, rhi_p, R["bins"] + 1)
    Hs = [np.histogram2d(z, r, bins=(zb, rb), density=True)[0] for z, r in zr]
    # colour scale (log) goes from the smallest to the largest non-empty bin
    pos = np.concatenate([Hh[Hh > 0] for Hh in Hs])
    norm = LogNorm(vmin=pos.min(), vmax=pos.max()) if (R["lognorm"] and len(pos)) else None
    # 3 slots in the row: the unused ones are hidden
    for k in range(3):
        ax = axs[1, k]
        if k >= n:
            ax.axis("off")
            continue
        # empty bins are masked (shown white)
        mesh = ax.pcolormesh(zb, rb, np.ma.masked_equal(Hs[k], 0).T, cmap=R["cmap"], norm=norm, rasterized=True)
        plt.colorbar(mesh, ax=ax).set_label(TEXT["density2d"])
        # orange line = border of the region that is inside the theoretical volume
        for zs_, rs_, M_ in envs:
            if M_.any():
                ax.contour(zs_, rs_, M_, levels=[0.5], colors=[R["envelope_color"]], linewidths=R["envelope_lw"])
        ax.plot([], [], color=R["envelope_color"], lw=R["envelope_lw"], label=TEXT["envelope"])
        ax.legend(loc="upper right")
        ax.set_xlabel(TEXT["z"]); ax.set_ylabel(TEXT["r"])
        ax.set_ylim(rlo_p, rhi_p)
        ax.set_title(TEXT["title_rz"].format(label=datasets[k]["label"]))
        ax.grid(False)
    fig.suptitle(title)
    return fig

# ----------------------------------------------------------------------------
# Overlaps (Monte-Carlo test between volumes, mother/daughter aware)
# ----------------------------------------------------------------------------
# Chain of mother volumes of volume n (its mother, its grand-mother...).
def _ancestors(geom, n):
    out = []
    while n in geom and geom[n].mother in geom:
        n = geom[n].mother
        out.append(n)
    return out

# Draws uniform random points in the bounding box of each copy and keeps only the points really inside the volume.
def _sample_inside(vol, n_total, rng):
    """Random points drawn INSIDE a volume (rejection sampling in the bounding box of each copy)."""
    n_i = max(n_total // len(vol.instances), 5000)
    pts = []
    for inst in vol.instances:
        P = rng.uniform(inst.lo, inst.hi, (n_i, 3))
        pts.append(P[inst.contains(P)])
    return np.vstack(pts)

# Tests the geometry itself, without hits. Overlaps cannot be seen on a plot, so points are drawn INSIDE each volume
# and checked against the other volumes.
def check_overlaps(geom, n=200_000, tol=1e-6):
    """For each volume: draw points INSIDE it, then test
       (1) do they stick out of the mother volume?  (2) do they fall inside an unrelated volume?
       (mother/daughter pairs of the same lineage are ignored)."""
    rng = np.random.default_rng(1)
    names = [k for k in geom if geom[k].mother is not None]
    anc = {k: set(_ancestors(geom, k)) for k in geom}
    pairs, extr = {}, []
    for v in names:
        # random points inside volume v
        P = _sample_inside(geom[v], n, rng)
        if not len(P):
            print(f"  [!] no point drawn inside {v}")
            continue
        # test 1: points of v that are outside its mother = v sticks out of its mother (+tol: touching surfaces are not an error)
        m = geom[v].mother
        if m in geom:
            k = int((~geom[m].contains(P, +tol)).sum())
            if k:
                extr.append((v, m, k, len(P)))
        # test 2: points of v that are also inside another volume w = overlap. Volumes of the same lineage (mother / daughter) are skipped;
        # -tol ignores surfaces that only touch each other
        for w in names:
            if w == v or w in anc[v] or v in anc[w]:
                continue
            c = int(geom[w].contains(P, -tol).sum())
            # a pair (v, w) is tested in both directions: keep the larger fraction
            key = tuple(sorted((v, w)))
            if c and c / len(P) > pairs.get(key, (0, 0, 0))[2]:
                pairs[key] = (v, w, c / len(P))
    print(f"\nOverlaps between unrelated volumes (~{n:.0e} points drawn per volume):")
    if not pairs:
        print("  none")
    for v, w, f in sorted(pairs.values(), key=lambda t: -t[2]):
        print(f"  {v:26s} x {w:26s} : {100 * f:.2f} % of the points of {v}")
    print("Volumes sticking out of their mother:")
    if not extr:
        print("  none")
    for v, m, k, N in extr:
        print(f"  {v:26s} sticks out of {m:12s} : {k}/{N} points ({100 * k / N:.2f} %)")

# ----------------------------------------------------------------------------
# Reads a hits file (any separator), converts x, y, z to mm and, if there is a volume column,
# keeps only the rows of the selected volume(s).
def load_points(path, a):
    pts = pd.read_csv(path, sep=None, engine="python")
    for c in "xyz":
        pts[c] = pts[c] * a.scale
    if a.volume_col in pts.columns:
        pts = pts[pts[a.volume_col].astype(str).apply(lambda s: fnmatch.fnmatch(s, a.volume))].reset_index(drop=True)
    return pts

# Reads the command line, loads geometry and hits, prints a summary, draws and saves the figures.
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("volume", nargs="?", help="volume name (wildcards OK: 'GAS_*')")
    ap.add_argument("--geom", default=GEOM_FILE, help="theoretical geometry file")
    ap.add_argument("--points", nargs="+", help="1 to 3 Geant4 point files (CSV with columns x,y,z [,volume])")
    ap.add_argument("--labels", nargs="+", help="dataset legends (default: file names)")
    ap.add_argument("--volume-col", default="volume", help="name of the volume column in the point files")
    ap.add_argument("--scale", type=float, default=1.0, help="factor to convert to mm (e.g. 10 if the files are in cm)")
    ap.add_argument("--tol", type=float, default=0.0, help="tolerance [mm] on the base solid")
    ap.add_argument("--demo", action="store_true", help="synthetic points (to test the script)")
    ap.add_argument("--list", action="store_true", help="list the volumes")
    ap.add_argument("--overlaps", action="store_true", help="overlaps between volumes + volumes sticking out of their mother")
    ap.add_argument("--save", help="prefix used to save the figures (formats and dpi: STYLE['save'])")
    a = ap.parse_args()
    # fonts, colours, sizes... from STYLE
    apply_style()
    geom = load_geometry(a.geom)
    if a.list:
        print("\n".join(f"{n:26s} {v.base.shape:9s} {v.material:10s} {len(v.instances)} copy(ies), {len(v.ops)} operation(s)"
                        for n, v in geom.items()))
    if a.overlaps:
        check_overlaps(geom)
    if not a.volume:
        return
    # names of the volumes to check
    selected = select(geom, a.volume)
    raw = []   # [(label, DataFrame)]
    # --demo: random points around the volume (about 2 % are forced outside) to test the script without Geant4 files
    if a.demo:
        lo = np.min([geom[n].extent()[0] for n in selected], axis=0)
        hi = np.max([geom[n].extent()[1] for n in selected], axis=0)
        pad = 0.05 * (hi - lo)
        rng = np.random.default_rng(0)
        pts = pd.DataFrame(rng.uniform(lo - pad, hi + pad, (200000, 3)), columns=list("xyz"))
        pts = pts[inside_mask(geom, pts, selected) | (rng.random(len(pts)) < 0.02)].reset_index(drop=True)
        raw.append(("demo", pts))
    # normal case: one dataset per hits file; labels default to the file names
    elif a.points:
        if len(a.points) > 3:
            raise SystemExit("3 datasets maximum")
        labels = a.labels or [FilePath(p).stem for p in a.points]
        if len(set(labels)) < len(labels):
            labels = [f"{l} ({i})" for i, l in enumerate(labels)]
        raw = [(l, load_points(p, a)) for l, p in zip(labels, a.points)]
    datasets = []
    emin = emax = None
    if raw:
        ext = [geom[n].extent() for n in selected]
        emin, emax = np.min([e[0] for e in ext], 0), np.max([e[1] for e in ext], 0)
    # for each dataset: which hits are inside the geometry + a summary printed in the terminal
    for label, pts in raw:
        ok = inside_mask(geom, pts, selected, a.volume_col, a.tol)
        n_out = int((~ok).sum())
        print(f"\n[{label}] {a.volume}: {len(pts)} points, {len(pts) - n_out} inside, {n_out} outside "
              f"({100 * n_out / max(len(pts), 1):.3f} %)")
        for i, lab in enumerate("xyz"):
            v = pts[lab]
            print(f"  {lab}: points [{v.min():9.2f}, {v.max():9.2f}]   theory [{emin[i]:9.2f}, {emax[i]:9.2f}]")
        datasets.append(dict(label=label, pts=pts, ok=ok))
    # draw the figures; the distributions need at least one hits file
    print(datasets)
    figs = [("3d", plot_3d(geom, selected, datasets, title=TEXT["title_3d"].format(volume=a.volume)))]
    if datasets:
        figs.append(("dist", plot_distributions(geom, selected, datasets,
                                                title=TEXT["title_dist"].format(volume=a.volume))))
    # write the figures to files: prefix_3d.* and prefix_dist.*
    if a.save:
        for tag, fig in figs:
            save_fig(fig, a.save, tag)
            print(f"  figure saved: {a.save}_{tag}.[{','.join(STYLE['save']['formats'])}]")
    # does nothing with the Agg backend (kept in case you switch to an interactive backend)
    plt.show()

if __name__ == "__main__":
    main()
