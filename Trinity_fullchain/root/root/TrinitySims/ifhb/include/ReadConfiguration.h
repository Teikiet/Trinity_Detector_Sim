#ifndef F_READCONFIG_H
#define F_READCONFIG_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <TROOT.h>

/**
 * \class ReadCofiguration
 * \brief Class to read and store all the configuration file parameters and settings that drive the simulations.
 * 
 * Class reads in a configuration file with multiple parameters to steer the simulations done. It creates an input file for each of the different simualtion packages done.
 * 
 * It creates an area based on the projection of the camera onto a sphere. From there, random locations within this area are obtained and a tau is sampled based on its exit probability.
 * 
 * For point sources the exit direction of the tau matches the location of the point source, (i.e. the tau emerges parallel to the vector pointing to the source). For diffuse sources, a random angle is obtained and matched with its corresponding azimuth angle.
 * 
 * All of the coordinates are with respect to the telescope location. The telescope is located at (0,0,H), where H is the height of the observatory.
 * 
 * The output files to steer the simulations will not follow this same coordinate system.
 * 
 * CHASM input files will have X->North Y->West Z-Up w.r.t. to the point the shower axis crosses the Earth.
 * 
 * GrISUDet input files will have X->East Y->South Z->Down. 
 * 
 * */
namespace IFHB{
	class ReadConfiguration{
	public:
		/**
		 * Constructor that uses the configuration file for reading in
		 * @param cfgFile A string witht the name of the config file
		 * */
		ReadConfiguration(std::string cfgFile);
		/**
		 * Destructor
		 * */
		~ReadConfiguration();

		/**
		 * Reads command line arguments
		 * 
		 * @param argc Number of arguments from command line
		 * @param argv Input from command line
		 * 
		 * */

		void ReadCommandLine(int argc, char** argv);

		float fTelAltitude;/**< float with telescope height meters**/
		float fTelLatitude;/**< float with telescope latitude degrees**/
		float fTelLongitude;/**< float with telescope longitude degrees**/
		float fTelPointingAngle;/**< float with the pointing angle from zenith in degrees**/
		float fTelFocalLength;/**<float with the focal length of the telescope**/
		float fTelCollectionArea;/**<float with the light collection effective area**/
		
		float fCameraWidth;/**< Width of camera detection area in mm**/
		float fCameraHeight;/**< float with the height of the camera area in mm**/
		float fCameraAngularWidth;/**< float with the angular width of the camera**/
		float fCameraAngularHeight;/**< float with the angular height of the camera**/


		int iNShowers;/**< int with number of showers to simulate**/
		double dNeutrinoEnergy;/**< double with energy of the neutrino simulated (if set to 0 a -2 spectrum is assumed [not yet implemented])*/
		bool bIsNuEnergySpectrum;/**< Flag to sample from energy spectrum instead of a fixed energy*/

		double dCosmicRayEnergy;/**< double with the energy of the cosmic ray simulated*/
		double dSpectralIndex;/**< double with spectral index for energy spectrum*/

		bool bIsPointSource;/**< bool whether it is a point source or not**/
		bool bIsBelowLimb;/**< bool whether to generate below the limb or above the limb events*/
		float fPointSourceZe;/**< Zenith angle where the point source is located w.r.t. telescope location**/
		float fPointSourceAz;/**< Azimuth angle from North where the point source is located w.r.t. telescope location**/

		float fTelCollectionSphere;/**< Collection sphere for IACT simulation
		**/

		double dAngleIncrease;/**< Amount by which to increase the FOV for the area simulation**/
		double dAngleShowerIncrease; /**< Amount by which to increase the Emergence angles (theta and phi)*/

		double dBSMMeanLifeMult;/**< A factor to modify the tau-mean life to allow BSM Decay*/

        double dESampleStart;/**< If sampling neutrino spectrum, minimum energy of spectrum*/
        double dESampleEnd;/**< If sampling neutrino spectrum, max energy of spectrum*/

		std::string sFileNameEmergenceAngles;/**< File with emergence angles related to data files*/
		std::string sFileNameEnergyFractions;/**< File with energy fractions related to data files*/
		std::string sFileNameEnergyDistribution;/**< File with energy distributions for different neutrino energies*/

		std::string sFileNameProbExit;/**< File with exit probability of a tau to exit for a given ENu*/
		std::string sFileNameNuEnergy;/**< File with neutrino energies related to data files*/
		std::string sFileNameShowerNMax;/**< Digitized Shower NMax vs E*/
		std::string sFileNameShowerXMax;/**< Digitized Shower XMax vs E*/
		std::string sFileNameShowerWidth;/**< Digitized Shower W vs E*/

		std::string sFileNameViewingAngle;/**< Viewing angle vs Alpha Angle*/
		std::string sFileNameViewingAngleInverse;/**< Alpha angle vs Viewing angle*/
		std::string sFileNameXSection;/**< Cross-section of particle E vs Cross-section*/

		bool bIsDraw; /**< Option to enable drawing the debug plots*/

	protected:
		void ReadLine(std::string iline, std::ifstream *inFileStream);
		std::string BoolToStringAction(Bool_t stage_status);
	};
};
#endif