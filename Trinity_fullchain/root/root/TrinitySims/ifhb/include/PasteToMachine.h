#ifndef F_PTM_H
#define F_PTM_H

#include <iostream>
#include <vector>

#include <TMath.h>

#include "Utilities.hpp"

namespace IFHB{
	/**
	 * \class PasteToMachine
	 * \brief Copy and paste values for simulation into other variables for storage in monitoring files and simulation files
	 * 
	 * The output of IFHB needs to be stored in 2 different files.
	 * One is a root file that will contain the configuration for the particular run of IFHB.
	 * The other is the input file for CHASM/GrOptics
	 * 
	 * A copy of the pertinent variables will be stored and placed in this class
	 * */
	class PTM{
		public:

			PTM();
			~PTM();
			
			//Variables that have one value per simulation run
			double dNeutrinoEnergy;/**< Energy of the incoming neutrino*/
			double dAreaSimulated;/**< Total area simulated on the surface*/
			double dPAreaSimulated;/**< Perpendicular area simulated*/
			double dPointingAngle;/**< Zenith angle where the telescope is pointed*/
			double dCollectionSphereR;/**< Collection sphere for the IACT in CHASM*/
			double dHorizonAngle;/**< Angle from zenith where the horizon is located*/
			double dOffsetTilt;/**< Offset correction due to tilt in GrOptics in m*/
			double dOffsetRot;/**< Offset correction due to Azimuthal rotationin GrOptics*/
			std::vector<double> vTelescopePosition;/**< Telescope position in polar coordinates*/

			std::vector<double> vViewingPolar;/**< Polar Angles for simulation area*/
			std::vector<double> vViewingLong;/**< Longitude Angles for simulation area*/

			bool bIsPointSource;/**< Whether the simulation corresponds to a point source**/
			double dSourceAz;/**< Azimuth angle of the source */
			double dSourceZe;/**< Zenith angle of the source */

			//Variables that change from shower to shower
			double dEmergenceBeta;/**< Emergence angle of the shower*/
			double dEmergenceAz;/**< Emergence azimuth of the shower*/
			double dEShower;/**< Energy deposited into the shower*/
			double dETau;/**< Energy of the Tau particle */
			double dXMax;/**< GH XMax parameter*/
			double dNMax;/**< GH NMax parameter*/
			double dX0;/**< GH X0 parameter*/
			double dLambda;/**< GH lambda parameter*/
			double dPExit;/**< Probability of tau to exit*/

			std::vector<double> vShowerLocation;/**< Location of the shower axis crossing the Earth in polar coordinates*/
			std::vector<double> vTelescopePointing;/**< Vector describing the telescope's optical axis*/
			std::vector<double> vTelescopePointingCenter;/**< Vector describing the intersection of telescope's optical axis and Earth*/
			std::vector<std::vector<double>> vRotMatrix; /**< Rotation matrix for the particular source*/


			/**
			 * 
			 * Gets the output for CHASM and GrOptics to run
			 * 
			 * return a string that is parsed by the script running CHASM and GrOptics
			 * 
			 * */

			std::string GetOutputString();

			/**
			 * 
			 * Gets Output for EASCherSim. 
			 * 
			 * @return a string that is parsed for inputs to easchersim
			 * 
			*/
			std::string GetEASCSString();

			/**
			 * Get Polar angle as String
			 * 
			 * return a string with the polar angle for CHASM
			 * */

			std::string GetStrPolarAngle();
			/**
			 * Get Azimuth angle as string for CHASM
			 * 
			 * return a string with the azimuth angle
			 * */
			std::string GetStrAzimuthAngle();


			/**
			 * Get position of telescope in CHASM coordinates
			 * 
			 * return a string with the position vector in the format X Y Z
			 * */

			std::string GetStrCHASMTelPos();


			/**
			 * Get collection sphere radius for CHASM
			 * 
			 * return a string with the radius of the collection sphere
			 * 
			 * */

			std::string GetStrCollectionSphereR();

			/**
			 * Get X Max
			 * 
			 * return a string with GH XMax
			 * 
			 * */

			std::string GetStrXMax();

			/**
			 * Get N Max
			 * 
			 * return a string with GH N Max
			 * */

			std::string GetStrNMax();

			/**
			 * Get X0
			 * 
			 * return a string with GH X0
			 * */

			std::string GetStrX0();

			/**
			 * Get Lambda
			 * 
			 * return a string with GH Lambda
			 * */

			std::string GetStrLambda();

			/**
			 * 
			 * Get Shower Energy
			 * 
			 * return a string with the energy of the particular shower 
			 * */

			std::string GetStrEShower();

			/**
			 * 
			 * Get Telescope position for GrOptics
			 * 
			 * return telescope position with rotation correction for GrOptics
			 * */

			std::string GetStrGrOpTelPos();


			/**
			 * 
			 * Get Telescope rotation for GrOptics
			 * 
			 * return a string with the rotation for GrOptics
			 * 
			 * */


			std::string GetStrGrOpRot();




	};
}
#endif