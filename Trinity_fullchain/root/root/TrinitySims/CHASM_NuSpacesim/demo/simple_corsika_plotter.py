#!/usr/bin/env python3
"""
Simplified CORSIKA photon visualizer - work directly with bunches
"""

import eventio
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap
import argparse
# import interactive matplotlib stuff? 
import mpl_interactions

def create_spectral_colormap():
    """
    Create a colormap that approximates the visible light spectrum
    """
    # Define colors for different wavelength ranges (approximate visible spectrum)
    colors = [
        (0.0, (0.4, 0.0, 1.0)),    # Violet ~380nm
        (0.1, (0.0, 0.0, 1.0)),    # Blue ~450nm  
        (0.3, (0.0, 1.0, 1.0)),    # Cyan ~490nm
        (0.5, (0.0, 1.0, 0.0)),    # Green ~520nm
        (0.7, (1.0, 1.0, 0.0)),    # Yellow ~570nm
        (0.85, (1.0, 0.5, 0.0)),   # Orange ~600nm
        (1.0, (1.0, 0.0, 0.0))     # Red ~700nm
    ]
    return LinearSegmentedColormap.from_list('spectrum', colors)

def extract_and_plot_corsika(filename):
    """
    Extract and plot CORSIKA photon data directly from bunches
    """
    print(f"Reading CORSIKA file: {filename}")
    
    telescope_positions = {}  # Store telescope positions
    
    with eventio.EventIOFile(filename) as f:
        event_num = 0
        
        for obj in f:
            if isinstance(obj, eventio.iact.objects.TelescopeDefinition):
                print("Found telescope definition")
                obj.parse()
                # Check for telescope position information
                for attr in ['data', 'positions', 'telescopes']:
                    if hasattr(obj, attr):
                        val = getattr(obj, attr)
                        print(f"  TelescopeDefinition.{attr}: {type(val)} - {val}")
                
            elif isinstance(obj, eventio.iact.objects.EventHeader):
                event_num += 1
                print(f"\nEvent {event_num} started")
                
            elif isinstance(obj, eventio.iact.objects.ArrayOffsets):
                print("Found array offsets")
                obj.parse()
                # Check for telescope offset information
                if hasattr(obj, 'columns'):
                    print(f"  ArrayOffsets columns: {obj.columns}")
                if hasattr(obj, 'data'):
                    print(f"  ArrayOffsets data: {obj.data}")
                
            elif isinstance(obj, eventio.iact.objects.TelescopeData):
                print("Processing telescope data...")
                
                for subobj in obj:
                    if isinstance(subobj, eventio.iact.objects.Photons):
                        subobj.parse()
                        bunches = subobj.parse_data()
                        
                        print(f"  Telescope {subobj.telescope_id}:")
                        print(f"    Bunches: {len(bunches)}")
                        print(f"    Total photons: {bunches['photons'].sum():.1f}")
                        
                        # Check the photon weights distribution
                        weights = bunches['photons']
                        print(f"    Weight stats: min={weights.min():.3f}, max={weights.max():.3f}, mean={weights.mean():.3f}")
                        print(f"    Non-zero weights: {np.sum(weights > 0)}")
                        print(f"    Zero weights: {np.sum(weights == 0)}")
                        
                        # Plot directly using bunches (each bunch is a point weighted by photon count)
                        plot_bunches_directly(bunches, event_num, subobj.telescope_id, telescope_positions)

