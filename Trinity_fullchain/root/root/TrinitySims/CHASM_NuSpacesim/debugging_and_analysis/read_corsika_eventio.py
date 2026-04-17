#!/usr/bin/env python3
"""
Working CORSIKA file reader using eventio
"""

import eventio
import numpy as np
import matplotlib.pyplot as plt

def read_corsika_with_eventio(filename):
    """
    Read CORSIKA file using eventio library
    """
    print(f"Reading CORSIKA file: {filename}")
    
    run_info = {}
    events = []
    
    with eventio.EventIOFile(filename) as f:
        for obj in f:
            if isinstance(obj, eventio.iact.objects.RunHeader):
                print(f"Run Header found:")
                print(f"  Run number: {obj.header['run_number']}")
                print(f"  Date: {obj.header['date_of_begin_run']}")
                print(f"  CORSIKA version: {obj.header['version_of_program']}")
                print(f"  Observation level: {obj.header['observation_level_cm']} cm")
                print(f"  Energy range: {obj.header['energy_range_gev']}")
                print(f"  Zenith range: {obj.header['zenith_range_rad']}")
                print(f"  Azimuth range: {obj.header['azimuth_range_rad']}")
                run_info = obj.header
                
            elif isinstance(obj, eventio.iact.objects.EventHeader):
                print(f"\nEvent Header found:")
                print(f"  Event number: {obj.header['event_number']}")
                print(f"  Particle ID: {obj.header['particle_id']}")
                print(f"  Energy: {obj.header['total_energy']} GeV")
                print(f"  Starting altitude: {obj.header['starting_altitude_cm']} cm")
                print(f"  First interaction height: {obj.header['first_interaction_height_cm']} cm")
                
            elif isinstance(obj, eventio.iact.objects.Photons):
                print(f"  Photons found: {len(obj.photons)} photons")
                if len(obj.photons) > 0:
                    events.append({
                        'photons': obj.photons,
                        'event_number': len(events) + 1
                    })
                    
                    # Print some statistics about the photons
                    photons = obj.photons
                    print(f"  Photon data shape: {photons.shape}")
                    print(f"  Columns: {photons.dtype.names}")
                    
                    if len(photons) > 0:
                        print(f"  X range: {photons['x'].min():.2f} to {photons['x'].max():.2f} cm")
                        print(f"  Y range: {photons['y'].min():.2f} to {photons['y'].max():.2f} cm")
                        print(f"  Wavelength range: {photons['wavelength'].min():.1f} to {photons['wavelength'].max():.1f} nm")
            
            elif isinstance(obj, eventio.iact.objects.Longitudinal):
                print(f"  Longitudinal development data found")
                
            else:
                print(f"  Other object: {type(obj)}")
    
    return run_info, events

def plot_photon_data(events, max_events=3):
    """
    Plot photon data from events
    """
    print(f"\nPlotting photon data for up to {max_events} events...")
    
    for i, event in enumerate(events[:max_events]):
        photons = event['photons']
        event_num = event['event_number']
        
        if len(photons) == 0:
            print(f"Event {event_num}: No photons to plot")
            continue
            
        fig, axes = plt.subplots(2, 2, figsize=(12, 10))
        fig.suptitle(f'Event {event_num} - {len(photons)} photons', fontsize=14)
        
        # 2D position plot
        ax1 = axes[0, 0]
        scatter = ax1.scatter(photons['x']/100, photons['y']/100, 
                             c=photons['wavelength'], cmap='viridis', 
                             alpha=0.6, s=1)
        ax1.set_xlabel('X position (m)')
        ax1.set_ylabel('Y position (m)')
        ax1.set_title('Photon positions (colored by wavelength)')
        plt.colorbar(scatter, ax=ax1, label='Wavelength (nm)')
        
        # Wavelength histogram
        ax2 = axes[0, 1]
        ax2.hist(photons['wavelength'], bins=50, alpha=0.7, edgecolor='black')
        ax2.set_xlabel('Wavelength (nm)')
        ax2.set_ylabel('Number of photons')
        ax2.set_title('Wavelength distribution')
        
        # Time histogram
        ax3 = axes[1, 0]
        ax3.hist(photons['time'], bins=50, alpha=0.7, edgecolor='black')
        ax3.set_xlabel('Time (ns)')
        ax3.set_ylabel('Number of photons')
        ax3.set_title('Arrival time distribution')
        
        # Direction cosines
        ax4 = axes[1, 1]
        ax4.scatter(photons['cx'], photons['cy'], alpha=0.6, s=1)
        ax4.set_xlabel('Direction cosine X')
        ax4.set_ylabel('Direction cosine Y')
        ax4.set_title('Direction cosines')
        
        plt.tight_layout()
        plt.show()
        
        # Print some statistics
        print(f"\nEvent {event_num} statistics:")
        print(f"  Total photons: {len(photons)}")
        print(f"  Wavelength: {photons['wavelength'].min():.1f} - {photons['wavelength'].max():.1f} nm")
        print(f"  Time spread: {photons['time'].min():.2f} - {photons['time'].max():.2f} ns")
        print(f"  Spatial spread: X={photons['x'].std()/100:.1f}m, Y={photons['y'].std()/100:.1f}m")

def main():
    filename = 'testfile.dat'
    
    try:
        run_info, events = read_corsika_with_eventio(filename)
        
        print(f"\nSummary:")
        print(f"  Total events found: {len(events)}")
        
        if events:
            total_photons = sum(len(event['photons']) for event in events)
            print(f"  Total photons: {total_photons}")
            
            # Plot the data
            plot_photon_data(events)
        else:
            print("  No events with photon data found")
            
    except Exception as e:
        print(f"Error reading file: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()