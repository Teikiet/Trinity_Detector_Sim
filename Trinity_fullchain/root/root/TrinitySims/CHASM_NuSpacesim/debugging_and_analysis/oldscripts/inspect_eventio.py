#!/usr/bin/env python3
"""
Working CORSIKA file reader using eventio - fixed version
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
            print(f"Processing object: {type(obj)}")
            
            if isinstance(obj, eventio.iact.objects.RunHeader):
                print(f"Run Header found:")
                print(f"  Object attributes: {dir(obj)}")
                # Try different ways to access the data
                if hasattr(obj, 'parse'):
                    obj.parse()
                if hasattr(obj, 'data'):
                    print(f"  Data type: {type(obj.data)}")
                    if hasattr(obj.data, 'keys'):
                        print(f"  Data keys: {list(obj.data.keys())}")
                        run_info = obj.data
                    else:
                        print(f"  Data: {obj.data}")
                        
            elif isinstance(obj, eventio.iact.objects.EventHeader):
                print(f"Event Header found:")
                if hasattr(obj, 'parse'):
                    obj.parse()
                if hasattr(obj, 'data'):
                    print(f"  Event data: {obj.data}")
                    
            elif isinstance(obj, eventio.iact.objects.Photons):
                print(f"Photons found:")
                if hasattr(obj, 'parse'):
                    obj.parse()
                if hasattr(obj, 'photons'):
                    photons = obj.photons
                    print(f"  Number of photons: {len(photons)}")
                    if len(photons) > 0:
                        print(f"  Photon data type: {type(photons)}")
                        print(f"  Photon data shape: {photons.shape if hasattr(photons, 'shape') else 'No shape'}")
                        if hasattr(photons, 'dtype'):
                            print(f"  Photon columns: {photons.dtype.names}")
                        events.append({
                            'photons': photons,
                            'event_number': len(events) + 1
                        })
                elif hasattr(obj, 'data'):
                    print(f"  Photon data: {obj.data}")
                    
            else:
                print(f"  Other object: {type(obj)}")
                if hasattr(obj, 'parse'):
                    obj.parse()
                if hasattr(obj, 'data'):
                    print(f"    Data: {obj.data}")
    
    return run_info, events

def simple_eventio_inspection(filename):
    """
    Simple inspection of eventio file
    """
    print(f"=== Simple EventIO inspection ===")
    
    with eventio.EventIOFile(filename) as f:
        for i, obj in enumerate(f):
            print(f"\nObject {i}: {type(obj)}")
            print(f"  Object dir: {[attr for attr in dir(obj) if not attr.startswith('_')]}")
            
            # Try to parse and examine
            try:
                if hasattr(obj, 'parse'):
                    obj.parse()
                    print("  Parsed successfully")
                
                # Check for common attributes
                for attr in ['data', 'header', 'photons', 'particles']:
                    if hasattr(obj, attr):
                        val = getattr(obj, attr)
                        print(f"  {attr}: {type(val)} - {val if not hasattr(val, '__len__') or len(val) < 10 else f'Length: {len(val)}'}")
                        
            except Exception as e:
                print(f"  Error parsing: {e}")
            
            if i > 10:  # Don't read too many objects
                break

def main():
    filename = 'testfile.dat'
    
    # First do simple inspection
    simple_eventio_inspection(filename)
    
    # Then try to read properly
    try:
        run_info, events = read_corsika_with_eventio(filename)
        
        print(f"\nSummary:")
        print(f"  Run info: {run_info}")
        print(f"  Total events found: {len(events)}")
        
        if events:
            total_photons = sum(len(event['photons']) for event in events)
            print(f"  Total photons: {total_photons}")
            
    except Exception as e:
        print(f"Error reading file: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()