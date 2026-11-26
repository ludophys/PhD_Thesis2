#!/usr/bin/env python3
"""
Contour 3D de la géométrie théorique + vérification que des points Geant4 y sont contenus.

Usage :
  python check_geometry.py --list
  python check_geometry.py VESSEL_GAS_BODY
  python check_geometry.py VESSEL_GAS_BODY --demo
  python check_geometry.py VESSEL_GAS_BODY --points hits.csv
  python check_geometry.py "GAS_*" --points hits.csv
  python check_geometry.py --overlaps

Sauvegardes :
  - Figures PNG : avec --save PREFIXE

Le style de tous les graphiques se règle dans le bloc STYLE en haut du fichier.

Unités : mm, degrés.
"""

import argparse
import fnmatch
from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from matplotlib.colors import LogNorm
from matplotlib.path import Path as MplPath
from mpl_toolkits.mplot3d.art3d import Line3DCollection


GEOM_FILE = "geometry_theory.csv"


# ============================================================================
# STYLE DES GRAPHIQUES  (tout se modifie ici)
# ----------------------------------------------------------------------------
# - Les dictionnaires nommés "*_kw" sont passés tels quels à Matplotlib :
#   on peut donc y ajouter n'importe quel argument valide de la fonction
#   correspondante (ex. "zorder", "linestyle", "edgecolor", ...).
# - Les clés "show" activent/désactivent un élément (True / False).
# ============================================================================

