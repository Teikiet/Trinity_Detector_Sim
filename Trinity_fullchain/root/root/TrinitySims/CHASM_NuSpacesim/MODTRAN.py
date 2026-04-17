import numpy as np
from scipy import interpolate
import matplotlib.pyplot as plt


# this is a better way of getting the optical depth from a transmittance table.
def transmittance_to_bottomup_tau(file_in, file_out=None):
    """
    Convert top-down transmittance data (from MODTRAN profile) into
    bottom-up cumulative optical depth data.

    Parameters
    ----------
    file_in : str
        Path to the input MODTRAN-style file. Format:
        wavelength[nm]  T(h=0)  T(h=1km)  ...  T(h=48km)
        where T is top-down transmittance (fraction of light transmitted from TOA to altitude h)
    file_out : str, optional
        If given, saves the resulting optical depth data table to this file.

    Returns
    -------
    wavelengths_nm : ndarray
        Wavelengths in nm
    tau_bottomup : ndarray
        2D array of shape (N_wavelengths, N_altitudes) giving cumulative optical depth
        from ground to each altitude (bottom-up).
    """

    # Load data
    data = np.loadtxt(file_in)
    MODTRAN_wavelengths = data[:, 0]
    T_topdown = data[:, 1:]
    MODTRAN_altitudes = np.arange(0, 49, 1)

    # Numerical safety: clip to avoid log(0)
    T_topdown = np.clip(T_topdown, 1e-300, 1.0)

    # Compute optical depths (top-down)
    tau_topdown = -np.log(T_topdown)

    # Bottom-up optical depth at altitude h:
    # tau_bu = tau_total - tau_topdown
    tau_total = tau_topdown[:, 0]  # optical depth from TOA to ground
    tau_bottomup = tau_total[:, np.newaxis] - tau_topdown


    # build an interpolator for bottom-up tau
    interpolated_tau = interpolate.RegularGridInterpolator(
        (MODTRAN_wavelengths, MODTRAN_altitudes),
        tau_bottomup,
        bounds_error=False,
        fill_value=None
    )

    # Optionally save output file
    if file_out:
        out_data = np.column_stack((MODTRAN_wavelengths, tau_bottomup))
        np.savetxt(file_out, out_data, fmt="%.6e",
                   header="Wavelength[nm]  Tau_bottomup(h=0..max)")
        
    return MODTRAN_wavelengths, MODTRAN_altitudes, tau_bottomup, interpolated_tau

