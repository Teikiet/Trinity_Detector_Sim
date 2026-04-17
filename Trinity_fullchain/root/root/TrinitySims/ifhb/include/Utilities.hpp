#ifndef F_UTILS_H
#define F_UTILS_H

#include <iostream>
#include <fstream>
#include <TMath.h>

#include "Constants.hpp"

namespace IFHB{
	/** 
	 * \namespace Utilities
	 * \brief Utility algorithms used for caluclations
	 * 
	 * Algorithms used in various calculations.  
	 * 
	 * */
	namespace Utilities{

		/**
		 * Linear interpolation between 2 points evaluated at x
		 * @param x1 double containing the x-coordinate of the first point
		 * @param y1 double containing the y-coordinate of the first point
		 * @param x2 double containing the x-coordinate of the second point
		 * @param y2 double containing the y-coordinate of the second point
		 * @param x double containing the x-coordinate where to evaluate the interpolation
		 * 
		 * return a double with the value of the interpolation at x
		 * */
		inline double LinearInterpolate(double x1, double y1, double x2, double y2,double x ){
			double slope = (y2-y1)/(x2-x1);
			double intercept = y1 - slope*x1;
			double interp = slope*x+intercept;

			return interp;
		}

		/**
		 * Distance described by a line originating from the balloon's position and intersecting Earth's surface. The line segment considers an angle beta (which relates to alpha by the LUT) equal to the viewing angle of the balloon.
		 * 
		 * @param alpha angle between the line from the Center of the Earth to the balloon and where the pointing direction intersects the Earth in radians.
		 * @param altitude Altitude of the balloon above Earth's surface w.r.t. sea level in km
		 * 
		 * return a double with the distance from the surface to the balloon following the pointing angle in km
		 * */
		inline double DistanceToSurface(double alpha, double altitude){
			double sqDistance = TMath::Power(IFHB::Constant::REarth,2.0)+TMath::Power(IFHB::Constant::REarth+altitude,2.0) - 2.0*IFHB::Constant::REarth*(IFHB::Constant::REarth+altitude)*TMath::Cos(alpha);

			return TMath::Sqrt(sqDistance);
		}

		/**
		 * Calculate the maximum alpha for a telescope at the specific altitude (location of the limb). Alpha is defined as the angle between the line originating from the center of the Earth to the balloon and the location of the limb
		 * 
		 * @param altitude altitude of the telescope
		 * 
		 * return a double with the angle to the horizon in radians
		 * */

		inline double HorizonAngle(double altitude){
			double sinAMax = TMath::Sqrt(TMath::Power(IFHB::Constant::REarth+altitude,2.0)-TMath::Power(IFHB::Constant::REarth,2.0))/(IFHB::Constant::REarth+altitude);

			double alphaMax = TMath::ASin(sinAMax);

			return alphaMax;
		}

		/**
		 * Integrate the over the area of the spherical section over which the camera is projected
		 * 
		 * @param phi1 lower limit of integration the phi direction in radians
		 * @param phi2 upper limit of integration in phi direction in radians
		 * @param theta1 lower limit of integration in the theta direction in radians
		 * @param theta2 upper limit of integratin in the theta direction in radians
		 * 
		 * return solid angle of the camera projection on the sphere in steradians.
		 * */
		inline double SphericalSolidAngle(double theta1, double theta2, double phi1, double phi2){
			double solidAngle = (TMath::Cos(theta1)-TMath::Cos(theta2))*(phi2-phi1);
			return solidAngle;
		}

		/**
		 * Law of cosines calculator
		 * 
		 * Calculates side c, when a, b and the angle between them are known
		 * 
		 * @param a one side of the triangle
		 * @param b adjacent side to a
		 * @param alpha, angle between a and b
		 * 
		 * return side c of described triangle
		 * */

		inline double CosineLaw(double a, double b, double alpha){

			return TMath::Sqrt(TMath::Power(a,2.0)+TMath::Power(b,2.0)-2.0*a*b*TMath::Cos(alpha));

		}

		/**
		 * Integrates over the path of the particle to find the column depth
		 * An exponential atmosphere is assumed.
		 * 
		 * @param xlim Limit of integration in meters
		 * @param beta Angle of emergence
		 * @param bins Optional parameter for number of samples in Riemann Sum
		 * 
		 * return column depth in g/cm^2
		 * */
		inline double ExpAtmColumnDepth(double xlim, double beta,  int bins = 100, double hInsert = 0){
			double step = xlim/bins;
			double height = 0;
			double columnDepth = 0;

			for(int i = 0; i<bins; i++){
				height = Utilities::CosineLaw(i*step*1e-3,IFHB::Constant::REarth+hInsert,beta+TMath::Pi()/2.0)-IFHB::Constant::REarth;
				columnDepth+=TMath::Exp(-1.0*height*1.0e5/(IFHB::Constant::atmosphericScaleHeight*1.0e5))*step*1.0e2; //convert km to cm

			}

			return columnDepth*IFHB::Constant::atmosphericDensity;
		}

