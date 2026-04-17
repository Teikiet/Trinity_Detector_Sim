from corsikaio import CorsikaCherenkovFile
import matplotlib.pyplot as plt
import os


print(f"File exists: {os.path.exists('testfile.dat')}")
print(f"File size: {os.path.getsize('testfile.dat')} bytes")

# Try different approaches to read the file
try:
    with CorsikaCherenkovFile('testfile.dat') as f:
        print(f"Successfully opened file")
        print(f"Run header keys: {list(f.run_header.keys())}")
        print(f"Run number: {f.run_header['run_number']}")
        print(f"Version: {f.version}")
        
        # Try to iterate through events
        event_count = 0
        for e in f:
            print(f"Event {event_count}: Total energy = {e.header['total_energy']}")
            print(f"Number of photons: {len(e.photons)}")
            if len(e.photons) > 0:
                print(f"Photon data columns: {e.photons.dtype.names}")
                # Only plot first few events to avoid overwhelming output
                if event_count < 3:
                    plt.figure(figsize=(8, 6))
                    plt.scatter(e.photons['x'], e.photons['y'], alpha=0.5, s=1)
                    plt.xlabel('X position')
                    plt.ylabel('Y position')
                    plt.title(f'Event {event_count} - Photon positions')
                    plt.show()
            event_count += 1
            if event_count >= 5:  # Limit to first 5 events
                break
                
except Exception as e:
    print(f"Error reading with corsikaio: {e}")
    print("\nTrying alternative approach...")
    
    # Alternative: try to read file structure manually
    with open('testfile.dat', 'rb') as f:
        # Read first few bytes to examine structure
        header_bytes = f.read(100)
        print(f"First 100 bytes (hex): {header_bytes.hex()}")
        print(f"First 100 bytes (as much as possible as ASCII): {header_bytes}")
        
        # Look for CORSIKA markers
        f.seek(0)
        data = f.read(1000)
        if b'RUNH' in data:
            runh_pos = data.find(b'RUNH')
            print(f"Found RUNH at position: {runh_pos}")
        if b'EVTH' in data:
            evth_pos = data.find(b'EVTH')
            print(f"Found EVTH at position: {evth_pos}")