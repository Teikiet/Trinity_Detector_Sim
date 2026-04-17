#!/usr/bin/env python3
"""
Final working CORSIKA file reader using eventio
"""

import eventio
import numpy as np
import matplotlib.pyplot as plt

def read_corsika_eventio_final(filename):
    """
    Read CORSIKA file using eventio library - final working version
    """
    print(f"Reading CORSIKA file: {filename}")
    
    run_info = {}
    events = []
    current_event = {}
    
    with eventio.EventIOFile(filename) as f:
        for obj in f:
            print(f"Processing: {type(obj).__name__}")
            
            # Handle different object types
            if isinstance(obj, eventio.iact.objects.RunHeader):
                # Don't try to parse RunHeader directly, just note it
                print("  Run header found")
                
            elif isinstance(obj, eventio.iact.objects.EventHeader):
                print("  Event header found")
                # Start a new event
                current_event = {'photons': [], 'event_number': len(events) + 1}
                
            elif isinstance(obj, eventio.iact.objects.TelescopeData):
                print("  Telescope data found (contains subobjects)")
                # This contains subobjects - iterate through them
                try:
                    for subobj in obj:
                        print(f"    Subobject: {type(subobj).__name__}")
                        if isinstance(subobj, eventio.iact.objects.Photons):
                            subobj.parse()
                            photons = subobj.photons
                            print(f"    Found {len(photons)} photons")
                            if len(photons) > 0:
                                current_event['photons'].extend(photons)
                except Exception as e:
                    print(f"    Error reading telescope subobjects: {e}")
                    
            elif isinstance(obj, eventio.iact.objects.EventEnd):
                print("  Event end")
                if current_event and len(current_event['photons']) > 0:
                    # Convert photon list to numpy array
                    current_event['photons'] = np.array(current_event['photons'])
                    events.append(current_event)
                    print(f"    Event {current_event['event_number']} added with {len(current_event['photons'])} photons")
                current_event = {}
                
            elif isinstance(obj, eventio.iact.objects.Photons):
                # Direct photons (shouldn't happen in telescope mode but just in case)
                obj.parse()
                photons = obj.photons
                print(f"  Direct photons: {len(photons)}")
                if current_event:
                    current_event['photons'].extend(photons)
                    
            else:
                # Try to parse other objects
                try:
                    obj.parse()
                    print(f"  {type(obj).__name__} parsed successfully")
                except Exception as e:
                    print(f"  {type(obj).__name__} parse failed: {e}")
    
    return run_info, events

def plot_photon_events(events, max_events=3):
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
            
        print(f"Event {event_num}: Plotting {len(photons)} photons")
        print(f"  Photon data type: {type(photons)}")
        
        # Check if photons is a structured array
        if hasattr(photons, 'dtype') and photons.dtype.names:
            print(f"  Photon columns: {photons.dtype.names}")
            
            fig, axes = plt.subplots(2, 2, figsize=(12, 10))
            fig.suptitle(f'Event {event_num} - {len(photons)} photons', fontsize=14)
            
            # 2D position plot
            ax1 = axes[0, 0]
            if 'x' in photons.dtype.names and 'y' in photons.dtype.names:
                x_data = photons['x'] / 100  # Convert cm to m
                y_data = photons['y'] / 100
                
                if 'wavelength' in photons.dtype.names:
                    scatter = ax1.scatter(x_data, y_data, c=photons['wavelength'], 
                                        cmap='viridis', alpha=0.6, s=1)
                    plt.colorbar(scatter, ax=ax1, label='Wavelength (nm)')
                else:
                    ax1.scatter(x_data, y_data, alpha=0.6, s=1)
                    
                ax1.set_xlabel('X position (m)')
                ax1.set_ylabel('Y position (m)')
                ax1.set_title('Photon positions')
                
            # Wavelength histogram
            ax2 = axes[0, 1]
            if 'wavelength' in photons.dtype.names:
                ax2.hist(photons['wavelength'], bins=50, alpha=0.7, edgecolor='black')
                ax2.set_xlabel('Wavelength (nm)')
                ax2.set_ylabel('Number of photons')
                ax2.set_title('Wavelength distribution')
            
            # Time histogram
            ax3 = axes[1, 0]
            if 'time' in photons.dtype.names:
                ax3.hist(photons['time'], bins=50, alpha=0.7, edgecolor='black')
                ax3.set_xlabel('Time (ns)')
                ax3.set_ylabel('Number of photons')
                ax3.set_title('Arrival time distribution')
            
            # Direction cosines
            ax4 = axes[1, 1]
            if 'cx' in photons.dtype.names and 'cy' in photons.dtype.names:
                ax4.scatter(photons['cx'], photons['cy'], alpha=0.6, s=1)
                ax4.set_xlabel('Direction cosine X')
                ax4.set_ylabel('Direction cosine Y')
                ax4.set_title('Direction cosines')
            
            plt.tight_layout()
            plt.show()
            
            # Print statistics
            print_photon_statistics(photons, event_num)
        else:
            print(f"  Photons data structure not recognized: {photons}")

def print_photon_statistics(photons, event_num):
    """
    Print statistics about photon data
    """
    print(f"\nEvent {event_num} statistics:")
    print(f"  Total photons: {len(photons)}")
    
    if 'wavelength' in photons.dtype.names:
        wl = photons['wavelength']
        print(f"  Wavelength: {wl.min():.1f} - {wl.max():.1f} nm (mean: {wl.mean():.1f})")
    
    if 'time' in photons.dtype.names:
        t = photons['time']
        print(f"  Time: {t.min():.2f} - {t.max():.2f} ns (spread: {t.std():.2f})")
    
    if 'x' in photons.dtype.names and 'y' in photons.dtype.names:
        x, y = photons['x'], photons['y']
        print(f"  Position: X std={x.std()/100:.1f}m, Y std={y.std()/100:.1f}m")
        print(f"  Coverage: X={x.min()/100:.1f} to {x.max()/100:.1f}m, Y={y.min()/100:.1f} to {y.max()/100:.1f}m")

def main():
    filename = 'testfile.dat'
    
    try:
        run_info, events = read_corsika_eventio_final(filename)
        
        print(f"\n" + "="*50)
        print(f"SUMMARY:")
        print(f"  File: {filename}")
        print(f"  Events with photons: {len(events)}")
        
        if events:
            total_photons = sum(len(event['photons']) for event in events)
            print(f"  Total photons: {total_photons:,}")
            
            # Show photon distribution per event
            for event in events:
                print(f"    Event {event['event_number']}: {len(event['photons']):,} photons")
            
            # Plot the data
            plot_photon_events(events)
        else:
            print("  No events with photon data found")
            
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()