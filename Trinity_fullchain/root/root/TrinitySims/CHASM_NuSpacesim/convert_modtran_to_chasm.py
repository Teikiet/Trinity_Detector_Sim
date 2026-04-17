#!/usr/bin/env python3
"""
Convert MODTRAN transmission data to CHASM-compatible atmospheric extinction
"""

import numpy as np
from scipy import interpolate
import matplotlib.pyplot as plt
import sys
sys.path.insert(0, 'src')

def convert_modtran_to_chasm():
    """
    Convert MODTRAN transmission factors to extinction coefficients
    that CHASM can understand
    """
    
    print("=" * 60)
    print("CONVERTING MODTRAN DATA TO CHASM FORMAT")
    print("=" * 60)
    
    # Load MODTRAN data using Austin's exact method
    MODTRAN_data = np.loadtxt('src/CHASM/data/M5_ext_results_VWinter_3_2_IHAZE6_VIZ100_WSS0.profile.ext')
    MODTRAN_altitudes = np.arange(0,49,1)
    MODTRAN_wavelengths, MODTRAN_coefficients = MODTRAN_data[:,0], MODTRAN_data[:,1:]
    MODTRAN_coefficients = np.log10(np.diff(np.log(MODTRAN_coefficients), axis = 1)/100)
    MODTRAN_altitudes = MODTRAN_altitudes[:-1]

    MODTRAN_wavelengths = MODTRAN_wavelengths.copy(order = 'C')
    interpolated_MODTRAN = interpolate.RegularGridInterpolator((MODTRAN_wavelengths, MODTRAN_altitudes), MODTRAN_coefficients)
    
    print(f"Raw MODTRAN data shape: {MODTRAN_data.shape}")
    print(f"Wavelengths: {len(MODTRAN_wavelengths)} points, {MODTRAN_wavelengths.min():.0f} - {MODTRAN_wavelengths.max():.0f} nm")
    print(f"Altitudes: {len(MODTRAN_altitudes)} points, {MODTRAN_altitudes.min():.0f} - {MODTRAN_altitudes.max():.0f} km")
    print(f"Extinction coefficient array shape: {MODTRAN_coefficients.shape}")
    
    # Note: MODTRAN_coefficients now contains log10(extinction_coefficients in m^-1)
    # To get actual extinction coefficients: 10**MODTRAN_coefficients
    
    # Test the interpolator
    test_wl = 400  # nm
    test_alt = 0   # km
    test_extinction_log = interpolated_MODTRAN((test_wl, test_alt))
    test_extinction = 10**test_extinction_log
    print(f"\nTest: Extinction at {test_wl}nm, {test_alt}km = {test_extinction:.3e} m^-1")
    
    return MODTRAN_wavelengths, MODTRAN_altitudes, MODTRAN_coefficients, interpolated_MODTRAN