STYLE = {

    # ------------------------------------------------------------------
    # Paramètres globaux Matplotlib (rcParams) : police, axes, etc.
    # ------------------------------------------------------------------
    "rcparams": {
        "axes.prop_cycle": plt.cycler(color=sns.color_palette("colorblind")),
        "font.family": "sans-serif",
        "font.size": 10,
        "axes.linewidth": 1.0,
        "axes.edgecolor": "black",
        "axes.facecolor": "white",
        "figure.facecolor": "white",
        "xtick.direction": "out",
        "ytick.direction": "out",
        "mathtext.default": "regular",
    },

    # ------------------------------------------------------------------
    # Paramètres communs de sauvegarde PNG (--save)
    # ------------------------------------------------------------------
    "savefig_kw": {
        "dpi": 150,
        "bbox_inches": "tight",
        "transparent": False,
    },

    # ------------------------------------------------------------------
    # FIGURE 1 : géométrie 3D
    # ------------------------------------------------------------------
    "fig_3d": {
        "figsize": (9, 8),
        "tight_layout": {},                     # None pour désactiver

        # Titre ({title} = nom du volume + nombre de points)
        "title_kw": {"fontsize": 14, "fontweight": "normal", "pad": 10},

        # Axes
        "xlabel": "x [mm]",
        "ylabel": "y [mm]",
        "zlabel": "z [mm]",
        "label_kw": {"fontsize": 14, "labelpad": 6},
        "tick_kw": {"labelsize": 9},
        "grid": True,
        "axes_off": False,                      # True : cache axes + fond
        "pane_color": (1.0, 1.0, 1.0, 0.0),     # fond des 3 plans (RGBA)
        "view": {"elev": 25, "azim": -60},      # angle de vue initial
        "box_aspect": False,                     # proportions réelles du volume

        # Contours de la géométrie, par rôle booléen
        "roles": {
            "base":     {"show": True, "color": "tab:blue",  "ls": "-",  "lw": 1.0},
            "subtract": {"show": True, "color": "tab:red",   "ls": "--", "lw": 0.8},
            "union":    {"show": True, "color": "tab:green", "ls": "-",  "lw": 0.8},
        },

        # Points Geant4
        "max_pts": 20000,                       # sous-échantillonnage affichage
        "inside": {
            "show": True,
            "s": 3, "c": "k", "alpha": 0.4, "marker": "o",
            "label": "Inside",
        },
        "outside": {
            "show": True,
            "s": 3, "c": "red", "alpha": 1.0, "marker": "o",
            "label": "Outside",
        },

        # Légende
        "legend": {
            "show": True,
            "loc": "best", "fontsize": 12, "frameon": True,
        },
    },

    # ------------------------------------------------------------------
    # FIGURE 2 : distributions (x, y, z, et (z, r))
    # ------------------------------------------------------------------
    "fig_dist": {
        "figsize": (12, 9),
        "tight_layout": {},                     # ex. {"rect": (0, 0, 1, 0.97)}
        "suptitle_kw": {"fontsize": 15, "fontweight": "normal"},

        # --- Histogrammes 1D (x, y, z) ---
        "hist": {
            "bins": 120,
            "yscale": "log",                    # "log" ou "linear"
            "xlabel": "{lab} [mm]",             # {lab} = x, y ou z
            "ylabel": "Events",
            "title": "",
            "label_kw": {"fontsize": 14},
            "title_kw": {"fontsize": 11},
            "tick_kw": {"labelsize": 12},
            "grid": {"visible": False, "alpha": 0.3, "ls": ":"},

            "inside": {
                "show": True,
                "color": "tab:blue", "alpha": 1.0, "label": "Inside",
                "histtype": "bar", "edgecolor": "none",
            },
            "outside": {
                "show": True,
                "color": "red", "alpha": 1.0, "label": "Outside",
                "histtype": "bar", "edgecolor": "none",
            },

            # Lignes verticales = limites théoriques du volume
            "limits": {"show": True, "color": "red", "ls": "--", "lw": 1.0},

            "legend": {
                "show": True,
                "title_fmt": "Limits : [{emin:.1f}, {emax:.1f}]",
                "loc": "best", "fontsize": 10, "title_fontsize": 10,
                "frameon": True, "framealpha": 1.0,
            },
        },

        # --- Histogramme 2D (z, r) ---
        "hist2d": {
            "bins": 120,
            "cmap": "viridis",
            "cmin": 1,
            "log_color": False,                 # True : échelle de couleur log
            "xlabel": "z [mm]",
            "ylabel": "r = sqrt(x²+y²) [mm]",
            "title": "(z, r) profil",
            "label_kw": {"fontsize": 14},
            "title_kw": {"fontsize": 14},
            "tick_kw": {"labelsize": 12},
            "grid": {"visible": False, "alpha": 0.3, "ls": ":"},

            "colorbar": {
                "show": True,
                "label": "Events",
                "label_kw": {"fontsize": 14},
                "tick_kw": {"labelsize": 12},
            },

            # Profil théorique (z, r) superposé
            "profile": {"show": True, "color": "r", "ls": "-", "lw": 1.2},
        },
    },
}


def _kw(d, drop=("show",)):
    """Copie d'un dictionnaire de style sans les clés non-Matplotlib."""
    return {k: v for k, v in d.items() if k not in drop}


def apply_global_style():
    plt.rcParams.update(STYLE["rcparams"])


# ----------------------------------------------------------------------------
# Rotations
# ----------------------------------------------------------------------------

def _rot(axis, a):
    c, s = np.cos(np.radians(a)), np.sin(np.radians(a))

    return {
        "X": np.array([[1, 0, 0], [0, c, -s], [0, s, c]]),
        "Y": np.array([[c, 0, s], [0, 1, 0], [-s, 0, c]]),
        "Z": np.array([[c, -s, 0], [s, c, 0], [0, 0, 1]])
    }[axis]


def parse_rot(s):
    R = np.eye(3)

    if isinstance(s, str) and s.strip():
        for tok in s.replace(" ", "").split(","):
            R = _rot(tok[0].upper(), float(tok[1:])) @ R

    return R


def _f(v, default=0.0):
    return default if pd.isna(v) else float(v)


# ----------------------------------------------------------------------------
# Primitives
# ----------------------------------------------------------------------------

