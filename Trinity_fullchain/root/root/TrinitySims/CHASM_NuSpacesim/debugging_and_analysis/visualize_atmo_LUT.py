import numpy as np
import matplotlib.pyplot as plt

plotJustNew = True

def read_optical_depth_table(file_path):
    """
    Reads an optical depth table from a text file.
    
    Parameters:
        file_path (str): Path to the optical depth table file.


    Returns:        
        wavelengths (np.ndarray): 1D array of wavelengths in nm.
        altitudes (np.ndarray): 1D array of altitudes in meters.
        ecoeff (np.ndarray): 2D array of extinction coefficients (optical depth).
    """

    data = np.loadtxt(file_path)
    wavelengths = data[0, 1:]  # First row, excluding first column
    altitudes = data[1:, 0]    # First column, excluding first row
    ecoeff = data[1:, 1:]      # Remaining data

    return wavelengths, altitudes, ecoeff



def plot_optical_depth_comparison(l_list_nm, height_m, ecoeff_new,
                                  wavelengths_orig, altitudes_orig, ecoeff_orig):
    

    # determine color limits based on the original data >300nm
    wl_mask_orig = wavelengths_orig >= 350
    wl_mask_new = l_list_nm >= 350

    vmin = min(ecoeff_orig[wl_mask_orig, :].min(), ecoeff_new[wl_mask_new, :].min())
    vmax = max(ecoeff_orig[wl_mask_orig, :].max(), ecoeff_new[wl_mask_new, :].max())

    if not plotJustNew:
        plt.figure(figsize=(1.3*15, 1.3*5))
        plt.subplot(1, 3, 1)
        plt.pcolormesh(l_list_nm, height_m / 1000, ecoeff_new.T, shading='auto', cmap='gray_r')
        plt.clim(vmin, vmax)
        plt.xlim(left=350)
        plt.title("MODTRAN Optical Depth")
        plt.xlabel("Wavelength (nm)")
        plt.ylabel("Altitude (km)")
        plt.colorbar(label="Optical Depth τ")


        plt.subplot(1, 3, 2)
        plt.pcolormesh(wavelengths_orig, altitudes_orig / 1000, ecoeff_orig.T, shading='auto', cmap='gray_r')
        plt.clim(vmin, vmax)
        plt.xlim(left=350)
        plt.title("Existing Optical Depth Table")
        plt.xlabel("Wavelength (nm)")
        plt.colorbar(label="Optical Depth τ")
        

        # color map based on the min/max of the difference only in >350nm
        diff_data = ecoeff_new.T - ecoeff_orig.T
        wl_mask = l_list_nm >= 350
        vmin_diff = diff_data[:, wl_mask].min()
        vmax_diff = diff_data[:, wl_mask].max()

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
        plt.xlim(left=350)
        plt.title("Difference (MODTRAN - Original)")
        plt.xlabel("Wavelength (nm)")
        plt.ylabel("Altitude (km)")
        plt.colorbar(label="Difference in Optical Depth $\Delta$τ")

        plt.tight_layout()
        plt.show()
    
    else:
        plt.figure(figsize=(1.3*7, 1.3*5))
        plt.subplot(1, 1, 1)
        plt.pcolormesh(l_list_nm, height_m / 1000, ecoeff_new.T, shading='auto', cmap='gray_r')
        plt.clim(vmin, vmax)
        plt.xlim(left=350)
        plt.title("MODTRAN Optical Depth")
        plt.xlabel("Wavelength (nm)")
        plt.ylabel("Altitude (km)")
        plt.colorbar(label="Optical Depth τ")

        plt.tight_layout()
        plt.show()


if __name__ == "__main__":
    # Load the original abstable
    original_abstable = "/root/TrinitySims/CHASM_NuSpacesim/src/CHASM/data/abstable.npz"
    original = np.load(original_abstable)
    wavelengths_orig = original["wavelength"]
    altitudes_orig = original["height"]
    ecoeff_orig = original["ecoeff"]
    
    # Load the MODTRAN-based abstable
    modtran_abstable = "/root/TrinitySims/CHASM_NuSpacesim/src/CHASM/data/abstable_MODTRAN_bottomup.npz"
    modtran = np.load(modtran_abstable)
    wavelengths_new = modtran["wavelength"]
    altitudes_new = modtran["height"]
    ecoeff_new = modtran["ecoeff"]
    
    print(f"Original abstable shape: wavelengths={len(wavelengths_orig)}, altitudes={len(altitudes_orig)}, ecoeff={ecoeff_orig.shape}")
    print(f"MODTRAN abstable shape: wavelengths={len(wavelengths_new)}, altitudes={len(altitudes_new)}, ecoeff={ecoeff_new.shape}")
    
    # Plot comparison
    plot_optical_depth_comparison(
        wavelengths_new, altitudes_new, ecoeff_new,
        wavelengths_orig, altitudes_orig, ecoeff_orig
    )