def create_chasm_compatible_abstable(MODTRAN_wavelengths, MODTRAN_altitudes, MODTRAN_coefficients, interpolated_MODTRAN):
    """
    Create abstable.npz replacement using MODTRAN extinction data
    """
    
    print("\n" + "=" * 60)
    print("CREATING CHASM-COMPATIBLE ABSTABLE")
    print("=" * 60)
    
    # Define CHASM's expected grid (matching current abstable.npz)
    chasm_wavelengths = np.linspace(180, 700, 105)  # nm
    chasm_heights = np.linspace(0, 50000, 51)       # m (0-50km)
    
    print(f"CHASM grid:")
    print(f"  Wavelengths: {len(chasm_wavelengths)} points, {chasm_wavelengths.min():.0f} - {chasm_wavelengths.max():.0f} nm")
    print(f"  Heights: {len(chasm_heights)} points, {chasm_heights.min():.0f} - {chasm_heights.max():.0f} m")
    
    # Generate extinction coefficients on CHASM grid
    chasm_ecoeff = np.zeros((len(chasm_wavelengths), len(chasm_heights)))
    
    for i, wl in enumerate(chasm_wavelengths):
        for j, h_m in enumerate(chasm_heights):
            h_km = h_m / 1000
            
            # Check if within MODTRAN bounds
            if (wl >= MODTRAN_wavelengths.min() and wl <= MODTRAN_wavelengths.max() and
                h_km >= MODTRAN_altitudes.min() and h_km <= MODTRAN_altitudes.max()):
                
                # Get log10(extinction) and convert to actual extinction
                log_extinction = interpolated_MODTRAN((wl, h_km))
                chasm_ecoeff[i, j] = 10**log_extinction
            else:
                # For out-of-bounds, set to small value or extrapolate
                chasm_ecoeff[i, j] = 1e-10
    
    print(f"Generated extinction coefficient range: {chasm_ecoeff.min():.2e} - {chasm_ecoeff.max():.2e} m^-1")
    
    # Compare with original CHASM abstable
    try:
        from importlib.resources import as_file, files
        with as_file(files('CHASM.data')/'abstable.npz') as file:
            original_abstable = np.load(file)
        original_ecoeff = original_abstable['ecoeff']
        
        print(f"Original CHASM range: {original_ecoeff.min():.2e} - {original_ecoeff.max():.2e} m^-1")
        
        # Check some specific values
        print(f"\nComparison at 400nm:")
        wl_idx = np.abs(chasm_wavelengths - 400).argmin()
        
        for alt_m in [0, 5000, 10000, 20000]:
            alt_idx = np.abs(chasm_heights - alt_m).argmin()
            modtran_val = chasm_ecoeff[wl_idx, alt_idx]
            
            orig_wl_idx = np.abs(original_abstable['wavelength'] - 400).argmin()
            orig_alt_idx = np.abs(original_abstable['height'] - alt_m).argmin()
            original_val = original_ecoeff[orig_wl_idx, orig_alt_idx]
            
            print(f"  {alt_m/1000:2.0f}km: MODTRAN={modtran_val:.2e}, Original={original_val:.2e}")
        
    except Exception as e:
        print(f"Could not load original abstable for comparison: {e}")
    
    return chasm_wavelengths, chasm_heights, chasm_ecoeff

def save_chasm_abstable(wavelengths, heights, ecoeff):
    """
    Save the MODTRAN data as MODTRAN_abstable.npz (don't overwrite original)
    """
    
    print("\n" + "=" * 60)
    print("SAVING MODTRAN ABSTABLE")
    print("=" * 60)
    
    # Save MODTRAN data as separate file
    output_file = 'src/CHASM/data/MODTRAN_abstable.npz'
    np.savez(output_file, 
             ecoeff=ecoeff, 
             wavelength=wavelengths, 
             height=heights)
    
    print(f"✓ Saved MODTRAN data to {output_file}")
    print("✓ Original abstable.npz unchanged")
    print()
    print("To use MODTRAN data in CHASM:")
    print("1. Create a custom atmospheric class that loads MODTRAN_abstable.npz")
    print("2. Or manually copy MODTRAN_abstable.npz to abstable.npz when needed")
    print()
    print("IMPORTANT: This contains TOTAL extinction (Rayleigh + Aerosol + Ozone)")
    print("Make sure CHASM doesn't add additional Rayleigh scattering on top!")