def plot_bunches_directly(bunches, event_num, telescope_id, telescope_positions=None):
    """
    Plot photon bunches directly (each bunch as a point, sized by weight)
    """
    print(f"  Plotting bunches for Event {event_num}, Telescope {telescope_id}")
    
    # Filter out zero-weight bunches
    valid_bunches = bunches[bunches['photons'] > 0]
    print(f"    Valid bunches (weight > 0): {len(valid_bunches)}")
    
    if len(valid_bunches) == 0:
        print("    No valid bunches to plot")
        return
    
    # Extract properties
    x = valid_bunches['x'] / 100  # Convert cm to m
    y = valid_bunches['y'] / 100
    weights = valid_bunches['photons']
    wavelength = valid_bunches['wavelength']
    time = valid_bunches['time']
    zem = valid_bunches['zem'] / 100  # Emission height in m
    cx = valid_bunches['cx']
    cy = valid_bunches['cy']
    
    # Create comprehensive plot
    fig, axes = plt.subplots(2, 3, figsize=(18, 12))
    fig.suptitle(f'Event {event_num} - CORSIKA Cherenkov Photon Bunches\n{len(valid_bunches):,} bunches, {weights.sum():.0f} total photons', fontsize=16)
    
    # Create custom spectral colormap
    spectral_cmap = create_spectral_colormap()
    
    # 1. Ground pattern sized by photon count, colored by wavelength
    ax1 = axes[0, 0]
    # rather than using spectral map use blue to red map for wavelength
    scatter1 = ax1.scatter(x, y, s=weights*0.1, c=wavelength, cmap='rainbow', alpha=0.9)
    # Add telescope position marker (assuming telescope is at origin for single telescope)
    ax1.scatter(0, 0, s=200, c='black', marker='s', edgecolors='white', linewidth=2, label=f'Telescope {telescope_id}', zorder=10)
    ax1.set_xlabel('X position (m)')
    ax1.set_ylabel('Y position (m)')
    ax1.set_title('Ground Pattern (size ∝ photon count, color = wavelength)')
    ax1.set_xscale('symlog')
    ax1.set_yscale('symlog')
    ax1.grid(True, alpha=0.3)
    ax1.legend()
    plt.colorbar(scatter1, ax=ax1, label='Wavelength (nm)')
    
    # 2. Ground pattern colored by emission height
    ax2 = axes[0, 1]
    scatter2 = ax2.scatter(x, y, s=weights*0.1, c=zem, cmap='plasma', alpha=0.6)
    # Add telescope position marker
    ax2.scatter(0, 0, s=200, c='black', marker='s', edgecolors='white', linewidth=2, label=f'Telescope {telescope_id}', zorder=10)
    ax2.set_xlabel('X position (m)')
    ax2.set_ylabel('Y position (m)')
    ax2.set_title('Ground Pattern (color = emission height)')
    ax2.set_xscale('symlog')
    ax2.set_yscale('symlog')
    ax2.grid(True, alpha=0.3)
    ax2.legend()
    plt.colorbar(scatter2, ax=ax2, label='Emission Height (m)')
    
    # 3. Ground pattern colored by arrival time
    ax3 = axes[0, 2]
    scatter3 = ax3.scatter(x, y, s=weights*0.1, c=time, cmap='coolwarm', alpha=0.6)
    # Add telescope position marker
    ax3.scatter(0, 0, s=200, c='black', marker='s', edgecolors='white', linewidth=2, label=f'Telescope {telescope_id}', zorder=10)
    ax3.set_xlabel('X position (m)')
    ax3.set_ylabel('Y position (m)')
    ax3.set_title('Ground Pattern (color = arrival time)')
    ax3.set_xscale('symlog')
    ax3.set_yscale('symlog')
    ax3.grid(True, alpha=0.3)
    ax3.legend()
    plt.colorbar(scatter3, ax=ax3, label='Time (ns)')
    
    # 4. Wavelength distribution (weighted histogram)
    ax4 = axes[1, 0]
    # Create histogram with color gradient matching wavelengths
    hist_vals, bin_edges = np.histogram(wavelength, bins=50, weights=weights)
    bin_centers = (bin_edges[:-1] + bin_edges[1:]) / 2
    
    # Use the spectral colormap to color each bar
    norm = plt.Normalize(vmin=wavelength.min(), vmax=wavelength.max())
    colors = spectral_cmap(norm(bin_centers))
    
    bars = ax4.bar(bin_centers, hist_vals, width=bin_edges[1]-bin_edges[0], 
                   color=colors, alpha=0.8, edgecolor='black', linewidth=0.5)
    ax4.set_xlabel('Wavelength (nm)')
    ax4.set_ylabel('Number of photons')
    ax4.set_title('Wavelength Distribution (colored by wavelength)')
    ax4.grid(True, alpha=0.3)
    
    # 5. Time distribution (weighted histogram)
    ax5 = axes[1, 1]
    ax5.hist(time, bins=50, weights=weights, alpha=0.7, edgecolor='black', color='green')
    ax5.set_xlabel('Arrival Time (ns)')
    ax5.set_ylabel('Number of photons')
    ax5.set_title('Arrival Time Distribution (weighted)')
    ax5.grid(True, alpha=0.3)
    
    # 6. Direction cosines (sized by weight)
    ax6 = axes[1, 2]
    ax6.scatter(cx, cy, s=weights*0.5, alpha=0.6, color='red')
    ax6.set_xlabel('Direction Cosine X')
    ax6.set_ylabel('Direction Cosine Y')
    ax6.set_title('Direction Cosines (size ∝ photon count)')
    ax6.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.show()
    
    # Print statistics
    print(f"\n=== Event {event_num} Statistics ===")
    print(f"Valid bunches: {len(valid_bunches):,}")
    print(f"Total photons: {weights.sum():.0f}")
    print(f"Photons per bunch: min={weights.min():.3f}, max={weights.max():.3f}, mean={weights.mean():.3f}")
    
    print(f"\nSpatial coverage:")
    print(f"  X: {x.min():.1f} to {x.max():.1f} m (range: {x.max()-x.min():.1f} m)")
    print(f"  Y: {y.min():.1f} to {y.max():.1f} m (range: {y.max()-y.min():.1f} m)")
    print(f"  Area: ~{(x.max()-x.min())*(y.max()-y.min()):.0f} m²")
    
    print(f"\nWavelength: {wavelength.min():.1f} - {wavelength.max():.1f} nm (mean: {np.average(wavelength, weights=weights):.1f})")
    print(f"Time: {time.min():.2f} - {time.max():.2f} ns (weighted mean: {np.average(time, weights=weights):.2f})")
    print(f"Emission height: {zem.min():.0f} - {zem.max():.0f} m (weighted mean: {np.average(zem, weights=weights):.0f})")

def main():
    # filename = '/root/TrinitySims/corsikaIOreader/debugging_and_analysis/testfile.dat'
    # get args from command line
    parser = argparse.ArgumentParser(description='Simple CORSIKA Photon Bunch Plotter')
    parser.add_argument('filename', type=str, help='Path to the CORSIKA output')
    args = parser.parse_args()
    filename = args.filename



    try:
        extract_and_plot_corsika(filename)
        
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()