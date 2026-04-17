import numpy as np
import matplotlib.pyplot as plt
from MODTRAN import getExtinctionCoefficient


def generate_optical_depth_abscoefficient(modtran_file, original_abstable, out_file="abstable_MODTRAN_bottomup.npz"):
    """Generate bottom-up optical depth table from MODTRAN extinction data."""

    # get the altitude and the wavelength array from the original abstable
    original = np.load(original_abstable)
    altitudes_m = original["height"]  # in meters
    wavelengths_nm = original["wavelength"]  # in nm



    wl_mod, alt_km_mod, alpha = getExtinctionCoefficient(
        wl_requested=wavelengths_nm,
        alt_meters_requested=altitudes_m,
        filename=modtran_file
    )

    height_m = alt_km_mod * 1000.0
    l_list_nm = wl_mod

    print(len(height_m))

    # Integrate bottom-up
    dh = np.diff(height_m)
    alpha_seg = 0.5 * (alpha[:, :-1] + alpha[:, 1:])
    seg_tau = alpha_seg * dh[None, :]
    cum_from_bottom = np.cumsum(seg_tau, axis=1)
    ecoeff_new = np.concatenate([np.zeros((alpha.shape[0], 1)), cum_from_bottom], axis=1)

    # Save
    np.savez(out_file, ecoeff=ecoeff_new, wavelength=l_list_nm, height=height_m)
    print(f"Saved optical depth dataset to: {out_file}")

    # Comparison
    original = np.load(original_abstable)
    wavelengths_orig = original["wavelength"]
    altitudes_orig = original["height"]
    ecoeff_orig = original["ecoeff"]

    # Debug: Print array shapes
    print(f"ecoeff_new shape: {ecoeff_new.shape}")
    print(f"ecoeff_orig shape: {ecoeff_orig.shape}")
    print(f"l_list_nm length: {len(l_list_nm)}")
    print(f"wavelengths_orig length: {len(wavelengths_orig)}")

    # make this plot have colors on the same scale as the new one
    doPlotting(l_list_nm, height_m, ecoeff_new, wavelengths_orig, altitudes_orig, ecoeff_orig)


def doPlotting(l_list_nm, height_m, ecoeff_new, wavelengths_orig, altitudes_orig, ecoeff_orig, showplot=True, wavelength_limit_nm=(200,700), alt_limit_km=(0,10)):
    """Helper function to do plotting of optical depth comparison."""
    plt.figure(figsize=(1.3*15, 1.3*5))

    wl_mask_left, wl_mask_right = wavelength_limit_nm
    alt_mask_bottom, alt_mask_top = alt_limit_km

    # determine color limits based on the original data within plotting range
    wl_mask_orig = (wavelengths_orig >= wl_mask_left) & (wavelengths_orig <= wl_mask_right) # filter for wavelengths in plotting range
    height_mask_orig = (altitudes_orig >= alt_mask_bottom*1000) & (altitudes_orig <= alt_mask_top*1000)
    wl_mask_new = (l_list_nm >= wl_mask_left) & (l_list_nm <= wl_mask_right)
    height_mask_new = (height_m >= alt_mask_bottom*1000) & (height_m <= alt_mask_top*1000)

    # Use np.ix_ to apply boolean masks across two axes (wavelength x height)
    vmin = min(
        ecoeff_orig[np.ix_(wl_mask_orig, height_mask_orig)].min(),
        ecoeff_new[np.ix_(wl_mask_new, height_mask_new)].min()
    )
    vmax = max(
        ecoeff_orig[np.ix_(wl_mask_orig, height_mask_orig)].max(),
        ecoeff_new[np.ix_(wl_mask_new, height_mask_new)].max()
    )

    plt.subplot(1, 3, 1)
    plt.pcolormesh(l_list_nm, height_m / 1000, ecoeff_new.T, shading='auto', cmap='gray_r')
    plt.clim(vmin, vmax)
    plt.xlim(left=wl_mask_left, right=wl_mask_right)
    plt.ylim(bottom=alt_mask_bottom, top=alt_mask_top)
    plt.title("MODTRAN Optical Depth")
    plt.xlabel("Wavelength (nm)")
    plt.ylabel("Altitude (km)")
    plt.colorbar(label="Optical Depth τ")


    plt.subplot(1, 3, 2)
    plt.pcolormesh(wavelengths_orig, altitudes_orig / 1000, ecoeff_orig.T, shading='auto', cmap='gray_r')
    plt.clim(vmin, vmax)
    plt.xlim(left=wl_mask_left, right=wl_mask_right)
    plt.ylim(bottom=alt_mask_bottom, top=alt_mask_top)
    plt.title("Existing Optical Depth Table")
    plt.xlabel("Wavelength (nm)")
    plt.colorbar(label="Optical Depth τ")
    

    # color map based on the min/max of the difference only in >350nm
    diff_data = ecoeff_new.T - ecoeff_orig.T
    # Apply both wavelength (>350 nm and within plot range) and altitude plot-range masks using np.ix_
    wl_mask_diff = (l_list_nm >= 350) & (l_list_nm >= wl_mask_left) & (l_list_nm <= wl_mask_right)
    height_mask_diff = (height_m >= alt_mask_bottom * 1000) & (height_m <= alt_mask_top * 1000)

    # Guard against empty selection; fall back to full range if needed
    if np.any(wl_mask_diff) and np.any(height_mask_diff):
        diff_region = diff_data[np.ix_(height_mask_diff, wl_mask_diff)]
        vmin_diff = diff_region.min()
        vmax_diff = diff_region.max()
    else:
        vmin_diff = diff_data.min()
        vmax_diff = diff_data.max()

    # define a custom colorbar with white at zero, blue for negative, red for positive
    # Use symmetric range around zero for proper centering
    max_abs_diff = max(abs(vmin_diff), abs(vmax_diff))
    vmin_symmetric = -max_abs_diff
    vmax_symmetric = max_abs_diff
    
    cmap_diff = plt.get_cmap('coolwarm')
    norm_diff = plt.Normalize(vmin=vmin_symmetric, vmax=vmax_symmetric)
    sm = plt.cm.ScalarMappable(cmap=cmap_diff, norm=norm_diff)
    sm.set_array([])


    # difference plot
    plt.subplot(1, 3, 3)
    plt.pcolormesh(l_list_nm, height_m / 1000, diff_data, shading='auto', cmap=cmap_diff)
    plt.clim(vmin_symmetric, vmax_symmetric)
    plt.xlim(left=wl_mask_left, right=wl_mask_right)
    plt.ylim(bottom=alt_mask_bottom, top=alt_mask_top)
    plt.title("Difference (MODTRAN - Original)")
    plt.xlabel("Wavelength (nm)")
    plt.ylabel("Altitude (km)")
    plt.colorbar(label="Difference in Optical Depth $\Delta$τ")

    plt.tight_layout()

    if showplot:
        plt.show()
    
    plt.savefig("optical_depth_comparison.png", dpi=300)