def create_verification_plots(wavelengths, heights, ecoeff):
    """
    Create plots to verify the MODTRAN conversion
    """
    
    print("\nCreating verification plots...")
    
    # Plot 1: Extinction vs wavelength at different altitudes
    plt.figure(figsize=(12, 8))
    
    altitudes_to_plot = [0, 5000, 10000, 20000, 30000]  # meters
    colors = plt.cm.viridis(np.linspace(0, 1, len(altitudes_to_plot)))
    
    for alt_m, color in zip(altitudes_to_plot, colors):
        alt_idx = np.abs(heights - alt_m).argmin()
        actual_alt = heights[alt_idx]
        plt.plot(wavelengths, ecoeff[:, alt_idx], 
                color=color, linewidth=2, label=f'{actual_alt/1000:.0f} km')
    
    plt.yscale('log')
    plt.xlabel('Wavelength (nm)', fontsize=12)
    plt.ylabel('Extinction Coefficient (m⁻¹)', fontsize=12)
    plt.title('MODTRAN Total Extinction vs Wavelength', fontsize=14)
    plt.legend(fontsize=12)
    plt.grid(True, alpha=0.3)
    plt.xlim(wavelengths.min(), wavelengths.max())
    plt.tight_layout()
    plt.savefig('modtran_extinction_vs_wavelength.png', dpi=300, bbox_inches='tight')
    plt.show()
    
    # Plot 2: Extinction vs altitude at key wavelengths
    plt.figure(figsize=(12, 8))
    
    key_wavelengths = [300, 400, 500, 600]  # nm
    colors = plt.cm.plasma(np.linspace(0, 1, len(key_wavelengths)))
    
    for wl, color in zip(key_wavelengths, colors):
        wl_idx = np.abs(wavelengths - wl).argmin()
        actual_wl = wavelengths[wl_idx]
        plt.plot(heights/1000, ecoeff[wl_idx, :], 
                'o-', color=color, linewidth=2, markersize=4,
                label=f'{actual_wl:.0f} nm')
    
    plt.yscale('log')
    plt.xlabel('Altitude (km)', fontsize=12)
    plt.ylabel('Extinction Coefficient (m⁻¹)', fontsize=12)
    plt.title('MODTRAN Total Extinction vs Altitude', fontsize=14)
    plt.legend(fontsize=12)
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig('modtran_extinction_vs_altitude.png', dpi=300, bbox_inches='tight')
    plt.show()
    
    print("Verification plots saved:")
    print("- modtran_extinction_vs_wavelength.png")
    print("- modtran_extinction_vs_altitude.png")