def getTauDirectly(wl_requested, alt_meters_requested, filename):
    """Return τ(λ,h) [unitless] on a full grid, filling out-of-bounds"""
    mod_wl_nm, mod_alt_km, tau_bu, interp_tau = transmittance_steps_to_bottomup_tau(file_in=filename)
    wl_requested = np.asarray(wl_requested)
    alt_km_requested = np.asarray(alt_meters_requested) / 1000.0
    print("MODTRAN altitude range:", mod_alt_km.min(), "→", mod_alt_km.max())
    print("Input altitudes range:", alt_km_requested.min(), "→", alt_km_requested.max())

    # Identify and log out-of-bounds values
    wl_oob_low  = wl_requested[wl_requested < mod_wl_nm.min()]
    wl_oob_high = wl_requested[wl_requested > mod_wl_nm.max()]
    alt_oob_low  = alt_km_requested[alt_km_requested < mod_alt_km.min()]
    alt_oob_high = alt_km_requested[alt_km_requested > mod_alt_km.max()]

    if wl_oob_low.size or wl_oob_high.size:
        print(f"⚠️  Wavelengths out of bounds:")
        if wl_oob_low.size:
            print(f"   Below MODTRAN range ({mod_wl_nm.min()} nm): {wl_oob_low}")
        if wl_oob_high.size:
            print(f"   Above MODTRAN range ({mod_wl_nm.max()} nm): {wl_oob_high}")

    if alt_oob_low.size or alt_oob_high.size:
        print(f"⚠️  Altitudes out of bounds:")
        if alt_oob_low.size:
            print(f"   Below MODTRAN range ({mod_alt_km.min()} km): {alt_oob_low}")
        if alt_oob_high.size:
            print(f"   Above MODTRAN range ({mod_alt_km.max()} km): {alt_oob_high}")

    # delete the out-of-bounds values for interpolation
    wl_interpolate = wl_requested[
        (wl_requested >= mod_wl_nm.min()) & (wl_requested <= mod_wl_nm.max())
    ]
    alt_km_interpolate = alt_km_requested[
        (alt_km_requested >= mod_alt_km.min()) & (alt_km_requested <= mod_alt_km.max())
    ]

    # Interpolation grid for all requested λ,h
    W, H = np.meshgrid(wl_interpolate, alt_km_interpolate, indexing='ij')

    # Query interpolator
    tau_values = interp_tau(np.column_stack((W.ravel(), H.ravel())))

    # find any nan/inf results (out-of-bounds) and set to zero
    tau_values = np.nan_to_num(tau_values, nan=0.0)
    
    
    tau = tau_values.reshape(W.shape)
    print(f"Interpolated optical depth τ on grid of shape {tau.shape}")


    #we create columns of the last valid tau for out of bounds wavelengths
    if wl_oob_low.size > 0:
        last_valid_row = tau[0:1, :]  # shape (1, n_altitudes)
        repeat_rows = np.repeat(last_valid_row, wl_oob_low.size, axis=0)  # shape (n_oob_low, n_altitudes)
        tau = np.vstack((repeat_rows, tau))
    if wl_oob_high.size > 0:
        last_valid_row = tau[-1:, :]  # shape (1, n_altitudes)
        repeat_rows = np.repeat(last_valid_row, wl_oob_high.size, axis=0)  # shape (n_oob_high, n_altitudes)
        tau = np.vstack((tau, repeat_rows))
    # Do the same for altitudes, repeating the last valid row
    if alt_oob_high.size > 0:
        last_valid_col = tau[:, -1:]  # shape (n_wavelengths, 1)
        repeat_cols = np.repeat(last_valid_col, alt_oob_high.size, axis=1)  # shape (n_wavelengths, n_oob_high)
        tau = np.hstack((tau, repeat_cols))

    print(f"Final extinction coefficient α shape after adding out-of-bounds zeros: {tau.shape}")

    return wl_requested, alt_km_requested, tau # conversion factor


def transmittance_steps_to_bottomup_tau(file_in, file_out=None, layer_thickness_km=1.0):
    """
    Convert step transmittance data (fraction transmitted through each layer)
    into bottom-up cumulative optical depth data.

    Parameters
    ----------
    file_in : str
        Path to input table:
        wavelength[nm]  f(h=0→1km)  f(h=1→2km) ... f(h=47→48km)
        where each f is the fraction transmitted *through that layer*.
    file_out : str, optional
        If given, saves the resulting optical depth table.
    layer_thickness_km : float
        Layer thickness, default 1 km.

    Returns
    -------
    wavelengths_nm : ndarray
        Wavelengths in nm
    altitudes_km : ndarray
        Altitudes of layer tops (0..N_layers)
    tau_bottomup : ndarray
        Bottom-up cumulative optical depth (unitless)
    interpolated_tau : RegularGridInterpolator
        Interpolator τ(λ, h)
    """

    # --- Load data ---
    data = np.loadtxt(file_in)
    wavelengths = data[:, 0]
    f_layer = data[:, 1:]  # step transmittances
    n_layers = f_layer.shape[1]
    altitudes_km = np.arange(0, n_layers + 1, layer_thickness_km)

    # --- Numerical safety ---
    f_layer = np.clip(f_layer, 1e-300, 1.0)

    # --- Per-layer optical depth ---
    # τ_j = -ln(f_j)
    tau_layer = -np.log(f_layer)

    # --- Cumulative (bottom-up) optical depth ---
    # τ_bu(h_k) = Σ_j<k τ_j
    tau_bottomup = np.cumsum(tau_layer, axis=1)

    # Pad with τ=0 at ground (h=0)
    tau_bottomup = np.hstack((np.zeros((tau_bottomup.shape[0], 1)), tau_bottomup))

    # --- Interpolator ---
    interpolated_tau = interpolate.RegularGridInterpolator(
        (wavelengths, altitudes_km),
        tau_bottomup,
        bounds_error=False,
        fill_value=None
    )

    # --- Optional output file ---
    if file_out:
        out_data = np.column_stack((wavelengths, tau_bottomup))
        np.savetxt(file_out, out_data, fmt="%.6e",
                   header="Wavelength[nm]  Tau_bottomup(h=0..max)")

    return wavelengths, altitudes_km, tau_bottomup, interpolated_tau

