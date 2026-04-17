#!/usr/bin/env python3
"""
Compare CHASM's original abstable (optical depth table) against the MODTRAN-derived one.

Default inputs:
- Original: src/CHASM/data/abstable.npz
- MODTRAN:  src/CHASM/data/absfile_MODTRAN.npz

This script produces several figures in the ./plots/ directory:
- tau_vs_height.png: tau(h) at selected wavelengths (nm)
- transmission_vs_height.png: T(h)=exp(-tau(h)) at selected wavelengths
- tau_vs_wavelength_at_heights.png: tau(lambda) at selected heights (km)
- percent_diff_tau_heatmap.png: percent difference heatmap across the grid

You can override inputs and selections via CLI:
  python3 demo/plot_atmabs_compare.py \
    --orig src/CHASM/data/abstable.npz \
    --modtran src/CHASM/data/absfile_MODTRAN.npz \
    --wl 300 350 400 500 700 \
    --hk 2 5 10 20
"""
from __future__ import annotations
import argparse
from pathlib import Path
import numpy as np
import matplotlib
matplotlib.use("Agg")  # headless
import matplotlib.pyplot as plt


def load_npz(path: Path) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    d = np.load(path)
    return d['wavelength'], d['height'], d['ecoeff']


def find_nearest_indices(values: np.ndarray, targets: np.ndarray) -> list[int]:
    idx = []
    for t in targets:
        i = int(np.argmin(np.abs(values - t)))
        idx.append(i)
    return idx


def plot_tau_vs_height(wavelengths_nm, heights_m, tau_orig, tau_mod, chosen_wls, out_dir: Path):
    plt.figure(figsize=(10,6))
    for wl in chosen_wls:
        i = int(np.argmin(np.abs(wavelengths_nm - wl)))
        plt.plot(heights_m/1000.0, tau_orig[i,:], label=f"orig τ @ {wavelengths_nm[i]:.0f} nm", ls='-')
        plt.plot(heights_m/1000.0, tau_mod[i,:], label=f"MODTRAN τ @ {wavelengths_nm[i]:.0f} nm", ls='--')
    plt.xlabel('Height (km)')
    plt.ylabel('Cumulative optical depth τ')
    plt.title('τ(h) comparison at selected wavelengths')
    plt.grid(True, ls='--', alpha=0.4)
    plt.legend(ncol=2, fontsize=9)
    out = out_dir / 'tau_vs_height.png'
    plt.tight_layout()
    plt.savefig(out, dpi=150)
    plt.close()


def plot_transmission_vs_height(wavelengths_nm, heights_m, tau_orig, tau_mod, chosen_wls, out_dir: Path):
    plt.figure(figsize=(10,6))
    for wl in chosen_wls:
        i = int(np.argmin(np.abs(wavelengths_nm - wl)))
        T_orig = np.exp(-tau_orig[i,:])
        T_mod  = np.exp(-tau_mod[i,:])
        plt.plot(heights_m/1000.0, T_orig, label=f"orig T @ {wavelengths_nm[i]:.0f} nm", ls='-')
        plt.plot(heights_m/1000.0, T_mod,  label=f"MODTRAN T @ {wavelengths_nm[i]:.0f} nm", ls='--')
    plt.xlabel('Height (km)')
    plt.ylabel('Transmission T = exp(-τ)')
    plt.yscale('log')
    plt.title('Transmission to ground vs height at selected wavelengths')
    plt.grid(True, ls='--', which='both', alpha=0.4)
    plt.legend(ncol=2, fontsize=9)
    out = out_dir / 'transmission_vs_height.png'
    plt.tight_layout()
    plt.savefig(out, dpi=150)
    plt.close()


