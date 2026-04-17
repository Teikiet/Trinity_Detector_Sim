#ifndef F_SOURCE_H
#define F_SOURCE_H

#include <iostream>

#include <TMath.h>
#include <TRandom3.h>

#include "ReadConfiguration.h"
#include "Constants.hpp"
#include "Utilities.hpp"

namespace IFHB{
	class Source{
	public:
		/**
		 * Standard constructor with no parameters
		 * */
		Source();
		/**
		 * Standard constructor with input from config file
		 * */
		Source(ReadConfiguration *readConf);
		/**
		 * Standard destructor
		 * */
		~Source();
		/**
		 * Sets random engine to be the same for the simulation
		 * 
		 * @param rand A TRandom3 nrandom number generator
		 * */
		void SetRandomEngine(TRandom3 *rand);

		/**
		 * Runs the random number generator to populate where
		 * the shower axis will be placed and its 
		 * geometrical trajectory
		 * 
		 * If is diffuse source, zenith angle is set to -1 and betaEmergence and phiEmergence are also randomly generated. Spanning the semisphere at the location.
		 * For point sources, beta and phi are defined and only latitude and longitude are calculated.
		 * 
		 * @param latLimLow Lower latitude region to be simulated
		 * @param latLimHigh Highest latitude region to be simulated
		 * @param longLimLow Lowest longitude region to be simulated
		 * @param longLimHigh Highest longitude region to be simulated
		 * 
		 * @param betaLimLow Optional parameter for lowest emergence angle
		 * @param betaLimHigh Optional parameter for highest emergence angle
		 * @param phiLimLow Optional parameter for lowest phi direction of shower
		 * @param phiLimHigh Optional parameter for highest phi direction of shower
		 * 
		 * */

		void RunGeographicSimulation(double latLimLow, double latLimHigh, double longLimLow, double longLimHigh);
		/**
		 * Rund diffuse source, betaEmergence and phiEmergence are also randomly generated. Spanning the semisphere at the location.
		 *  
		 * @param betaLimLow Optional parameter for lowest emergence angle
		 * @param betaLimHigh Optional parameter for highest emergence angle
		 * @param phiLimLow Optional parameter for lowest phi direction of shower
		 * @param phiLimHigh Optional parameter for highest phi direction of shower
		 * 
		 * */

		void RunDiffuseSimulation (double betaLimLow=0, double betaLimHigh=0, double phiLimLow=0, double phiLimHigh=0);


		/**
		 * 
		 * Calculates the rotation matrix that allows converting from IFHB coordinate system to CHASM coordinate system
		 * 
		 * IFHB uses a CArtesian coordinate system at Earth's origin, and CHASM uses a Cartesian coordinate with yx>N, y->W, z->Up(locally).
		 * 
		 * Therefore a rotation is needed to convert the vector
		 * 
		 * @param theta Polar angle of source location (here wrongly named latitude)
		 * @param phi Angle from x-axis here called longitude
		 * 
		 * */

		void CalculateRotMatrix();

		/**
		 * 
		 * Gets the position of the source in cartesian coordinates using IFHB coordinate system
		 * 
		 * return pos a vector with the position (x,y,z)
		 * */

		std::vector<double> GetPosition();

		/**
		 * 
		 * Sets the position vector of the source in cartesian coordinates in the IFHB coordinate system
		 * 
		 * */
		void SetPosition();

		/**
		 * Get Zenith Angle in radians
		 * 
		 * return zenithAngle in radians
		 * */

		double GetZenithAngle();

		/**
		 * 
		 * Get azimuth angle in radians
		 * 
		 * return azimuthAngle in radians
		 * */

		double GetAzimuthAngle();

		/**
		 * Set source emergence angles
		 * 
		 * @param betaEmergence Elevation angle of emergence
		 * @param phiEmergence Phi angle w.r.t. telescope
		 * 
		 * 
		 * */

		void SetEmergenceAngles(double betaEmergence, double phiEmergence);

		/**
		 * Get Latitude of source
		 * 
		 * return a double with the source latitude in radians
		 * */

		double GetLatitude();

		/**
		 * Get Longitde of source
		 * 
		 * return a double with the source longitude in radians
		 * */

		double GetLongitude();

		/**
		 * 
		 * Get elevation emergence angle beta
		 * 
		 * return a double with angle of emergence in elevation
		 * 
		 * */
		double GetEmergenceBeta();

		/**
		 * 
		 * Get emergence azimuth
		 * 
		 * return a double with the azimuth of emergence of the shower
		 * 
		 * */

		double GetEmergenceAzimuth();

		/**
		 * 
		 * Get RotationMatrix
		 * 
		 * return a std::vector<std::vector<double>> With the rotation matrix
		 * */

		std::vector<std::vector<double>> GetRotationMatrix();
		/**
		 * Calculate vector from shower location to telescope
		 * @param telPos A std::vector<double> of size 3 with the position vector of the telescope
		 * 
		*/

		void CalculateShowerToTel(std::vector<double> telPos);
		/**
		 * 
		 * Calculate the emergence angle by rotating the directional vector of the shower for point sources
		 * 
		 * */

		void CalculateElevationAngle();

		/**
		 * 
		 * Calculate the emergence azimuth angle by rotating the directional vector of the shower for point sources
		 * 
		 * * 
		 * */

		void CalculateAzimuthAngle();

		/**
		 * 
		 * Calculates the directional vector of the shower
		 * 
		 * */

		void CalculateShowerDir();

	private:
		double betaEmergenceAngle;/**< Elevation angle for shower*/
		double phiEmergenceAngle;/**< Azimuthal angle for shower*/
		double zenithAngle;/**< Zenith angle of point source*/
		double azimuthAngle;/**< Azimuth angle of point source*/
		double latitude;/**< Latitude of shower axis crossing Earth (for CHASM) in radians*/
		double longitude;/**< Longitude of shower axis crossing the Earth (for CHASM) in radians*/

		std::vector<double> pos; /**< Cartesian position vector for shower axis. This to calculate telescope location in CHASM*/

		std::vector<std::vector<double>> rotM; /**< Rotation Matrix for source*/
		std::vector<double> shwrDir; /**< Vector describing the direction of the shower for point sources*/
		

		TRandom3 *rand;

	};
}
#endif