def getMODTRAN(file='src/CHASM/data/M5_ext_results_VWinter_3_2_IHAZE6_VIZ100_WSS0.profile.ext'):
    """
    Load MODTRAN extinction data and return interpolator.
    Returns:
        wavelengths_nm : array (nm)
        altitudes_km   : array (km)
        coeff_log10    : 2D array log10(alpha[m^-1])
        interpolated_MODTRAN : RegularGridInterpolator
    """
    MODTRAN_data = np.loadtxt(file)
    MODTRAN_altitudes = np.arange(0, 49, 1)
    MODTRAN_wavelengths, MODTRAN_coefficients_raw = MODTRAN_data[:, 0], MODTRAN_data[:, 1:]
    # MODTRAN_coefficients = np.diff(np.log(MODTRAN_coefficients_raw), axis=1) / 100
    # print where invalid values are

    # Compute logarithmic finite differences
    diff_vals = np.diff(np.log(MODTRAN_coefficients_raw), axis=1) / 100.0

    # Replace any nonpositive values with a small positive floor
    diff_vals[diff_vals <= 0] = 1e-30

    # Now safe to take log10
    MODTRAN_coefficients = diff_vals # np.log10(diff_vals)

    invalid_mask = ~np.isfinite(MODTRAN_coefficients)
    if np.any(invalid_mask):
        print("Warning: Invalid values found in MODTRAN coefficients at indices:")
        invalid_indices = np.argwhere(invalid_mask)
        for idx in invalid_indices:
            print(f"  Wavelength value {MODTRAN_wavelengths[idx[0]]} nm, Altitude value {MODTRAN_altitudes[idx[1]]} km, Value: {MODTRAN_coefficients_raw[idx[0], idx[1]]}")
    MODTRAN_altitudes = MODTRAN_altitudes[:-1]

    interpolated_MODTRAN = interpolate.RegularGridInterpolator(
        (MODTRAN_wavelengths, MODTRAN_altitudes),
        MODTRAN_coefficients,
        bounds_error=False,
        fill_value=None
    )
    return MODTRAN_wavelengths, MODTRAN_altitudes, MODTRAN_coefficients, interpolated_MODTRAN