class Prim:

    def __init__(self, row):
        self.name = row["name"]
        self.role = row["role"]
        self.shape = row["shape"]

        self.hz = _f(row["hz"])

        self.rmin1 = _f(row["rmin1"])
        self.rmax1 = _f(row["rmax1"])
        self.rmin2 = _f(row["rmin2"], self.rmin1)
        self.rmax2 = _f(row["rmax2"], self.rmax1)

        self.ty1 = _f(row["ty1"])
        self.ty2 = _f(row["ty2"])

        self.tx1, self.tx2, self.tx3, self.tx4 = (
            _f(row[k]) for k in ("tx1", "tx2", "tx3", "tx4")
        )

        self.t = np.array([
            _f(row["x"]),
            _f(row["y"]),
            _f(row["z"])
        ])

        self.R = parse_rot(row["rot"])

        self.poly = None

        if self.shape == "extruded":
            self.poly = np.array([
                [float(c) for c in p.split(":")]
                for p in row["polygon"].split(";")
            ])

    # --- Appartenance ---

    def contains(self, p, tol=0.0):

        x, y, z = p.T
        hz = self.hz

        inz = np.abs(z) <= hz + tol

        if self.shape in ("tubs", "cons"):

            r = np.hypot(x, y)
            t = np.clip((z + hz) / (2 * hz), 0, 1)

            rmin = self.rmin1 + (self.rmin2 - self.rmin1) * t
            rmax = self.rmax1 + (self.rmax2 - self.rmax1) * t

            return (
                inz
                & (r >= rmin - tol)
                & (r <= rmax + tol)
            )

        if self.shape == "trap":

            t = np.clip((z + hz) / (2 * hz), 0, 1)

            h = self.ty1 + (self.ty2 - self.ty1) * t
            xlo = self.tx1 + (self.tx3 - self.tx1) * t
            xhi = self.tx2 + (self.tx4 - self.tx2) * t

            u = np.clip((y + h) / (2 * h), 0, 1)
            hx = xlo + (xhi - xlo) * u

            return (
                inz
                & (np.abs(y) <= h + tol)
                & (np.abs(x) <= hx + tol)
            )

        if self.shape == "extruded":
            return inz & MplPath(self.poly).contains_points(
                p[:, :2], radius=-tol
            )

        raise ValueError(f"forme inconnue : {self.shape}")

    # --- Contour ---

    def outline(self, n=72):

        hz = self.hz
        lines = []

        if self.shape in ("tubs", "cons"):

            ph = np.linspace(0, 2 * np.pi, n + 1)

            for sgn, rmin, rmax in (
                (-1, self.rmin1, self.rmax1),
                (+1, self.rmin2, self.rmax2)
            ):

                for r in (rmin, rmax):
                    if r > 0:
                        lines.append(
                            np.c_[
                                r * np.cos(ph),
                                r * np.sin(ph),
                                np.full_like(ph, sgn * hz)
                            ]
                        )

            for a in np.radians([0, 90, 180, 270]):

                for k in (0, 1):

                    r1, r2 = (
                        (self.rmin1, self.rmin2)
                        if k == 0
                        else (self.rmax1, self.rmax2)
                    )

                    if r1 > 0 or r2 > 0:
                        lines.append(np.array([
                            [r1 * np.cos(a), r1 * np.sin(a), -hz],
                            [r2 * np.cos(a), r2 * np.sin(a), +hz]
                        ]))

        elif self.shape == "trap":

            c = lambda tx_a, tx_b, ty, z: [
                (-tx_a, -ty, z),
                (tx_a, -ty, z),
                (tx_b, ty, z),
                (-tx_b, ty, z)
            ]

            lo = np.array(c(self.tx1, self.tx2, self.ty1, -hz))
            hi = np.array(c(self.tx3, self.tx4, self.ty2, +hz))

            lines += [
                np.vstack([lo, lo[:1]]),
                np.vstack([hi, hi[:1]])
            ]

            lines += [
                np.array([lo[i], hi[i]])
                for i in range(4)
            ]

        elif self.shape == "extruded":

            P = self.poly

            lo = np.c_[P, np.full(len(P), -hz)]
            hi = np.c_[P, np.full(len(P), +hz)]

            lines += [
                np.vstack([lo, lo[:1]]),
                np.vstack([hi, hi[:1]])
            ]

            lines += [
                np.array([lo[i], hi[i]])
                for i in range(len(P))
            ]

        return lines


