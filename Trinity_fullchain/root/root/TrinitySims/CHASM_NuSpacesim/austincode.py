import numpy as np
import matplotlib.pyplot as plt
from scipy import interpolate
import constants as const
# from test_abstable_contents import MODTRAN_altitudes, MODTRAN_wavelengths

##########################################################################################################################################################

# Can get rid of everything in this section if you just want to use Elterman tables

def atmos_density(z):
    """Calculates the atmospheric density (in g/cm^3) of the atmosphere using Linsley's model for input altitude(s) (in km)

    Parameters
    ----------
        z: float
            altitude in km

    Returns
    -------
        float
            atm density in g/cm^3
    """

    # Find the appropriate atmospheric layer for the given altitude(s)
    if isinstance(z, np.ndarray):
        layers = np.argmax(np.less(z[:, None], const.atmos_layer_boundaries[None, :]), axis=1)

    else:
        layers = np.argmax(np.less(z, const.atmos_layer_boundaries))

    # Calculate the atmospheric density
    density = np.where(layers < 4, (const.atmos_b[layers] / const.atmos_c[layers]) * np.exp(-z * 1e5 / const.atmos_c[layers]), 1 / const.atmos_c[layers])

    # Above the maximum altitude in the model, the density is 0
    density[z > const.z_max] = 0

    return density

def number_density(z):
    """Calculates the particle number density (in particles/cm^3) of the atmosphere for input altitude(s) (in km)

    Parameters
    ----------
        z: float
            altitude in km

    Returns
    -------
        float
            particle number density (in particles/cm^3)
    """

    return atmos_density(z) * (const.N_A / const.mm_air) * 1e-3

def index_of_refraction(z, wavelength):
    """Calculates the index of refraction.

    Calculates the index of refraction for input altitude(s) (in km) and wavelength(s) (in nm)
    Baseline approximation of the index of refraction, as given in:
    Handbook of Chemistry and Physics, 67th Edition, R.C. Weast ed. (The Chemical Rubber Co., Cleveland, 1986) E373
    
    Wavelength correction to the index of refraction as given in K. Bernlohr, Astropart. Phys., 30:149-158, 2008.

    Parameters
    ----------
        z: float
            altitude in km
        wavelength: float
            wavelength in nm

    Returns
    -------
        float
            index of refraction
    """

    n1 = 1 + (0.283e-3) * atmos_density(z) / atmos_density(0)

    if isinstance(z, np.ndarray) and isinstance(wavelength, np.ndarray):
        n = 1 + (n1[:, None] - 1) * (0.967 + 0.033 * (400 / wavelength[None, :])**2.5)
    else:
        n = 1 + (n1 - 1) * (0.967 + 0.033 * (400 / wavelength)**2.5)

    return n

def rayleigh_extinction_coefficient(z, wavelength):
    """ Calculates the Rayleigh extinction coefficient.

    Calculates the Rayleigh extinction coefficient (in m^-1) given input altitude(s) (in km) and wavelength(s) (in nm) a, 375-384
    King correction factor (Fk) to account for depolarization. Constant results from:
    R. Thalman, K. J. Zarzana, M. A. Tolbert, and R. Volkamer., Air. J. Quant. Spectrosc. Radiat. Transf., (147):171-177, 2014.
    
    Lord Rayleigh, The London, Edinburgh, and Dublin Philosophical Magazine and Journal of Science, 47:287

    Parameters
    ----------
        z: float
            altitude in km
        wavelength: float
            wavelength in nm

    Returns
    -------
        float
            Rayleigh extinction coefficient (in m^-1)
    """

    n2 = index_of_refraction(z, wavelength)**2
    N = number_density(z) * 1e6

    Fk = 1.0608

    alpha_Rayleigh = 24 * np.pi**3 * (((n2 - 1) / (n2 + 2))**2) * Fk * (1 / ((wavelength * 1e-9)**4)) * (1 / N[:, None])

    return alpha_Rayleigh

##########################################################################################################################################################

