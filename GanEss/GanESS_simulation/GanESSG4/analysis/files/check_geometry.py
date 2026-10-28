#!/usr/bin/env python3
"""
Contour 3D de la géométrie théorique + vérification que des points Geant4 y sont contenus.

Usage :
  python check_geometry.py --list
  python check_geometry.py VESSEL_GAS_BODY                      # contour seul
  python check_geometry.py VESSEL_GAS_BODY --demo               # points synthétiques pour tester
  python check_geometry.py VESSEL_GAS_BODY --points hits.csv    # points Geant4 (colonnes x,y,z [,volume])
  python check_geometry.py "GAS_*" --points hits.csv            # wildcards acceptés
  python check_geometry.py --overlaps                           # recouvrements entre volumes (test MC)

Le fichier de géométrie (geometry_theory.csv) se lit avec pandas : pd.read_csv("geometry_theory.csv").
Unités : mm, degrés. Voir make_geometry_table.py pour la description des colonnes.
"""
import argparse
import fnmatch
import numpy as np
import pandas as pd
import matplotlib
#matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.path import Path
from mpl_toolkits.mplot3d.art3d import Line3DCollection

GEOM_FILE = "geometry_theory.csv"
ROLE_STYLE = {"base": dict(color="tab:blue", ls="-", lw=1.0),
              "subtract": dict(color="tab:red", ls="--", lw=0.8),
              "union": dict(color="tab:green", ls="-", lw=0.8)}


# ----------------------------------------------------------------------------
# Rotations (convention CLHEP : rotateX(a) => R = Rx(a) @ R_courant)
# ----------------------------------------------------------------------------
def _rot(axis, a):
    c, s = np.cos(np.radians(a)), np.sin(np.radians(a))
    return {"X": np.array([[1, 0, 0], [0, c, -s], [0, s, c]]),
            "Y": np.array([[c, 0, s], [0, 1, 0], [-s, 0, c]]),
            "Z": np.array([[c, -s, 0], [s, c, 0], [0, 0, 1]])}[axis]


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
        self.name, self.role, self.shape = row["name"], row["role"], row["shape"]
        self.hz = _f(row["hz"])
        self.rmin1, self.rmax1 = _f(row["rmin1"]), _f(row["rmax1"])
        self.rmin2 = _f(row["rmin2"], self.rmin1)
        self.rmax2 = _f(row["rmax2"], self.rmax1)
        self.ty1, self.ty2 = _f(row["ty1"]), _f(row["ty2"])
        self.tx1, self.tx2, self.tx3, self.tx4 = (_f(row[k]) for k in ("tx1", "tx2", "tx3", "tx4"))
        self.t = np.array([_f(row["x"]), _f(row["y"]), _f(row["z"])])
        self.R = parse_rot(row["rot"])
        self.poly = None
        if self.shape == "extruded":
            self.poly = np.array([[float(c) for c in p.split(":")] for p in row["polygon"].split(";")])

    # --- appartenance (coordonnées locales, N x 3) ---
    def contains(self, p, tol=0.0):
        x, y, z = p.T
        hz = self.hz
        inz = np.abs(z) <= hz + tol
        if self.shape in ("tubs", "cons"):
            r = np.hypot(x, y)
            t = np.clip((z + hz) / (2 * hz), 0, 1)
            rmin = self.rmin1 + (self.rmin2 - self.rmin1) * t
            rmax = self.rmax1 + (self.rmax2 - self.rmax1) * t
            return inz & (r >= rmin - tol) & (r <= rmax + tol)
        if self.shape == "trap":   # theta = phi = alpha = 0
            t = np.clip((z + hz) / (2 * hz), 0, 1)
            h = self.ty1 + (self.ty2 - self.ty1) * t
            xlo = self.tx1 + (self.tx3 - self.tx1) * t
            xhi = self.tx2 + (self.tx4 - self.tx2) * t
            u = np.clip((y + h) / (2 * h), 0, 1)
            hx = xlo + (xhi - xlo) * u
            return inz & (np.abs(y) <= h + tol) & (np.abs(x) <= hx + tol)
        if self.shape == "extruded":
            return inz & Path(self.poly).contains_points(p[:, :2], radius=-tol)
        raise ValueError(f"forme inconnue : {self.shape}")

    # --- contour (polylignes en coordonnées locales) ---
    def outline(self, n=72):
        hz, lines = self.hz, []
        if self.shape in ("tubs", "cons"):
            ph = np.linspace(0, 2 * np.pi, n + 1)
            for sgn, rmin, rmax in ((-1, self.rmin1, self.rmax1), (+1, self.rmin2, self.rmax2)):
                for r in (rmin, rmax):
                    if r > 0:
                        lines.append(np.c_[r * np.cos(ph), r * np.sin(ph), np.full_like(ph, sgn * hz)])
            for a in np.radians([0, 90, 180, 270]):
                for k in (0, 1):
                    r1, r2 = (self.rmin1, self.rmin2) if k == 0 else (self.rmax1, self.rmax2)
                    if r1 > 0 or r2 > 0:
                        lines.append(np.array([[r1 * np.cos(a), r1 * np.sin(a), -hz],
                                               [r2 * np.cos(a), r2 * np.sin(a), +hz]]))
        elif self.shape == "trap":
            c = lambda tx_a, tx_b, ty, z: [(-tx_a, -ty, z), (tx_a, -ty, z), (tx_b, ty, z), (-tx_b, ty, z)]
            lo = np.array(c(self.tx1, self.tx2, self.ty1, -hz))
            hi = np.array(c(self.tx3, self.tx4, self.ty2, +hz))
            lines += [np.vstack([lo, lo[:1]]), np.vstack([hi, hi[:1]])]
            lines += [np.array([lo[i], hi[i]]) for i in range(4)]
        elif self.shape == "extruded":
            P = self.poly
            lo = np.c_[P, np.full(len(P), -hz)]
            hi = np.c_[P, np.full(len(P), +hz)]
            lines += [np.vstack([lo, lo[:1]]), np.vstack([hi, hi[:1]])]
            lines += [np.array([lo[i], hi[i]]) for i in range(len(P))]
        return lines


