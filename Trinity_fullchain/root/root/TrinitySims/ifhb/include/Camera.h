#ifndef F_CAMERA_H
#define F_CAMERA_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <TROOT.h>
#include <TMath.h>

#include "ReadConfiguration.h"

#include "Constants.hpp"
#include "Utilities.hpp"

namespace IFHB{
	/**
	 * \class Camera
	 * \brief Class to store camera related parameters
	 * 
	 * Class to create a Camera object that stores its dimensions and projections to ground.
	 * */

	class Camera{
	public:
		/**
		 * Empty constructor for Camera object
		 * */
		Camera();
		/**
		 * Constructor for Camera object from ReadConfiguration object
		 * @param readConfig ReadConfiguration object containing user parameters for the simulation
		 * */
		Camera(ReadConfiguration *readConfig);
		/**
		 * Camera object destructor
		 * */
		~Camera();

		/**
		 * Calculates the are on the Earth over which the camera is projected.
		 * This calculation facilitates diffuse source simulations as an area is defined on the observed sphere.
		 * 
		 * @param phi1 lower limit of the latitude FOV vertically in radians
		 * @param phi2 upper limit of the latitude FOV vertically in radians
		 * @param theta1 lower limit of the longitude FOV horizontally in radians
		 * @param theta2 upper limit of the longitude FOV horizontally in radians
		 * 
		 * @return a double containing the area projection in square km
		 * */
		double GetGroundProjection(double phi1, double phi2, double theta1, double theta2);
		/**
		 * Calculates the perpendicular area over projection covered by the camera.
		 * This calculation is used to calculating the acceptance of the telescope in the particular configuration.
		 * From the way the geometry is set up a quarter of the rectangle is calculated using this procedure, therefore, a factor of 4 is included.
		 * 
		 * @param alpha Angle with respect to the line connecting the center of the Earth and the observation point, where the lower part of the FOV intersects with the sphere
		 * @param altitude altitude at which the observatory is located.
		 * 
		 * return a double containing the area projection in square km
		 * */
		double GetPerpendicularProjection(double alpha, double altitude);

		/**
		 * Calculate the latitude projection of the FOV
		 * 
		 * @param fov should be FOV/2 for the +/-
		 * 
		 * return a double containing the latitude where the FOV intersects the Earth
		 * */

		double GetVerticalViewingAngle(double dFov, double dPointing);
		/**
		 * Calculate the longitudinal projection of the FOV
		 * 
		 * @param fov should be FOV/2 for the +/-
		 * 
		 * return a double containing the longitude where the FOV intersects the Earth
		 * */
		double GetHorizontalViewingAngle(double dFov, double dPointing);

		/**
		 * 
		 * Gets the vertical viewing limits
		 * 
		 * return a std::vector<double> with [low,up] viewing limits
		 * */

		std::vector<double> GetVerticalViewingLimits();
		/**
		 * 
		 * Gets the horizontal viewing limits
		 * 
		 * return a std::vector<double> with [low,up] viewing limits
		 * */

		std::vector<double> GetHorizontalViewingLimits();

		/**
		 * 
		 * Sets viewing limit of the camera as seen from the telescope
		 * 
		 * @param dPointing complementary angle to zenith pointing angle.
		 * 
		 * */
		void SetViewingLimit(double dPointing);

		/**
		 * 
		 * Get Camera angular height
		 * 
		 * */

		double GetAngularHeight();

		/**
		 * 
		 * Get Camera angular width
		 * 
		 * */

		double GetAngularWidth();

		/**
		 * 
		 * Caclulates Longitude angle for Camera FOV
		 * A Square projected onto a sphere from a distance h
		 * Has a Latitude span dependent on the angle of incidence
		 * Since IFHB uses polar coord system, the longitude span
		 * depends on the polar angle where the camera is projected
		 * 
		 * With the current variables used the area is undershot at the camera corners. Since the area will be enhanced anyways to extend the area simulated, there should be no problem.
		 * 
		 * @param altitude Telescope altitude in meters
		 * @param alphaVertical angle between the top of the camera and the position of the telescope
		 * @param alphaHorizontal angle between the side of the camera and the position of the telescope. 
		 * @param viewingLatLim the minimum polar angle (Latitude closest to North) where the camera points to
		 * 
		 * return The latitude limit where the camera will be projected
		 * */

		double CalculateLongitudeLimit(double altitude, double alphaVertical, double alphaHorizontal, double viewingLAtLim, double fovBoost);


	private:
		double dCameraWidth;/**< Camera detection plane physical width**/
		double dCameraHeight;/**< Camera detection plane physical height**/
		double dCameraAngularWidth;/**< Telescope FOV width**/
		double dCameraAngularHeight;/**< Telescope FOV height**/

		double dCameraVViewLimUp; /**< Upper Vertical Viewing limit of FOV radians*/
		double dCameraVViewLimLow; /**< Lower Vertical Viewing limit of FOV radians*/
		double dCameraHViewLimUp; /**< Upper Horizontal Viewing limit of FOV radians*/
		double dCameraHViewLimLow; /**< Lower Horizontal Viewing limit of FOV radians*/

		
	};
}
#endif