def atmospheric_extinction_coefficient(file):
    r"""Loads in 2D array of atmospheric extinction coefficients from predefined file (over altitude and wavelength).

    Base implementation uses the results of L. Elterman. Number Tech. Rep. AFCRL-68-0153. 1968.

    Parameters
    ----------
        file: str
            file containing atmospheric extinction coefficients over altitude and wavelength

    Returns
    -------
        list
            wavelength in nm, altitudes in km, coefficients in m^-1
    """

    # Load wavelengths in nm
    wavelengths = np.loadtxt(file, delimiter=',', max_rows=1)

    # Load altitudes in km
    altitudes = np.loadtxt(file, delimiter=',', skiprows=1, max_rows=1)

    # Load in extinction coefficients in km^-1
    coefficients = np.loadtxt(file, delimiter=',', skiprows=2)

    # Remove any 0 values from array
    coefficients[coefficients == 0.0] = 1e-10

    # Convert to m^-1 and take the logarithm to smooth the data
    coefficients = np.log10(coefficients / 1000)

    return wavelengths, altitudes, coefficients
    
##########################################################################################################################################################

def getMODTRANextinction(file):
    r"""Loads in 2D array of atmospheric extinction coefficients from MODTRAN output file (over altitude and wavelength).

    Parameters
    ----------
        file: str
            MODTRAN output file containing atmospheric extinction coefficients over altitude and wavelength

    Returns
    -------
        list
            wavelength in nm, altitudes in km, coefficients in m^-1
    """

    # Load MODTRAN data
    MODTRAN_data = np.loadtxt(file)
    MODTRAN_altitudes = np.arange(0,49,1)
    MODTRAN_wavelengths, MODTRAN_coefficients = MODTRAN_data[:,0], MODTRAN_data[:,1:]
    MODTRAN_coefficients = np.log10(np.diff(np.log(MODTRAN_coefficients), axis = 1)/100)
    MODTRAN_altitudes = MODTRAN_altitudes[:-1]

    return MODTRAN_wavelengths, MODTRAN_altitudes, MODTRAN_coefficients

# Set up 2D interpolator of extinction coefficient from Elterman 1968 tabulated data
# Elterman_wavelengths, Elterman_altitudes, Elterman_coefficients = atmospheric_extinction_coefficient('Elterman_1968.txt')
# interpolated_Elterman = interpolate.RegularGridInterpolator((Elterman_wavelengths, Elterman_altitudes), Elterman_coefficients)

# Trying to read MODTRAN file (never done this before)
MODTRAN_data = np.loadtxt('src/CHASM/data/M5_ext_results_VWinter_3_2_IHAZE6_VIZ100_WSS0.profile.ext')
MODTRAN_altitudes = np.arange(0,49,1)
MODTRAN_wavelengths, MODTRAN_coefficients = MODTRAN_data[:,0], MODTRAN_data[:,1:]
MODTRAN_coefficients = np.log10(np.diff(np.log(MODTRAN_coefficients), axis = 1)/100)
MODTRAN_altitudes = MODTRAN_altitudes[:-1]

MODTRAN_wavelengths = MODTRAN_wavelengths.copy(order = 'C')
interpolated_MODTRAN = interpolate.RegularGridInterpolator((MODTRAN_wavelengths, MODTRAN_altitudes), MODTRAN_coefficients)

# Pick your altitudes and wavelength
altitude = 40
wavelengths = np.linspace(270,900,200)
altitudes = altitude*np.ones(len(wavelengths))

# Interpolate
MODTRAN = 10**interpolated_MODTRAN((wavelengths, altitudes))
#Rayleigh = 10**interpolated_Rayleigh((wavelengths, altitudes))

# Plot!
fig = plt.figure()
# plt.plot(wavelengths, Elterman, color = 'blue', label = 'Elterman 1968 (Rayleigh + Aerosol + Ozone)')
plt.plot(wavelengths, MODTRAN, color = 'blue', linestyle = '--', label = 'MODTRAN (Rayleigh + Aerosol + Ozone)')
#plt.plot(wavelengths, Rayleigh, color = 'blue', linestyle = ':', label = 'Rayleigh')
plt.yscale('log')
plt.xlabel('Wavelength (nm)')
plt.ylabel(r'$\alpha \, (\mathrm{m}^{-1})$')
plt.title('Altitude = '+str(altitude)+'km')
plt.grid(which = 'both')
plt.legend()
plt.show()