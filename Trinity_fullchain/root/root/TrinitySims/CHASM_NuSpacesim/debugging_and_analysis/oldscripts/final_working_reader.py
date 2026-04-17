#!/usr/bin/env python3
"""
FINAL WORKING CORSIKA photon reader and visualizer
"""

import eventio
import numpy as np
import matplotlib.pyplot as plt

def extract_photon_bunches(filename):
    """
    Extract photon bunch data from CORSIKA file
    """
    print(f"Extracting photon data from: {filename}")
    
    events = []
    
    with eventio.EventIOFile(filename) as f:
        current_event = {'bunches': [], 'event_number': 0}
        
        for obj in f:
            if isinstance(obj, eventio.iact.objects.EventHeader):
                # Start new event
                current_event = {'bunches': [], 'event_number': len(events) + 1}
                print(f"Event {current_event['event_number']} started")
                
            elif isinstance(obj, eventio.iact.objects.TelescopeData):
                print("  Processing telescope data...")
                
                for subobj in obj:
                    if isinstance(subobj, eventio.iact.objects.Photons):
                        subobj.parse()
                        
                        print(f"    Telescope {subobj.telescope_id}:")
                        print(f"      Number of photon bunches: {subobj.n_bunches}")
                        print(f"      Total photons (weighted): {subobj.n_photons:.1f}")
                        
                        # Get the photon bunch data
                        bunches = subobj.parse_data()
                        print(f"      Bunch data shape: {bunches.shape}")
                        print(f"      Bunch data columns: {bunches.dtype.names}")
                        
                        current_event['bunches'].append(bunches)
                            
            elif isinstance(obj, eventio.iact.objects.EventEnd):
                if current_event['bunches']:
                    # Combine bunches from all telescopes
                    all_bunches = np.concatenate(current_event['bunches']) if len(current_event['bunches']) > 1 else current_event['bunches'][0]
                    current_event['bunches'] = all_bunches
                    events.append(current_event)
                    total_weight = all_bunches['photons'].sum()
                    print(f"  Event {current_event['event_number']} completed with {len(all_bunches)} bunches, {total_weight:.1f} total photons")
    
    return events

def expand_bunches_to_photons(bunches, max_photons_per_bunch=1000):
    """
    Expand photon bunches to individual photons (sampling approach for visualization)
    """
    # For visualization, we'll sample individual photons from the bunches
    # Each bunch represents multiple photons with the same properties
    
    photons_list = []
    
    for bunch in bunches:
        n_photons_in_bunch = int(bunch['photons'])
        if n_photons_in_bunch > 0:
            # Limit the number of photons per bunch for memory reasons
            n_to_sample = min(n_photons_in_bunch, max_photons_per_bunch)
            
            # Create individual photons with the bunch properties
            for _ in range(n_to_sample):
                photons_list.append([
                    bunch['x'],
                    bunch['y'], 
                    bunch['cx'],
                    bunch['cy'],
                    bunch['time'],
                    bunch['zem'],
                    bunch['wavelength']
                ])
    
    if photons_list:
        return np.array(photons_list)
    else:
        return np.array([]).reshape(0, 7)