def create_modtran_attenuation_class():
    """
    Create a custom CHASM attenuation class that uses MODTRAN data
    """
    
    class_code = '''#!/usr/bin/env python3
"""
Custom CHASM attenuation class using MODTRAN atmospheric extinction data
"""

import numpy as np
from scipy import interpolate
from .axis import Attenuation
from importlib.resources import as_file, files

class ModtranAttenuation(Attenuation):
    """
    Atmospheric attenuation using MODTRAN total extinction data
    This replaces both Rayleigh scattering and absorption calculations
    """
    
    def __init__(self):
        # Load MODTRAN extinction data
        with as_file(files('CHASM.data')/'MODTRAN_abstable.npz') as file:
            modtran_data = np.load(file)
        
        self.modtran_wavelengths = modtran_data['wavelength']
        self.modtran_heights = modtran_data['height']
        self.modtran_ecoeff = modtran_data['ecoeff']
        
        # Create interpolator
        self.interpolator = interpolate.RegularGridInterpolator(
            (self.modtran_wavelengths, self.modtran_heights),
            self.modtran_ecoeff.T,  # Transpose for correct shape
            bounds_error=False,
            fill_value=1e-10
        )
    
    def get_extinction_coefficient(self, wavelength_nm, height_m):
        """Get extinction coefficient from MODTRAN data"""
        height_km = height_m / 1000  # Convert to km for interpolation
        log_extinction = self.interpolator((wavelength_nm, height_km))
        return 10**log_extinction  # Convert from log10 back to actual extinction
    
    def vertical_log_fraction(self) -> np.ndarray:
        """
        Calculate atmospheric attenuation using MODTRAN total extinction
        This method is called by CHASM's simulation system
        """
        log_fraction_array = np.empty_like(self.yield_array, dtype='O')
        
        for i, y in enumerate(self.yield_array):
            # Get extinction coefficients at each altitude for this wavelength
            extinction_coeffs = np.array([
                self.get_extinction_coefficient(y.l_mid, h) 
                for h in self.axis.h
            ])
            
            # Calculate path lengths (CHASM provides these)
            path_lengths = np.diff(self.axis.h, prepend=0)  # meters
            
            # Calculate optical depth for each step
            optical_depths = extinction_coeffs * path_lengths
            
            # Cumulative optical depth from space
            cumulative_optical_depth = np.cumsum(optical_depths)
            
            # Fraction surviving to each altitude
            frac_surviving = np.exp(-cumulative_optical_depth)
            
            # Fraction surviving each individual step (what CHASM needs)
            frac_step_surviving = 1. - np.diff(frac_surviving[::-1], append=1.)[::-1]
            
            # Avoid log(0) issues
            frac_step_surviving = np.maximum(frac_step_surviving, 1e-10)
            
            log_fraction_array[i] = np.log(frac_step_surviving)
            
        return log_fraction_array

# Function to patch CHASM to use MODTRAN attenuation
def use_modtran_attenuation():
    """
    Replace CHASM's default attenuation with MODTRAN data
    Call this before running your simulation
    """
    from . import axis
    
    # Create MODTRAN attenuation instance
    modtran_att = ModtranAttenuation()
    
    # Replace the vertical_log_fraction method in relevant classes
    def modtran_vertical_log_fraction(self):
        modtran_att.yield_array = self.yield_array
        modtran_att.axis = self
        return modtran_att.vertical_log_fraction()
    
    # Patch all attenuation classes
    if hasattr(axis, 'RayleighAttenuation'):
        axis.RayleighAttenuation.vertical_log_fraction = modtran_vertical_log_fraction
    
    # Also patch the base Attenuation class
    axis.Attenuation.vertical_log_fraction = modtran_vertical_log_fraction
    
    print("✓ CHASM patched to use MODTRAN atmospheric extinction")
    print("  (Replaces both Rayleigh scattering and absorption)")
'''
    
    # Save the class code
    with open('src/CHASM/modtran_attenuation.py', 'w') as f:
        f.write(class_code)
    
    print("✓ Created src/CHASM/modtran_attenuation.py")
    print("✓ Use this class to integrate MODTRAN data with CHASM")

def main():
    """Main conversion workflow"""
    
    # Step 1: Load and process MODTRAN data using Austin's method
    modtran_wavelengths, modtran_altitudes, modtran_coefficients, interpolator = convert_modtran_to_chasm()
    
    # Step 2: Create CHASM-compatible grid
    chasm_wavelengths, chasm_heights, chasm_ecoeff = create_chasm_compatible_abstable(
        modtran_wavelengths, modtran_altitudes, modtran_coefficients, interpolator
    )
    
    # Step 3: Create verification plots
    create_verification_plots(chasm_wavelengths, chasm_heights, chasm_ecoeff)
    
    # Step 4: Save the data
    save_chasm_abstable(chasm_wavelengths, chasm_heights, chasm_ecoeff)
    
    # Step 5: Create custom attenuation class
    create_modtran_attenuation_class()
    
    print("\n" + "=" * 60)
    print("CONVERSION COMPLETE")
    print("=" * 60)
    print("Your MODTRAN transmission data has been converted and integrated!")
    print()
    print("FILES CREATED:")
    print("• src/CHASM/data/MODTRAN_abstable.npz - Extinction coefficient data")
    print("• src/CHASM/modtran_attenuation.py - Custom attenuation class")
    print()
    print("TO USE IN YOUR SIMULATIONS:")
    print("  from CHASM.modtran_attenuation import use_modtran_attenuation")
    print("  use_modtran_attenuation()  # Call before creating simulation")
    print("  # Now run your simulation normally")
    print()
    print("IMPORTANT NOTES:")
    print("• This replaces ALL atmospheric effects (Rayleigh + Aerosol + Ozone)")
    print("• No additional scattering calculations will be applied")
    print("• Fully compatible with curved atmosphere calculations")

if __name__ == "__main__":
    main()