# ----------------------------------------------------------------------------
# Volume = solide de base + opérations booléennes
# ----------------------------------------------------------------------------
class Volume:
    def __init__(self, name, df):
        self.name = name
        prims = [Prim(r) for _, r in df.iterrows()]
        assert prims[0].role == "base", f"{name}: la 1re ligne doit être 'base'"
        self.base, self.ops = prims[0], prims[1:]
        self.material = df.iloc[0]["material"]

    def contains(self, P, tol=0.0):
        pb = (P - self.base.t) @ self.base.R.T              # p_local = R (p - t)
        inside = self.base.contains(pb, tol)
        for op in self.ops:
            m = op.contains((pb - op.t) @ op.R.T)
            inside = inside & ~m if op.role == "subtract" else inside | m
        return inside

    def outlines(self):
        """[(polyligne globale (N,3), rôle)]"""
        out = []
        for ln in self.base.outline():
            out.append((ln @ self.base.R + self.base.t, "base"))
        for op in self.ops:
            for ln in op.outline():
                g = (ln @ op.R + op.t) @ self.base.R + self.base.t
                out.append((g, op.role))
        return out

    def extent(self):
        """(min, max) global en x, y, z du solide de base + unions (les soustractions sont ignorées)."""
        pts = np.vstack([l for l, role in self.outlines() if role != "subtract"])
        return pts.min(0), pts.max(0)

    def rz_profile(self):
        """Profil (z, r) du solide de base si c'est un tubs/cons d'axe z, sinon None."""
        b = self.base
        if b.shape not in ("tubs", "cons") or abs(abs(b.R[2, 2]) - 1) > 1e-6:
            return None
        s = b.R[2, 2]
        zlo, zhi = b.t[2] - s * b.hz, b.t[2] + s * b.hz       # z global de l'extrémité locale -hz / +hz
        return np.array([[zlo, b.rmin1], [zlo, b.rmax1], [zhi, b.rmax2], [zhi, b.rmin2], [zlo, b.rmin1]])


def load_geometry(path=GEOM_FILE):
    df = pd.read_csv(path)
    return {n: Volume(n, g) for n, g in df.groupby("volume", sort=False)}


def select(geom, pattern):
    names = [n for n in geom if fnmatch.fnmatch(n, pattern)]
    if not names:
        raise SystemExit(f"Aucun volume ne correspond à '{pattern}'. Volumes : {', '.join(geom)}")
    return names