def plot_corsika_event(event, max_photons=50000):
    """
    Plot CORSIKA event data 
    """
    bunches = event['bunches']
    event_num = event['event_number']
    
    if len(bunches) == 0:
        print(f"Event {event_num}: No bunches to plot")
        return
        
    print(f"\nPlotting Event {event_num}:")
    print(f"  {len(bunches)} photon bunches")
    print(f"  Total weighted photons: {bunches['photons'].sum():.1f}")
    
    # Expand to individual photons for visualization
    print("  Expanding bunches to individual photons for visualization...")
    photons = expand_bunches_to_photons(bunches, max_photons_per_bunch=max_photons//len(bunches) if len(bunches) > 0 else 1000)
    
    if len(photons) == 0:
        print("  No photons to plot")
        return
        
    print(f"  Plotting {len(photons)} sampled photons")
    
    # Extract photon properties
    x = photons[:, 0] / 100  # Convert cm to m
    y = photons[:, 1] / 100
    cx = photons[:, 2]
    cy = photons[:, 3]
    time = photons[:, 4]
    zem = photons[:, 5] / 100  # Emission height in m
    wavelength = photons[:, 6]
    
    # Create comprehensive plot
    fig, axes = plt.subplots(2, 3, figsize=(18, 12))
    fig.suptitle(f'Event {event_num} - CORSIKA Cherenkov Photons\n{len(bunches)} bunches, {bunches["photons"].sum():.0f} total photons', fontsize=16)
    
    # 1. Ground pattern colored by wavelength
    ax1 = axes[0, 0]
    scatter1 = ax1.scatter(x, y, c=wavelength, cmap='viridis', alpha=0.6, s=2)
    ax1.set_xlabel('X position (m)')
    ax1.set_ylabel('Y position (m)')
    ax1.set_title('Ground Pattern (colored by wavelength)')
    ax1.set_aspect('equal')
    plt.colorbar(scatter1, ax=ax1, label='Wavelength (nm)')
    
    # 2. Ground pattern colored by emission height
    ax2 = axes[0, 1]
    scatter2 = ax2.scatter(x, y, c=zem, cmap='plasma', alpha=0.6, s=2)
    ax2.set_xlabel('X position (m)')
    ax2.set_ylabel('Y position (m)')
    ax2.set_title('Ground Pattern (colored by emission height)')
    ax2.set_aspect('equal')
    plt.colorbar(scatter2, ax=ax2, label='Emission Height (m)')
    
    # 3. Ground pattern colored by arrival time
    ax3 = axes[0, 2]
    scatter3 = ax3.scatter(x, y, c=time, cmap='coolwarm', alpha=0.6, s=2)
    ax3.set_xlabel('X position (m)')
    ax3.set_ylabel('Y position (m)')
    ax3.set_title('Ground Pattern (colored by arrival time)')
    ax3.set_aspect('equal')
    plt.colorbar(scatter3, ax=ax3, label='Time (ns)')
    
    # 4. Wavelength distribution
    ax4 = axes[1, 0]
    ax4.hist(wavelength, bins=50, alpha=0.7, edgecolor='black', color='blue')
    ax4.set_xlabel('Wavelength (nm)')
    ax4.set_ylabel('Number of photons')
    ax4.set_title('Wavelength Distribution')
    ax4.grid(True, alpha=0.3)
    
    # 5. Time distribution
    ax5 = axes[1, 1]
    ax5.hist(time, bins=50, alpha=0.7, edgecolor='black', color='green')
    ax5.set_xlabel('Arrival Time (ns)')
    ax5.set_ylabel('Number of photons')
    ax5.set_title('Arrival Time Distribution')
    ax5.grid(True, alpha=0.3)
    
    # 6. Direction cosines
    ax6 = axes[1, 2]
    ax6.scatter(cx, cy, alpha=0.6, s=1, color='red')
    ax6.set_xlabel('Direction Cosine X')
    ax6.set_ylabel('Direction Cosine Y')
    ax6.set_title('Direction Cosines')
    ax6.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.show()
    
    # Print detailed statistics
    print_event_statistics(bunches, photons, event_num)

def print_event_statistics(bunches, photons, event_num):
    """
    Print detailed statistics about the event
    """
    print(f"\n=== Event {event_num} Statistics ===")
    print(f"Bunches: {len(bunches)}")
    print(f"Total photons (weighted): {bunches['photons'].sum():.1f}")
    print(f"Sampled photons for visualization: {len(photons)}")
    
    if len(photons) > 0:
        x, y = photons[:, 0], photons[:, 1]
        wavelength = photons[:, 6]
        time = photons[:, 4]
        zem = photons[:, 5]
        
        print(f"\nSpatial distribution:")
        print(f"  X: {x.min()/100:.1f} to {x.max()/100:.1f} m (std: {x.std()/100:.1f} m)")
        print(f"  Y: {y.min()/100:.1f} to {y.max()/100:.1f} m (std: {y.std()/100:.1f} m)")
        print(f"  Coverage area: ~{(x.max()-x.min())*(y.max()-y.min())/10000:.1f} m²")
        
        print(f"\nWavelength:")
        print(f"  Range: {wavelength.min():.1f} - {wavelength.max():.1f} nm")
        print(f"  Mean: {wavelength.mean():.1f} nm (std: {wavelength.std():.1f})")
        
        print(f"\nTiming:")
        print(f"  Range: {time.min():.2f} - {time.max():.2f} ns")
        print(f"  Spread: {time.std():.2f} ns")
        
        print(f"\nEmission heights:")
        print(f"  Range: {zem.min()/100:.1f} - {zem.max()/100:.1f} m")
        print(f"  Mean: {zem.mean()/100:.1f} m")

def main():
    filename = 'testfile.dat'
    
    try:
        events = extract_photon_bunches(filename)
        
        print(f"\n" + "="*70)
        print(f"CORSIKA FILE ANALYSIS COMPLETE")
        print(f"="*70)
        print(f"File: {filename}")
        print(f"Events found: {len(events)}")
        
        if events:
            total_bunches = sum(len(event['bunches']) for event in events)
            total_photons = sum(event['bunches']['photons'].sum() for event in events)
            
            print(f"Total photon bunches: {total_bunches:,}")
            print(f"Total photons (weighted): {total_photons:,.1f}")
            
            for event in events:
                bunches = event['bunches']
                print(f"  Event {event['event_number']}: {len(bunches):,} bunches, {bunches['photons'].sum():.1f} photons")
            
            # Plot all events
            for event in events:
                plot_corsika_event(event)
                
        else:
            print("No events with photon data found")
            
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()