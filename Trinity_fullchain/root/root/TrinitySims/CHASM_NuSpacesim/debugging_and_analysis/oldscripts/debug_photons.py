#!/usr/bin/env python3
"""
Debug photon object structure
"""

import eventio

def debug_photon_objects(filename):
    """
    Debug the structure of photon objects
    """
    print(f"Debugging photon objects in: {filename}")
    
    with eventio.EventIOFile(filename) as f:
        for obj in f:
            if isinstance(obj, eventio.iact.objects.TelescopeData):
                print("TelescopeData found - examining subobjects:")
                try:
                    for i, subobj in enumerate(obj):
                        print(f"  Subobject {i}: {type(subobj).__name__}")
                        print(f"    Attributes: {[attr for attr in dir(subobj) if not attr.startswith('_')]}")
                        
                        if isinstance(subobj, eventio.iact.objects.Photons):
                            print("    This is a Photons object!")
                            try:
                                subobj.parse()
                                print("    Parsed successfully")
                                
                                # Check all possible attributes
                                for attr in ['photons', 'data', 'bunches', 'particles']:
                                    if hasattr(subobj, attr):
                                        val = getattr(subobj, attr)
                                        print(f"    {attr}: {type(val)} - {len(val) if hasattr(val, '__len__') else val}")
                                        if hasattr(val, 'dtype'):
                                            print(f"      dtype: {val.dtype}")
                                        if hasattr(val, 'shape'):
                                            print(f"      shape: {val.shape}")
                                            
                                # Try to read the raw data
                                print("    Trying to read raw photon data...")
                                subobj.seek(0)
                                raw_data = subobj.read()
                                print(f"    Raw data length: {len(raw_data)} bytes")
                                
                            except Exception as e:
                                print(f"    Error parsing photons: {e}")
                                import traceback
                                traceback.print_exc()
                        
                        if i > 5:  # Don't examine too many
                            break
                            
                except Exception as e:
                    print(f"  Error iterating subobjects: {e}")
                    import traceback
                    traceback.print_exc()

def main():
    debug_photon_objects('testfile.dat')

if __name__ == "__main__":
    main()