# new function to generate optical depth taable using MODTRAN getTauDirectly()
def generate_optical_depth(modtran_file, original_abstable, out_file="abstable_MODTRAN_new.npz", save_data=False):
    """Generate bottom-up optical depth table from MODTRAN extinction data."""
    # get the altitude and the wavelength array from the original abstable
    original = np.load(original_abstable)
    altitudes_m = original["height"]  # in meters
    wavelengths_nm = original["wavelength"]  # in nm

    from MODTRAN import getTauDirectly

    # getTauDirectly returns (wavelengths_nm, altitudes_km, tau_array)
    _, _, tau = getTauDirectly(
        wl_requested=wavelengths_nm,
        alt_meters_requested=altitudes_m,
        filename=modtran_file
    )

    height_m = altitudes_m
    l_list_nm = wavelengths_nm

    # print(len(height_m))

    # Ensure ecoeff_new is the ndarray of optical depths
    ecoeff_new = np.asarray(tau)

    # Save
    if save_data:
        np.savez(out_file, ecoeff=ecoeff_new, wavelength=l_list_nm, height=height_m)
        print(f"Saved optical depth dataset to: {out_file}")

    # Comparison
    original = np.load(original_abstable)
    wavelengths_orig = original["wavelength"]
    altitudes_orig = original["height"]
    ecoeff_orig = original["ecoeff"]

    # Debug: Print array shapes
    print(f"ecoeff_new shape: {ecoeff_new.shape}")
    print(f"ecoeff_orig shape: {ecoeff_orig.shape}")
    print(f"l_list_nm length: {len(l_list_nm)}")
    print(f"wavelengths_orig length: {len(wavelengths_orig)}")

    # make this plot have colors on the same scale as the new one
    doPlotting(l_list_nm, height_m, ecoeff_new, wavelengths_orig, altitudes_orig, ecoeff_orig) 



if __name__ == "__main__":

    generate_optical_depth(
        modtran_file="/root/TrinitySims/CHASM_NuSpacesim/src/CHASM/data/M5_ext_results_VWinter_3_2_IHAZE6_VIZ100_WSS0.profile.ext",
        original_abstable="/root/TrinitySims/CHASM_NuSpacesim/src/CHASM/data/abstable.npz", save_data=True
    )