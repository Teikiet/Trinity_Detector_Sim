#ifndef F_TELESCOPE_H
#define F_TELESCOPE_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <TROOT.h>
#include <TMath.h>

#include "Constants.hpp"
#include "ReadConfiguration.h"

namespace IFHB{
/**
 * \class Telescope
 * \brief Class to store parameters related to the telescope or observatory
 * 
 * Class to create a Telescope object containing its characteristics
 * */
	class Telescope{
	public:
		Telescope();
		Telescope(ReadConfiguration *readConfig);
		~Telescope();
		/**
		 * Sets the telescope position
		 * Origin is at Center of Earth
		 * @param x x-coordinate in CHASM coords N
		 * @param y y-coordinate in CHASM coords W
		 * @param z z-coordinate in CHASM coords Up
		 * */
		void SetPosition();
		/**
		 * Sets the telescope from config file
		 * 
		 * */
		void SetPosition(double x, double y, double z);
		/**
		 * Sets the telescope position
		 * @param vector Vector of size 3 where vector is [x,y,z] (N,W,Up)
		 * */
		void SetPosition(std::vector<double> vector);
		/**
		 * Sets the telescope pointing vector
		 * */
		void SetPointingVector();
		/**
		 * Sets the telescope altitude
		 * @param h Double altitude of the balloon w.r.t. sea level
		 * */
		void SetAltitude(double h);
		/**
		 * Sets the telescope pointing angle for calculating the projection of the telescope camera.
		 * 
		 * @param angle A double with the angle from zenith where the telescope is pointed to.
		 * */
		void SetPointing(double angle);
		/**
		 * 
		 * Sets the intersection of the telescope pointing and Earth's Surface
		 * @param lat A latitude where the intersection happens
		 * @param lon A longitude where the intersection happens
		 * 
		*/
		void SetPointingCenter(double lat, double lon = 0);


		/**
		 * Calculates the location of the telescope in GrOptics
		 * Since Telescope is constructed with an offset o 2*focal length
		 * rotations displace the location of the telescope
		 * So a translation is needed
		 * 
		 * @param angle A double with the pointing angle
		 * 
		 * return displacement needed in the plane of rotation
		 * */
		double CalculateAngleOffset();

		/**
		 * Get the position vector of the telescope in IFHB coordinate system
		 *
		 * return pos a vector with the position (x,y,z) 
		 * */
		std::vector<double> GetPosition();
		/**
		 * Get the pointing vector of the optical axis
		 * 
		 * return vOA a vector with directional cosines of the pointing
		 * */
		std::vector<double> GetTelescopePointing();
		/**
		 * 
		 * Get Pointing angle
		 * 
		 * return dPointingAngle in degrees
		 * */

		double GetPointingAngle();

		/**
		 * 
		 * Get altitude of telescope
		 * 
		 * return dAltitude
		 * */
		double GetAltitude();

		/**
		 * 
		 * Get Latitude of telescope
		 * 
		 * return latitude in radians
		 * */

		double GetLatitude();

		/**
		 * 
		 * Get Longitude of telescope
		 * 
		 * return longitude in radians
		 * */
		double GetLongitude();

		/**
		 * 
		 * Calculates corresponding latitude to maintain
		 * a parallel trajectory for point sources
		 * 
		 * @param lat The latitude of the source in th ground
		 * 
		 * */

		double GetCorrespondingLat(double lat);

		/**
		 * Get the location of the intersection of the pointing and Earth's surface
		*/
		std::vector<double> GetPointingCenter();
	private:
		std::vector<double> pos;/**< Position vector of telescope**/
		std::vector<double> vPointing;/**< Vector in the direction of telescope pointing*/;
		std::vector<double> vPointingCenter;/**< Vector where the center of the camera crosses the Earth*/;
		double dAltitude;/**< Altitude of telescope location**/
		double dPointingAngle;/**< Angle from zenith for telescope pointing**/
		double dLat;/**< Telescope latitude*/
		double dLong;/**< Telescope longitude*/
		double dCollectionArea;/**< Telescope collection area**/
		double dCollectionSphereRadius;/**< IACT fiducial sphere radius**/
		double dFocalLength;/** Focal length of telescope optics*/
	};
};
#endif