def getExtinctionCoefficient(wl_requested, alt_meters_requested, filename):
    """Return α(λ,h) [m^-1] on a full grid, filling out-of-bounds"""
    mod_wl_nm, mod_alt_km, _, interp_mod = getMODTRAN(file=filename)
    wl_requested = np.asarray(wl_requested)
    alt_km_requested = np.asarray(alt_meters_requested) / 1000.0

    print("MODTRAN altitude range:", mod_alt_km.min(), "→", mod_alt_km.max())
    print("Input altitudes range:", alt_km_requested.min(), "→", alt_km_requested.max())

    # Identify and log out-of-bounds values
    wl_oob_low  = wl_requested[wl_requested < mod_wl_nm.min()]
    wl_oob_high = wl_requested[wl_requested > mod_wl_nm.max()]
    alt_oob_low  = alt_km_requested[alt_km_requested < mod_alt_km.min()]
    alt_oob_high = alt_km_requested[alt_km_requested > mod_alt_km.max()]

    if wl_oob_low.size or wl_oob_high.size:
        print(f"⚠️  Wavelengths out of bounds:")
        if wl_oob_low.size:
            print(f"   Below MODTRAN range ({mod_wl_nm.min()} nm): {wl_oob_low}")
        if wl_oob_high.size:
            print(f"   Above MODTRAN range ({mod_wl_nm.max()} nm): {wl_oob_high}")

    if alt_oob_low.size or alt_oob_high.size:
        print(f"⚠️  Altitudes out of bounds:")
        if alt_oob_low.size:
            print(f"   Below MODTRAN range ({mod_alt_km.min()} km): {alt_oob_low}")
        if alt_oob_high.size:
            print(f"   Above MODTRAN range ({mod_alt_km.max()} km): {alt_oob_high}")

    # delete the out-of-bounds values for interpolation
    wl_interpolate = wl_requested[
        (wl_requested >= mod_wl_nm.min()) & (wl_requested <= mod_wl_nm.max())
    ]
    alt_km_interpolate = alt_km_requested[
        (alt_km_requested >= mod_alt_km.min()) & (alt_km_requested <= mod_alt_km.max())
    ]

    # Interpolation grid for all requested λ,h
    W, H = np.meshgrid(wl_interpolate, alt_km_interpolate, indexing='ij')

    # Query interpolator
    log10_alpha = interp_mod(np.column_stack((W.ravel(), H.ravel())))

    # Replace NaN/Inf results (out-of-bounds) with 0 extinction
    log10_alpha = np.nan_to_num(log10_alpha, nan=-np.inf)
    alpha = log10_alpha.reshape(W.shape)
    alpha[~np.isfinite(alpha)] = 0.0

    print(f"Interpolated extinction coefficient α on grid of shape {alpha.shape}")

    # # now, for wavelengths requested that were out of bounds, we create rows of zeros with height equal to the number of altitudes actually interpolated
    # if wl_oob_low.size > 0:
    #     zero_rows_low = np.zeros((wl_oob_low.size, alt_km_interpolate.size))
    #     alpha = np.vstack((zero_rows_low, alpha))
    # if wl_oob_high.size > 0:
    #     zero_rows_high = np.zeros((wl_oob_high.size, alt_km_interpolate.size))
    #     alpha = np.vstack((alpha, zero_rows_high))
    
    # # now, for altitudes requested that were out of bounds, create rows of zeroes with length equal to the current number of wavelengths in alpha
    # if alt_oob_high.size > 0:
    #     zero_cols_high = np.zeros((alpha.shape[0], alt_oob_high.size))
    #     alpha = np.hstack((alpha, zero_cols_high))

    # Similar to above, but now we create columns of the last valid alpha value for out of bounds wavelengths
    if wl_oob_low.size > 0:
        last_valid_row = alpha[0:1, :]  # shape (1, n_altitudes)
        repeat_rows = np.repeat(last_valid_row, wl_oob_low.size, axis=0)  # shape (n_oob_low, n_altitudes)
        alpha = np.vstack((repeat_rows, alpha))
    if wl_oob_high.size > 0:
        last_valid_row = alpha[-1:, :]  # shape (1, n_altitudes)
        repeat_rows = np.repeat(last_valid_row, wl_oob_high.size, axis=0)  # shape (n_oob_high, n_altitudes)
        alpha = np.vstack((alpha, repeat_rows))
    # Do the same for altitudes, repeating the last valid row
    if alt_oob_high.size > 0:
        last_valid_col = alpha[:, -1:]  # shape (n_wavelengths, 1)
        repeat_cols = np.repeat(last_valid_col, alt_oob_high.size, axis=1)  # shape (n_wavelengths, n_oob_high)
        alpha = np.hstack((alpha, repeat_cols))


    # print new shape of alpha
    print(f"Final extinction coefficient α shape after adding out-of-bounds zeros: {alpha.shape}")

    return wl_requested, alt_km_requested, alpha

    

if __name__ == "__main__":

    MODTRAN_wavelengths, MODTRAN_altitudes, MODTRAN_coefficients, interpolated_MODTRAN = getMODTRAN()

    altitude = 30 # km
    wavelengths = np.linspace(200,900,200)
    altitudes = altitude*np.ones(len(wavelengths))

    # Interpolate
    MODTRAN_values = interpolated_MODTRAN((wavelengths, altitudes))

    plt.figure(figsize=(10,6))
    plt.plot(wavelengths, MODTRAN_values, label='MODTRAN Extinction', color='blue')
    plt.xlabel('Wavelength (nm)')
    plt.ylabel(r'Extinction Coefficient $\alpha \, (\mathrm{m}^{-1})$')
    plt.yscale('log')
    plt.title(f'MODTRAN Extinction Coefficient at {altitude} km Altitude')
    plt.legend()
    plt.grid(True, which='both', ls='--', alpha=0.5)
    plt.tight_layout()
    plt.show()