# ----------------------------------------------------------------------------
# Contenance
# ----------------------------------------------------------------------------
def inside_mask(geom, pts, selected, vol_col=None, tol=0.0):
    """Vrai si le point est dans son volume (colonne volume) ou, sans colonne, dans un des volumes choisis."""
    P = pts[["x", "y", "z"]].to_numpy(float)
    ok = np.zeros(len(P), bool)
    if vol_col is not None and vol_col in pts.columns:
        for n, idx in pts.groupby(vol_col).indices.items():
            if n in geom:
                ok[idx] = geom[n].contains(P[idx], tol)
            else:
                print(f"  [!] volume '{n}' absent du fichier de géométrie : points ignorés (comptés dehors)")
    else:
        for n in selected:
            ok |= geom[n].contains(P, tol)
    return ok


# ----------------------------------------------------------------------------
# Graphiques
# ----------------------------------------------------------------------------
def plot_3d(geom, selected, pts=None, ok=None, max_pts=20000, title=""):
    fig = plt.figure(figsize=(9, 8))
    ax = fig.add_subplot(111, projection="3d")
    lo, hi = np.full(3, np.inf), np.full(3, -np.inf)
    for n in selected:
        v = geom[n]
        for role in ("base", "subtract", "union"):
            segs = [l for l, r in v.outlines() if r == role]
            if segs:
                ax.add_collection3d(Line3DCollection(segs, **ROLE_STYLE[role]))
        a, b = v.extent()
        lo, hi = np.minimum(lo, a), np.maximum(hi, b)
    if pts is not None and len(pts):
        rng = np.random.default_rng(0)
        idx = rng.choice(len(pts), min(max_pts, len(pts)), replace=False)
        P = pts[["x", "y", "z"]].to_numpy(float)[idx]
        good = ok[idx]
        ax.scatter(*P[good].T, s=2, c="k", alpha=0.4, label="dedans")
        ax.scatter(*P[~good].T, s=6, c="red", label="DEHORS")
        lo, hi = np.minimum(lo, P.min(0)), np.maximum(hi, P.max(0))
        ax.legend()
    for setter, a, b in zip((ax.set_xlim, ax.set_ylim, ax.set_zlim), lo, hi):
        setter(a, b)
    ax.set_box_aspect(np.maximum(hi - lo, 1))
    ax.set_xlabel("x [mm]"); ax.set_ylabel("y [mm]"); ax.set_zlabel("z [mm]")
    ax.set_title(title)
    fig.tight_layout()
    return fig


def plot_distributions(geom, selected, pts, ok, title=""):
    fig, axs = plt.subplots(2, 2, figsize=(12, 9))
    ext = [geom[n].extent() for n in selected]
    emin = np.min([e[0] for e in ext], axis=0)
    emax = np.max([e[1] for e in ext], axis=0)
    P = pts[["x", "y", "z"]].to_numpy(float)
    for i, (ax, lab) in enumerate(zip(axs.flat[:3], "xyz")):
        lo, hi = min(emin[i], P[:, i].min()), max(emax[i], P[:, i].max())
        bins = np.linspace(lo, hi, 120)
        ax.hist(P[ok, i], bins=bins, color="tab:blue", alpha=0.7, label="dedans")
        ax.hist(P[~ok, i], bins=bins, color="red", alpha=0.8, label="dehors")
        for e in (emin[i], emax[i]):
            ax.axvline(e, color="k", ls="--")
        ax.set_xlabel(f"{lab} [mm]"); ax.set_ylabel("évènements"); ax.set_yscale("log")
        ax.legend(title=f"limites théo : [{emin[i]:.1f}, {emax[i]:.1f}]")
    ax = axs.flat[3]
    r = np.hypot(P[:, 0], P[:, 1])
    h = ax.hist2d(P[:, 2], r, bins=120, cmap="viridis", cmin=1)
    plt.colorbar(h[3], ax=ax, label="évènements")
    for n in selected:
        prof = geom[n].rz_profile()
        if prof is not None:
            ax.plot(prof[:, 0], prof[:, 1], "r-", lw=1.2)
    ax.set_xlabel("z [mm]"); ax.set_ylabel("r = sqrt(x²+y²) [mm]")
    ax.set_title("profil (z, r) : enveloppe du solide de base en rouge")
    fig.suptitle(title)
    fig.tight_layout()
    return fig