# ----------------------------------------------------------------------------
# Volume = solide de base + opérations booléennes
# ----------------------------------------------------------------------------

class Volume:

    def __init__(self, name, df):

        self.name = name

        prims = [Prim(r) for _, r in df.iterrows()]

        assert prims[0].role == "base", (
            f"{name}: la 1re ligne doit être 'base'"
        )

        self.base = prims[0]
        self.ops = prims[1:]
        self.material = df.iloc[0]["material"]

    def contains(self, P, tol=0.0):

        pb = (P - self.base.t) @ self.base.R.T

        inside = self.base.contains(pb, tol)

        for op in self.ops:

            m = op.contains((pb - op.t) @ op.R.T)

            inside = (
                inside & ~m
                if op.role == "subtract"
                else inside | m
            )

        return inside

    def outlines(self):

        out = []

        for ln in self.base.outline():
            out.append((
                ln @ self.base.R + self.base.t,
                "base"
            ))

        for op in self.ops:

            for ln in op.outline():

                g = (
                    (ln @ op.R + op.t)
                    @ self.base.R
                    + self.base.t
                )

                out.append((g, op.role))

        return out

    def extent(self):

        pts = np.vstack([
            l for l, role in self.outlines()
            if role != "subtract"
        ])

        return pts.min(0), pts.max(0)

    def rz_profile(self):

        b = self.base

        if (
            b.shape not in ("tubs", "cons")
            or abs(abs(b.R[2, 2]) - 1) > 1e-6
        ):
            return None

        s = b.R[2, 2]

        zlo = b.t[2] - s * b.hz
        zhi = b.t[2] + s * b.hz

        return np.array([
            [zlo, b.rmin1],
            [zlo, b.rmax1],
            [zhi, b.rmax2],
            [zhi, b.rmin2],
            [zlo, b.rmin1]
        ])


def load_geometry(path=GEOM_FILE):

    df = pd.read_csv(path)

    return {
        n: Volume(n, g)
        for n, g in df.groupby("volume", sort=False)
    }


def select(geom, pattern):

    names = [
        n for n in geom
        if fnmatch.fnmatch(n, pattern)
    ]

    if not names:
        raise SystemExit(
            f"Aucun volume ne correspond à '{pattern}'. "
            f"Volumes : {', '.join(geom)}"
        )

    return names


# ----------------------------------------------------------------------------
# Contenance
# ----------------------------------------------------------------------------

def inside_mask(geom, pts, selected, vol_col=None, tol=0.0):

    P = pts[["x", "y", "z"]].to_numpy(float)

    ok = np.zeros(len(P), bool)

    if vol_col is not None and vol_col in pts.columns:

        for n, idx in pts.groupby(vol_col).indices.items():

            if n in geom:
                ok[idx] = geom[n].contains(P[idx], tol)

            else:
                print(
                    f"  [!] volume '{n}' absent du fichier de géométrie : "
                    "points ignorés (comptés dehors)"
                )

    else:

        for n in selected:
            ok |= geom[n].contains(P, tol)

    return ok


# ----------------------------------------------------------------------------
# Graphiques
# ----------------------------------------------------------------------------