		/**
		 * 
		 * Check if the emergence angles to be simulated are covered in the available data sets
		 * 
		 * @param za source zenith angle
		 * @param viewingLimits The latitude viewing limits set by camera FOV
		 * @param lowLim Lowest earth emergence angle available (0.1 deg in radians is default)
		 * @param upLim Lowest earth emergence angle available (42 deg in radians is default)

		 * return a vector with the adjusted limits in case the previous range was not within the limits
		 * */
		inline std::vector<double> CheckEmergenceAngleValidity(double za, std::vector<double> viewingLimits, double lowLim = 0.0017453292519943296, double upLim = 0.715584993317675 ){
			
			std::vector<double> rViewingLimits = std::vector<double>(2,0.0);
			double lowLat = lowLim - za + TMath::Pi()/2.0;
			double upLat = upLim - za + TMath::Pi()/2.0;

			if(viewingLimits[0]<lowLat){
				rViewingLimits [0] = lowLat;
			}else{
				rViewingLimits[0] = viewingLimits[0];
			}

			if(viewingLimits[1]>upLat){
				rViewingLimits [1] = lowLat;
			}else{
				rViewingLimits[1] = viewingLimits[1];
			}

			if(rViewingLimits[0]>upLat || rViewingLimits[1]<lowLat){
				rViewingLimits[0] = -1;
				rViewingLimits[1] = -1;
			}


			return rViewingLimits;

		}
		/**
		 * 
		 * Performs a subtraction of 2 vectors 
		 * r = v2-v1
		 * 
		 * returns r a vector with the elementwise difference
		 * */
		inline std::vector<double> VectorSubtraction(std::vector<double> v1, std::vector<double>v2){
			std::vector <double> r = std::vector<double>(3,0);
			for(int i = 0; i<3; i++){
				r[i] = v2[i] - v1[i];
			}

			return r;
		}

		/**
		 * 
		 * Returns the cartesian magnitude of the vector v
		 * 
		 * @param v Vector describing the position of an object
		 * */
		inline double VectorMagnitude(std::vector<double> v){
			return TMath::Sqrt((v[0]*v[0])+(v[1]*v[1])+(v[2]*v[2]));
		}

		/**
		 * Return scalar product of 2 vectors
		 * @param v1 Vector to be multiplied
		 * @param v2 Vector to be multiplied
		 * 
		 * return  scalar product
		 * */

		inline double VectorScalarProduct(std::vector<double> v1, std::vector<double> v2){
			double prod =0;
			for(int i = 0; i<v1.size(); i++){
				prod += v1[i]*v2[i];
			}

			return prod;
		}

		/**
		 * 
		 * Calculates a square matrix (MxM) multiplication with a vector of dimension M.
		 * 
		 * @param v vector to be multiplied
		 * @param rot A matrix in a 2x2 vector
		 * 
		 * return A vector resulting from the multiplication
		 * */
		inline std::vector<double> VectorRotation(std::vector<double> v, std::vector<std::vector<double>> rot){

			std::vector<double> vRot = std::vector<double>(3,0);

			for(int i = 0; i < rot.size(); i++){
				for(int j = 0; j<rot[i].size(); j++ ){
					vRot[i] += rot[i][j]*v[j];
				}
			}

			return vRot;
		}

		/**
		 * 
		 * Rounds number to nearest quarter decade (for sampling available for neutrino energies in data files)
		 * 
		 * Rounding is centered about the quarter decades with mid points rounded away from 0
		 * 
		 * @param val a double to be rounded
		 * 
		 * return rounded number to quarter decade
		 * 
		 * */
		inline double RoundToQuarterDecade(double val){
			double qtr = 0.25;
			double fraction = val - (int)val;

			int count = 0;
			do{
				fraction -= qtr/2.0;
				count++;
			}while(fraction >= 0);

			fraction  = ((count - 1)/2 + (count - 1)%2)*(qtr);

			double roundVal = (int)val + fraction;

			return roundVal;
			
		}
		/**
		 * 
		 * Calculates angle using Law of Sines
		 * 
		 * @param a A double with a length of the triangle
		 * @param b A double with a length of the triangle
		 * @param A A double with the angle opposite to a in radians
		 * 
		 * @return B A double with the angle opposite to b
		*/

		inline double AngleFromSineLaw(double a, double b, double A){
			double B = TMath::ASin(b*TMath::Sin(A)/a);

			return B;
		}


	}
}

#endif