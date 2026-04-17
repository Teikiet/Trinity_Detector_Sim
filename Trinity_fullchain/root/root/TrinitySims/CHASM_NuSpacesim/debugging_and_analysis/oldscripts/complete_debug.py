#!/usr/bin/env python3
"""
Complete debug of photon object to find the actual data
"""

import eventio
import numpy as np

def complete_photon_debug(filename):
    """
    Complete debug of photon objects to find actual data
    """
    print(f"Complete photon debug for: {filename}")
    
    with eventio.EventIOFile(filename) as f:
        for obj in f:
            if isinstance(obj, eventio.iact.objects.TelescopeData):
                print("TelescopeData found:")
                
                for i, subobj in enumerate(obj):
                    if isinstance(subobj, eventio.iact.objects.Photons):
                        print(f"\n  Photons object {i}:")
                        subobj.parse()
                        
                        # Check all attributes
                        for attr in dir(subobj):
                            if not attr.startswith('_'):
                                try:
                                    val = getattr(subobj, attr)
                                    if callable(val):
                                        print(f"    {attr}: <method>")
                                    else:
                                        print(f"    {attr}: {type(val)} = {val if not hasattr(val, '__len__') or len(str(val)) < 100 else f'Length: {len(val)}'}")
                                        
                                        # If it's a numpy array, show more details
                                        if hasattr(val, 'shape'):
                                            print(f"      shape: {val.shape}")
                                        if hasattr(val, 'dtype'):
                                            print(f"      dtype: {val.dtype}")
                                            
                                except Exception as e:
                                    print(f"    {attr}: Error accessing - {e}")
                        
                        # Try to access the data through parse_data
                        try:
                            print(f"\n  Trying parse_data()...")
                            data = subobj.parse_data()
                            print(f"    parse_data() returned: {type(data)}")
                            if hasattr(data, 'shape'):
                                print(f"    shape: {data.shape}")
                            if hasattr(data, 'dtype'):  
                                print(f"    dtype: {data.dtype}")
                        except Exception as e:
                            print(f"    parse_data() failed: {e}")
                        
                        break  # Just look at first photon object
                break  # Just look at first telescope data
            
if __name__ == "__main__":
    complete_photon_debug('testfile.dat')