def plot_3d(geom, selected, pts=None, ok=None, title=""):

    S = STYLE["fig_3d"]

    fig = plt.figure(figsize=S["figsize"])
    ax = fig.add_subplot(111, projection="3d")

    lo = np.full(3, np.inf)
    hi = np.full(3, -np.inf)

    # --- Contours de la géométrie ---
    for n in selected:

        v = geom[n]

        for role in ("base", "subtract", "union"):

            st = S["roles"][role]

            if not st["show"]:
                continue

            segs = [
                l for l, r in v.outlines()
                if r == role
            ]

            if segs:
                ax.add_collection3d(
                    Line3DCollection(segs, **_kw(st))
                )

        a, b = v.extent()

        lo = np.minimum(lo, a)
        hi = np.maximum(hi, b)

    # --- Points Geant4 ---
    if pts is not None and len(pts):

        rng = np.random.default_rng(0)

        idx = rng.choice(
            len(pts),
            min(S["max_pts"], len(pts)),
            replace=False
        )

        P = pts[["x", "y", "z"]].to_numpy(float)[idx]
        good = ok[idx]

        if S["inside"]["show"]:
            ax.scatter(*P[good].T, **_kw(S["inside"]))

        if S["outside"]["show"]:
            ax.scatter(*P[~good].T, **_kw(S["outside"]))

        lo = np.minimum(lo, P.min(0))
        hi = np.maximum(hi, P.max(0))

        if S["legend"]["show"]:
            ax.legend(**_kw(S["legend"]))

    # --- Limites et proportions ---
    for setter, a, b in zip(
        (ax.set_xlim, ax.set_ylim, ax.set_zlim),
        lo, hi
    ):
        setter(a, b)

    if S["box_aspect"]:
        ax.set_box_aspect(np.maximum(hi - lo, 1))

    # --- Axes, labels, titre ---
    ax.set_xlabel(S["xlabel"], **S["label_kw"])
    ax.set_ylabel(S["ylabel"], **S["label_kw"])
    ax.set_zlabel(S["zlabel"], **S["label_kw"])

    ax.tick_params(**S["tick_kw"])
    ax.grid(S["grid"])

    for axis in (ax.xaxis, ax.yaxis, ax.zaxis):
        axis.set_pane_color(S["pane_color"])

    if S["axes_off"]:
        ax.set_axis_off()

    if S["view"]:
        ax.view_init(**S["view"])

    ax.set_title(title, **S["title_kw"])

    if S["tight_layout"] is not None:
        fig.tight_layout(**S["tight_layout"])

    return fig


def plot_distributions(geom, selected, pts, ok, title=""):

    S = STYLE["fig_dist"]
    H = S["hist"]
    H2 = S["hist2d"]

    fig, axs = plt.subplots(2, 2, figsize=S["figsize"])

    ext = [
        geom[n].extent()
        for n in selected
    ]

    emin = np.min(
        [e[0] for e in ext],
        axis=0
    )

    emax = np.max(
        [e[1] for e in ext],
        axis=0
    )

    P = pts[["x", "y", "z"]].to_numpy(float)

    # --- Histogrammes X, Y et Z ---
    for i, (ax, lab) in enumerate(
        zip(axs.flat[:3], "xyz")
    ):

        lo = min(emin[i], P[:, i].min())
        hi = max(emax[i], P[:, i].max())

        bins = np.linspace(lo, hi, H["bins"])

        if H["inside"]["show"]:
            ax.hist(P[ok, i], bins=bins, **_kw(H["inside"]))

        if H["outside"]["show"]:
            ax.hist(P[~ok, i], bins=bins, **_kw(H["outside"]))

        if H["limits"]["show"]:
            for e in (emin[i], emax[i]):
                ax.axvline(e, **_kw(H["limits"]))

        ax.set_xlabel(H["xlabel"].format(lab=lab), **H["label_kw"])
        ax.set_ylabel(H["ylabel"], **H["label_kw"])
        ax.set_title(H["title"].format(lab=lab), **H["title_kw"])
        ax.set_yscale(H["yscale"])
        ax.tick_params(**H["tick_kw"])
        ax.grid(**H["grid"])

        if H["legend"]["show"]:
            lg = _kw(H["legend"], drop=("show", "title_fmt"))
            lg["title"] = H["legend"]["title_fmt"].format(
                emin=emin[i], emax=emax[i]
            )
            ax.legend(**lg)

    # --- Distribution 2D (z, r) ---
    ax = axs.flat[3]

    r = np.hypot(P[:, 0], P[:, 1])

    h2_kw = dict(bins=H2["bins"], cmap=H2["cmap"], cmin=H2["cmin"])

    if H2["log_color"]:
        h2_kw["norm"] = LogNorm()

    h = ax.hist2d(P[:, 2], r, **h2_kw)

    if H2["colorbar"]["show"]:
        cb = fig.colorbar(h[3], ax=ax)
        cb.set_label(H2["colorbar"]["label"], **H2["colorbar"]["label_kw"])
        cb.ax.tick_params(**H2["colorbar"]["tick_kw"])

    # Profil théorique
    if H2["profile"]["show"]:
        for n in selected:

            prof = geom[n].rz_profile()

            if prof is not None:
                ax.plot(
                    prof[:, 0],
                    prof[:, 1],
                    **_kw(H2["profile"])
                )

    ax.set_xlabel(H2["xlabel"], **H2["label_kw"])
    ax.set_ylabel(H2["ylabel"], **H2["label_kw"])
    ax.set_title(H2["title"], **H2["title_kw"])
    ax.tick_params(**H2["tick_kw"])
    ax.grid(**H2["grid"])

    fig.suptitle(title, **S["suptitle_kw"])

    if S["tight_layout"] is not None:
        fig.tight_layout(**S["tight_layout"])

    return fig


