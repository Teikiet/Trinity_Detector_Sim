import numpy as np
import matplotlib.pyplot as plt
from MODTRAN import getExtinctionCoefficient

if __name__ == "__main__":
    # MODTRAN extinction file path
    modtran_file = "/root/TrinitySims/CHASM_NuSpacesim/src/CHASM/data/M5_ext_results_VWinter_3_2_IHAZE6_VIZ100_WSS0.profile.ext"

    # Define wavelength grid (nm) and altitude grid (m)
    wavelengths_nm = np.linspace(200, 900, 400)
    altitudes_m = np.linspace(0, 47000, 20)  # 0–47 km in meters

    # Use your helper to get α(λ,h)
    wl_mod, alt_km_mod, alpha = getExtinctionCoefficient( \
        wl_requested=wavelengths_nm,
        alt_meters_requested=altitudes_m,
        filename=modtran_file
    )

    # Plot extinction vs wavelength, with altitude as color
    colors = plt.cm.viridis(np.linspace(0, 1, len(alt_km_mod)))

    fig, ax = plt.subplots(figsize=(10, 6))
    for j, c in enumerate(colors):
        ax.plot(wl_mod, alpha[:, j], color=c, lw=1)

    # Add colorbar for altitude
    sm = plt.cm.ScalarMappable(
        cmap='viridis',
        norm=plt.Normalize(vmin=alt_km_mod.min(), vmax=alt_km_mod.max())
    )
    sm.set_array([])
    fig.colorbar(sm, ax=ax, label='Altitude (km)')

    ax.set_yscale('log')
    ax.set_xlabel('Wavelength (nm)')
    # auto ylim upper , ylim lower 10e-8
    ax.set_ylim(bottom=1e-9, top=1e-2)
    ax.set_ylabel(r'Extinction Coefficient $\alpha$ (m$^{-1}$)')
    ax.set_title('MODTRAN Extinction Coefficients (200–900 nm, 0–47 km)')
    ax.grid(True, which='both', ls='--', alpha=0.5)
    fig.tight_layout()
    plt.show()