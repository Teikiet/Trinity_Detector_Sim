# corsikaSimulationTools

[![DOI](https://zenodo.org/badge/216995301.svg)](https://zenodo.org/badge/latestdoi/216995301)

**corsikaIOreader** is a tool to read [CORSIKA](https://www.ikp.kit.edu/corsika/index.php) eventio files and 

* convert photon information into GrISU or GrOptics format
* fill root histograms with particle and Cherenkov photon information

corsikaIOreader assigns a wavelength according to Jelly's formula to each photon and applies atmospheric extinction using different extinction routines. 
A global quantum efficiency (value in [0.,1.]) can be applied to minimize the output file size.
A ROOT tree with all available MC options (primary type, energy, height of first interaction, etc. ) and
histograms for wavelength distributions, cherenkov photon angular and spatial distribution, emmision heigth and bunchsize 
distributions can be filled (optional).

Observe that for the histogramming (**not** for converting photons to grisudet format), the CORSIKA external atmosphere number 6 (US Standard) is hardwired (for other atmospheres: change variable iatmo in corsikaIOreader.cpp).

GrIsu: 
GrOptics: http://otte.gatech.edu/care/tutorial/ and  http://www.gtlib.gatech.edu/pub/IACT/GrOptics.git/

## Dependencies

* ROOT (http://root.cern.ch/) (hopefully all ROOT versions, tested with >4.0). ROOT is only needed for the random generator and the histogramming. 

## Compilation

Type *make corsikaIOreader*

## Run options

* *-cors* read CORSIKA telescope inputfile. 

* *-grisu*  outputfile name of grisudet readable outputfile (overwrites existing files). corsikaIOreader pipes the photons to standard output for -grisu stdout

* *-histo filename.root* output file for test histograms (ROOT file) and tree with MC information for each event. 
    
* *-shorthisto filename.root* write only tree with MC information for each event to disk (smaller file size option *-histo*)

* *-abs absfile name* there are three tables with atmospherice extinction values available (see Atmosphere|here for more about atmospheric extinction):
          
* *-queff FLOAT* apply global quantum efficiency (num in intervall [0.,1.] ("R" line). Default value 1.

* *-nevents INT* read first num events from file. 

* *-narray INT* read only first num arrays

* *-tel INT* read only first num telescopes

* *-seed INT* seed seed for random generators (default 0)

* *-cfg FILENAME* telescope configuration file (only needed when telescope numbers are different in CORSIKA and the detector simulations), see [data/TLLOC.dat](data/TLLOC.dat) for an example

* *-verbose* print parameters for each event (default off)

## Examples

Convert file file1.telescope to grisu format (file file1.grisu) using atmospheric extinction values from MODTRAN4

  corsikaIOreader -cors DAT023000.telescope -grisu DAT023000.grisu -abs us76_new

Convert file file1.telescope to grisu format and pipe this directly into grisudet (needs grisudet version >= 4.1.0)

 corsikaIOreader -cors DAT023000.telescope -grisu stdout -abs us76_new | grisudet detector.pilot

Typical example for VERITAS simulations

 corsikaIOreader -queff 0.5 -cors "${corsika_file}" -seed "${run}" -grisu stdout -abs "${cfg_atm}" -cfg "${cfg_ioreader}"

## Download

 Older versions of corsikaIOreader are available through https://wiki-zeuthen.desy.de/CTA/CorsikaIOreader
 (no guarantee that this website exists in future)

## Authors

Gernot Maier and Henrike Fleischhack

Based on skeleton.c in the CORSIKA IACT package. Use of Konrad Bernloehr's IACT routines, which are part of the CORSIKA distribution.

## Licence

corsikaIOreader is licensed under GPL3 licence, see LICENSE file
