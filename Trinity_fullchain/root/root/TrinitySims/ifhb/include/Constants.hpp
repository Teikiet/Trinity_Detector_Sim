#ifndef F_CONST_H
#define F_CONST_H

namespace IFHB{
	/**
	 * \namespace Constant
	 * \brief A collection of constants used in the calculations needed for the input files for the different simulation programs.
	 * */
	namespace Constant{
		const double REarth = 6371.0; /**< Radius of Earth at the equator in km*/
		const double speedOfLight = 299792458.0; /**< Speed of light in m/s*/
		const double tauMass = 1776.86e-3;/**< Rest mass of tau GeV*/
		const double tauMeanLife = 290.3e-15;/**< Mean lifetime in s*/
		const double atmosphericScaleHeight = 8376.9;/**< Atmospheric Scale heigh in meters*/
		const double atmosphericDensity = 1.293e-3;/**< Atmospheric Density in g/cm^3 at sea level*/
		const double atmosphericMWeight = 28.96; /**< Molecular weight of atmopshere in g/mol*/
		const double avogadroN = 6.022e23;
		const double atmpsphericNDensity = atmosphericDensity*avogadroN/atmosphericMWeight; /**< Atmopsheric Number density N/cm^3*/
        const double atmosphericTop = 122.7;/**< Top of the atmosphere deifintion in km*/
		const double mbToCm2 = 1e-27;/**< mb to cm2*/
	}
}
#endif