# ----------------------------------------------------------------------------
# Recouvrements (test Monte-Carlo)
# ----------------------------------------------------------------------------

def check_overlaps(geom, n=1_000_000, tol=1e-6):

    names = [
        k for k in geom
        if k != "MOTHER_VOLUME"
    ]

    m = geom["MOTHER_VOLUME"].base

    rng = np.random.default_rng(1)

    r = m.rmax1 * np.sqrt(rng.random(n))
    ph = 2 * np.pi * rng.random(n)

    P = np.c_[
        r * np.cos(ph),
        r * np.sin(ph),
        rng.uniform(-m.hz, m.hz, n)
    ]

    M = np.column_stack([
        geom[k].contains(P, -tol)
        for k in names
    ]).astype(np.int32)

    C = M.T @ M

    vol_mother = np.pi * m.rmax1 ** 2 * 2 * m.hz

    print(
        f"\nRecouvrements ({n:.0e} points tirés "
        "dans MOTHER_VOLUME) :"
    )

    found = False

    for i in range(len(names)):

        for j in range(i + 1, len(names)):

            if C[i, j]:

                found = True

                print(
                    f"  {names[i]:20s} x {names[j]:20s} : "
                    f"{C[i, j]} points "
                    f"(~{C[i, j] / n * vol_mother / 1e3:.0f} cm3)"
                )

    if not found:
        print("  aucun")


# ----------------------------------------------------------------------------
# Programme principal
# ----------------------------------------------------------------------------

