# CORSIKA File Analysis Summary

## Your testfile.dat Contains:

### File Structure:
- **File Format**: CORSIKA binary format (version 7.741)
- **File Size**: 2,331,864 bytes (~2.3 MB)
- **Date**: 251105 (November 25, 2015 or similar format)
- **Run Number**: 1

### Event Data:
- **Number of Events**: 1 cosmic ray shower
- **Particle Type**: Primary cosmic ray (ID 1 = proton)
- **Energy**: 100,000 GeV (100 TeV) 
- **Observation Level**: 0 cm (sea level)

### Cherenkov Photon Data:
- **Total Photon Bunches**: 71,560
- **Total Photons**: ~295,000 (weighted)
- **Telescope ID**: 0
- **Array ID**: 0

### Photon Properties:
Each photon bunch contains:
- **Position**: x, y coordinates on ground (cm)
- **Direction**: cx, cy direction cosines  
- **Time**: Arrival time (ns)
- **Wavelength**: Cherenkov wavelength (nm)
- **Emission Height**: zem (cm above sea level)
- **Weight**: Number of photons represented by this bunch

### Why the Python corsikaio Library Failed:
The issue was a version compatibility problem with the `corsikaio` library (version 0.5.0). The library's `CorsikaCherenkovFile` class had a bug where it tried to read with a negative buffer length, causing the "read length must be non-negative or -1" error.

### Working Solution:
Use the `eventio` library instead, which can properly read the CORSIKA file structure:
- Access photon bunches through `TelescopeData` → `Photons` → `parse_data()`
- Each bunch represents multiple photons with identical properties
- The 'photons' field gives the weight (number of photons) for each bunch

### Visualization:
The working scripts show:
1. **Ground pattern**: Spatial distribution of Cherenkov light
2. **Wavelength spectrum**: UV/blue dominated (typical for Cherenkov)  
3. **Time distribution**: Photon arrival time spread
4. **Emission heights**: Where in atmosphere photons were produced
5. **Direction cosines**: Angular distribution of photons

### Related to Your Original Error:
The negative altitude error you saw earlier (z = -2887 m) suggests the atmospheric absorption code was receiving incorrectly processed altitude data, possibly due to coordinate reference differences or file format issues. The CORSIKA file itself is valid and contains proper positive emission heights.