def plot_tau_vs_wavelength_at_heights(wavelengths_nm, heights_m, tau_orig, tau_mod, heights_km, out_dir: Path):
    plt.figure(figsize=(10,6))
    for hk in heights_km:
        j = int(np.argmin(np.abs(heights_m/1000.0 - hk)))
        plt.plot(wavelengths_nm, tau_orig[:, j], label=f"orig τ @ {heights_m[j]/1000.0:.0f} km", ls='-')
        plt.plot(wavelengths_nm, tau_mod[:, j],  label=f"MODTRAN τ @ {heights_m[j]/1000.0:.0f} km", ls='--')
    plt.xlabel('Wavelength (nm)')
    plt.ylabel('Cumulative optical depth τ')
    plt.title('τ(λ) at selected heights')
    plt.grid(True, ls='--', alpha=0.4)
    plt.legend(ncol=2, fontsize=9)
    out = out_dir / 'tau_vs_wavelength_at_heights.png'
    plt.tight_layout()
    plt.savefig(out, dpi=150)
    plt.close()


def plot_percent_diff_heatmap(wavelengths_nm, heights_m, tau_orig, tau_mod, out_dir: Path):
    # percent difference: (mod - orig)/orig * 100; guard divide-by-zero
    denom = np.where(tau_orig != 0, tau_orig, np.nan)
    pct = (tau_mod - tau_orig) / denom * 100.0
    plt.figure(figsize=(10,6))
    H, W = np.meshgrid(heights_m/1000.0, wavelengths_nm)
    im = plt.pcolormesh(H, W, pct, shading='auto', cmap='coolwarm', vmin=-50, vmax=50)
    plt.colorbar(im, label='% diff τ (MODTRAN - orig)/orig')
    plt.xlabel('Height (km)')
    plt.ylabel('Wavelength (nm)')
    plt.title('Percent difference in τ across grid')
    plt.tight_layout()
    out = out_dir / 'percent_diff_tau_heatmap.png'
    plt.savefig(out, dpi=150)
    plt.close()


def main():
    ap = argparse.ArgumentParser(description='Compare original abstable with MODTRAN-derived abstable.')
    ap.add_argument('--orig', type=Path, default=Path('src/CHASM/data/abstable.npz'))
    ap.add_argument('--modtran', type=Path, default=Path('src/CHASM/data/atmabs_MODTRAN.npz'))
    ap.add_argument('--wl', type=float, nargs='*', default=[300, 350, 400, 500, 700], help='Wavelengths (nm) to plot')
    ap.add_argument('--hk', type=float, nargs='*', default=[2, 5, 10, 20], help='Heights (km) to plot')
    ap.add_argument('--out', type=Path, default=Path('plots'))
    args = ap.parse_args()

    # If user meant atmabs.npz, handle that alias
    if not args.orig.exists():
        alias = Path('src/CHASM/data/atmabs.npz')
        if alias.exists():
            print(f"Original file not found, using alias: {alias}")
            args.orig = alias

    wl_o, h_o, tau_o = load_npz(args.orig)
    wl_m, h_m, tau_m = load_npz(args.modtran)

    # Basic validations
    if not (np.allclose(wl_o, wl_m) and np.allclose(h_o, h_m)):
        print('Warning: grids differ between files; attempting to proceed using original grid only.')
        # For simplicity, attempt to reindex MODTRAN on the nearest indices of original grid
        # along both axes (assumes grids are very close). If not, recommend regenerating on same grid.
        # Map wavelengths
        idx_w = [int(np.argmin(np.abs(wl_m - w))) for w in wl_o]
        # Map heights
        idx_h = [int(np.argmin(np.abs(h_m - z))) for z in h_o]
        tau_m = tau_m[np.array(idx_w)[:,None], np.array(idx_h)[None,:]]
        wl_m, h_m = wl_o, h_o

    out_dir = args.out
    out_dir.mkdir(parents=True, exist_ok=True)

    # Generate plots
    plot_tau_vs_height(wl_o, h_o, tau_o, tau_m, args.wl, out_dir)
    plot_transmission_vs_height(wl_o, h_o, tau_o, tau_m, args.wl, out_dir)
    plot_tau_vs_wavelength_at_heights(wl_o, h_o, tau_o, tau_m, args.hk, out_dir)
    plot_percent_diff_heatmap(wl_o, h_o, tau_o, tau_m, out_dir)

    print(f"Saved comparison plots to: {out_dir.resolve()}")


if __name__ == '__main__':
    main()