# ----------------------------------------------------------------------------
# Recouvrements (test Monte-Carlo entre volumes, hors MOTHER_VOLUME)
# ----------------------------------------------------------------------------
def check_overlaps(geom, n=1_000_000, tol=1e-6):
    names = [k for k in geom if k != "MOTHER_VOLUME"]
    m = geom["MOTHER_VOLUME"].base
    rng = np.random.default_rng(1)
    r = m.rmax1 * np.sqrt(rng.random(n))
    ph = 2 * np.pi * rng.random(n)
    P = np.c_[r * np.cos(ph), r * np.sin(ph), rng.uniform(-m.hz, m.hz, n)]
    M = np.column_stack([geom[k].contains(P, -tol) for k in names]).astype(np.int32)
    C = M.T @ M
    vol_mother = np.pi * m.rmax1 ** 2 * 2 * m.hz
    print(f"\nRecouvrements ({n:.0e} points tirés dans MOTHER_VOLUME) :")
    found = False
    for i in range(len(names)):
        for j in range(i + 1, len(names)):
            if C[i, j]:
                found = True
                print(f"  {names[i]:20s} x {names[j]:20s} : {C[i, j]} points  (~{C[i, j] / n * vol_mother / 1e3:.0f} cm3)")
    if not found:
        print("  aucun")


# ----------------------------------------------------------------------------
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("volume", nargs="?", help="nom du volume (wildcards OK : 'GAS_*')")
    ap.add_argument("--geom", default=GEOM_FILE, help="fichier de géométrie théorique")
    ap.add_argument("--points", help="fichier de points Geant4 (CSV avec colonnes x,y,z [,volume])")
    ap.add_argument("--volume-col", default="volume", help="colonne du nom de volume dans le fichier de points")
    ap.add_argument("--scale", type=float, default=1.0, help="facteur vers mm (ex. 10 si le fichier est en cm)")
    ap.add_argument("--tol", type=float, default=0.0, help="tolérance [mm] sur le solide de base")
    ap.add_argument("--demo", action="store_true", help="points synthétiques (test du script)")
    ap.add_argument("--list", action="store_true", help="liste les volumes")
    ap.add_argument("--overlaps", action="store_true", help="test de recouvrement entre volumes")
    ap.add_argument("--save", help="préfixe pour sauvegarder les figures en PNG")
    a = ap.parse_args()

    geom = load_geometry(a.geom)
    if a.list:
        print("\n".join(f"{n:20s} {v.base.shape:9s} {v.material:10s} {len(v.ops)} opération(s)" for n, v in geom.items()))
    if a.overlaps:
        check_overlaps(geom)
    if not a.volume:
        return

    selected = select(geom, a.volume)
    pts = ok = None
    if a.demo:
        lo = np.min([geom[n].extent()[0] for n in selected], axis=0)
        hi = np.max([geom[n].extent()[1] for n in selected], axis=0)
        pad = 0.05 * (hi - lo)
        rng = np.random.default_rng(0)
        pts = pd.DataFrame(rng.uniform(lo - pad, hi + pad, (200000, 3)), columns=list("xyz"))
        pts = pts[inside_mask(geom, pts, selected) | (rng.random(len(pts)) < 0.02)].reset_index(drop=True)
    elif a.points:
        pts = pd.read_csv(a.points, sep=None, engine="python")
        for c in "xyz":
            pts[c] = pts[c] * a.scale
        if a.volume_col in pts.columns:
            pts = pts[pts[a.volume_col].astype(str).apply(lambda s: fnmatch.fnmatch(s, a.volume))].reset_index(drop=True)
    if pts is not None:
        ok = inside_mask(geom, pts, selected, a.volume_col, a.tol)
        n_out = int((~ok).sum())
        print(f"\n{a.volume} : {len(pts)} points, {len(pts) - n_out} dedans, {n_out} dehors ({100 * n_out / max(len(pts), 1):.3f} %)")
        ext = [geom[n].extent() for n in selected]
        emin, emax = np.min([e[0] for e in ext], 0), np.max([e[1] for e in ext], 0)
        for i, lab in enumerate("xyz"):
            v = pts[lab]
            print(f"  {lab}: points [{v.min():9.2f}, {v.max():9.2f}]   théorie [{emin[i]:9.2f}, {emax[i]:9.2f}]")

    title = f"{a.volume}" + ("" if pts is None else f"  -  {len(pts)} points")
    plot_3d(geom, selected, pts, ok, title=title)
    if a.save:
        plt.gcf().savefig(f"{a.save}_3d.png", dpi=150)
    if pts is not None:
        plot_distributions(geom, selected, pts, ok, title=title)
        if a.save:
            plt.gcf().savefig(f"{a.save}_dist.png", dpi=150)
    plt.show()


if __name__ == "__main__":
    main()
