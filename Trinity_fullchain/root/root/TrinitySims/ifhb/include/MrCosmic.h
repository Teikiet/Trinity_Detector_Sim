#ifndef F_MRCOSMIC_H
#define F_MRCOSMIC_H

#include <iostream>
#include <vector>
#include <TGraph.h>
#include <TRandom3.h>

#include "ReadConfiguration.h"
#include "Utilities.hpp"
#include "Constants.hpp"
namespace IFHB{
	/**
	 * \class MrCosmic
	 * \brief Container class for above the limb cosmic ray showers
	 * 
	 * This class samples from an energy spectrum with a user-defined gamma. Calculates the distance to the first interaction based on the travesed atmospheric depth. An exponential atmosphere is assumed
	 * 
	 * I have named this class to maintain the play on words with Mrs Tau
	*/
	class MrCosmic{
		public:
			/**
			 * Standard MrCosmic constructor
			*/
		   MrCosmic();
		   /**
			* Standard MrCosmic constructor with parameters from ReadConfiguration object
		   */
		   MrCosmic(ReadConfiguration* readConf);
		   /**
			* Standard MrCosmic destructor
		   */
		   ~MrCosmic();
			/**
			* Sets Random number generator engine
			* 
			* @param rand An initialized TRandom3 number generator
			* 
			*/
		    void SetRandomEngine(TRandom3 *rand);

		   /**
			* Runs the simulation
			* 
			* MonteCarlo to find the location of the first interaction for that energy
			* 
			* @return A bool whether the particle interacted or not
		   */
		 	void RunSimulation();
			/**
			 * Calculates the probability of interaction for a particle going through the atmosphere
			 * 
			 * @return prob The probability of interaction
			*/
			double ProbabilityOfInteraction();
		   /**
			 * 
			 * Sample from energy spectrum for CR. In case a fixed energy for the shower is not desired, it randomly samples from a -2 index profile
			 * Range of sampling is from 10^6 to 10^12 GeV
			 * 

			 * */
		void SampleCosmicSpectrum(double eStart=1e6, double eEnd=1e12);
		   /**
			* Gets the distance to first interaction in g/cm2
			* 
			* @return a double with the grammage traversed by the particle
		   */
		  double GetDistOFI();
		  /**
		   * Loads cross section from file
		   * 
		   * @param filename A std::string with the path to the file
		   * 
		  */
		 void LoadXSectionFromFile(std::string filename);
		 /**
		  * 
		  * Sets geometric position of shower
		  * 
		  * @param polar Polar angle between z axis and location of shower
		  * @param phi Azimuthal angle from x axis of shower location
		  * 
		  * 
		 */
		void SetPosition(double polar, double phi);
		/**
		 * Sets Energy of cosmic Ray
		 * 
		 * @param energy A double with the energy in GeV
		*/
		void SetEnergy(double energy);
		/**
		 * Get position vector of cosmic rays
		 * 
		 * @return a std::vector with the cartesian location
		*/
		std::vector<double> GetPosition();
		/**
		 * Get Energy of particle
		 * 
		 * @return energy of particle in GeV
		*/
		double GetEnergy();
		/**
		 * Set Maximum grammage from insertion of shower to telescope
		 * 
		 * @param telPos A std::vector with the telescope position in km
		 * @param anglePositions Double with angle in radians between shower insertion and telescope position
		 * @param angleTraj A double with the angle between the insertion point and nadir
		*/
		void SetMaxDist(std::vector<double> telPos, double anglePositions, double angleTraj);


		private:
			double energy; /**< Energy of the shower primary*/
			double distOFI; /**< Distance to first interaction from entrance*/
			double angleBeta; /**< Angle between shower insertion point and particle trajectory*/
			double angleAlpha; /**< Angle between shower insertion point and telescope*/
			double xSection;/**< Particle cross-section for the given energy*/
			double maxDist;/**< Dist from particle insertion*/
			double gammaIndex;/**< Energy distribution gamma index*/
			std::vector<double> pos; /**< Vector pointing to the entrance point of the particle*/

			TGraph *grXSection; /**< Loaded cross section for particles as function of Energy*/
			TRandom3 *rand; /**< Random number generator engine*/

	};
}
#endif