def main():

    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter
    )

    ap.add_argument(
        "volume",
        nargs="?",
        help="Nom du volume (wildcards OK : 'GAS_*')"
    )

    ap.add_argument(
        "--geom",
        default=GEOM_FILE,
        help="Fichier de géométrie théorique"
    )

    ap.add_argument(
        "--points",
        help="Fichier de points Geant4 (CSV avec colonnes x,y,z [,volume])"
    )

    ap.add_argument(
        "--volume-col",
        default="volume",
        help="Colonne du nom de volume dans le fichier de points"
    )

    ap.add_argument(
        "--scale",
        type=float,
        default=1.0,
        help="Facteur vers mm (ex. 10 si le fichier est en cm)"
    )

    ap.add_argument(
        "--tol",
        type=float,
        default=0.0,
        help="Tolérance [mm] sur le solide de base"
    )

    ap.add_argument(
        "--demo",
        action="store_true",
        help="Points synthétiques (test du script)"
    )

    ap.add_argument(
        "--list",
        action="store_true",
        help="Liste les volumes"
    )

    ap.add_argument(
        "--overlaps",
        action="store_true",
        help="Test de recouvrement entre volumes"
    )

    ap.add_argument(
        "--save",
        help="Préfixe pour sauvegarder les figures en PNG"
    )

    a = ap.parse_args()

    apply_global_style()

    # ------------------------------------------------------------------------
    # Chargement géométrie
    # ------------------------------------------------------------------------

    geom = load_geometry(a.geom)

    if a.list:

        print(
            "\n".join(
                f"{n:20s} {v.base.shape:9s} "
                f"{v.material:10s} {len(v.ops)} opération(s)"
                for n, v in geom.items()
            )
        )

    if a.overlaps:
        check_overlaps(geom)

    if not a.volume:
        return

    selected = select(geom, a.volume)

    # ------------------------------------------------------------------------
    # Chargement des points
    # ------------------------------------------------------------------------

    pts = ok = None

    if a.demo:

        lo = np.min(
            [geom[n].extent()[0] for n in selected],
            axis=0
        )

        hi = np.max(
            [geom[n].extent()[1] for n in selected],
            axis=0
        )

        pad = 0.05 * (hi - lo)

        rng = np.random.default_rng(0)

        pts = pd.DataFrame(
            rng.uniform(
                lo - pad,
                hi + pad,
                (200000, 3)
            ),
            columns=list("xyz")
        )

        pts = pts[
            inside_mask(geom, pts, selected)
            | (rng.random(len(pts)) < 0.02)
        ].reset_index(drop=True)

    elif a.points:

        pts = pd.read_csv(
            a.points,
            sep=None,
            engine="python"
        )

        for c in "xyz":
            pts[c] = pts[c] * a.scale

        if a.volume_col in pts.columns:

            pts = pts[
                pts[a.volume_col].astype(str).apply(
                    lambda s: fnmatch.fnmatch(s, a.volume)
                )
            ].reset_index(drop=True)

    # ------------------------------------------------------------------------
    # Vérification des points
    # ------------------------------------------------------------------------

    if pts is not None:

        ok = inside_mask(
            geom,
            pts,
            selected,
            a.volume_col,
            a.tol
        )

        n_out = int((~ok).sum())

        print(
            f"\n{a.volume} : {len(pts)} points, "
            f"{len(pts) - n_out} dedans, "
            f"{n_out} dehors "
            f"({100 * n_out / max(len(pts), 1):.3f} %)"
        )

        ext = [
            geom[n].extent()
            for n in selected
        ]

        emin = np.min(
            [e[0] for e in ext],
            axis=0
        )

        emax = np.max(
            [e[1] for e in ext],
            axis=0
        )

        for i, lab in enumerate("xyz"):

            v = pts[lab]

            print(
                f"  {lab}: points "
                f"[{v.min():9.2f}, {v.max():9.2f}] "
                f"théorie "
                f"[{emin[i]:9.2f}, {emax[i]:9.2f}]"
            )

    title = (
        f"{a.volume}"
        + ("" if pts is None else f"  -  {len(pts)} points")
    )

    # ------------------------------------------------------------------------
    # FIGURE 1 : GEOMETRIE 3D
    # ------------------------------------------------------------------------

    print("\nGénération de la figure 3D...")

    fig_3d = plot_3d(
        geom,
        selected,
        pts,
        ok,
        title=title
    )

    if a.save:

        png_3d = Path(f"{a.save}_3d.png")
        png_3d.parent.mkdir(parents=True, exist_ok=True)

        fig_3d.savefig(png_3d, **STYLE["savefig_kw"])

        print(f"Figure PNG sauvegardée : {png_3d}")

    # ------------------------------------------------------------------------
    # FIGURE 2 : DISTRIBUTIONS
    # ------------------------------------------------------------------------

    if pts is not None:

        print("\nGénération des distributions...")

        fig_dist = plot_distributions(
            geom,
            selected,
            pts,
            ok,
            title=title
        )

        if a.save:

            png_dist = Path(f"{a.save}_dist.png")
            png_dist.parent.mkdir(parents=True, exist_ok=True)

            fig_dist.savefig(png_dist, **STYLE["savefig_kw"])

            print(f"Figure PNG sauvegardée : {png_dist}")

    print("\nToutes les figures ont été générées.")

    plt.show()


if __name__ == "__main__":
    main()