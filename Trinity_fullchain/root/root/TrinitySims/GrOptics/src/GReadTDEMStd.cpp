/*
VERSION4.0
30May2016
*/
/*!  GReadSegSCStd.cpp
     Charlie Duke
     Grinnell College
 */

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <map>
#include <list>
#include <iterator>
#include <algorithm>
#include <bitset>
#include <iomanip>

#include "TGraph.h"
#include "TMatrixD.h"
#include "TMath.h"
#include "Math/Vector3D.h"

using namespace std;

#include "GDefinition.h"
#include "GPilot.h"
#include "GUtilityFuncts.h"

#include "GTelescope.h"
#include "GTDEMTelescope.h"

#include "GTelescopeFactory.h"
#include "GTDEMTelescopeFactory.h"

#include "GReadTDEMStd.h"

#define DEBUG(x) *oLog << #x << " = " << x << endl
#define DEBUGS(x) *oLog << "       "<< #x << " = " << x << endl

GReadTDEMStd::GReadTDEMStd(const string &pilotfile,GTDEMTelescopeFactory *TDEMFactory  ) {

  // initialize variables
  bool debug = false;
  if (debug) {
    *oLog << "  -- GReadTDEMStd::GReadTDEMStd" << endl;
  }

  TDEMFac = 0;
  spilotfile = "";
  pi = 0;
  flagline = "";
  iStdNum = 0;
  opt = 0;
  
  TDEMFac = TDEMFactory;
  spilotfile = pilotfile;
  *oLog << "spilotfile " << spilotfile << endl;
  setupTDEMFactory();

};
/****************** end of GReadTDEMStd **********/

GReadTDEMStd::GReadTDEMStd(const string &pilotfile) {
  spilotfile = pilotfile;

  bool debug = false;
  if (debug) {
    *oLog << "  -- GReadTDEMStd::GReadTDEMStd(const string pilotfile) " 
          << pilotfile << endl;
  }

 // initialize variables
  TDEMFac = 0;
  pi = 0;
  flagline = "";
  iStdNum = 0;
  opt = 0; 
};
/****************** end of GReadTDEMStd **********/

GReadTDEMStd::~GReadTDEMStd() { 
  bool debug = false;
  if (debug) {
    *oLog << "  -- GReadTDEMStd::~GReadTDEMStd " << endl;
  }
  //SafeDelete(pi); 

};
/****************** end of ~GReadTDEMStd **********/

void GReadTDEMStd::
setTDEMTelescopeFactory(GTDEMTelescopeFactory *TDEMFactory) {
  bool debug = false;
  if (debug) {
    *oLog << "  -- GReadTDEMStd::setTDEMTelescopeFactory" << endl;
  }
  if (TDEMFac != 0 ) {
    cerr << "GReadTDEMStd::setSCTelescopeFactory " << endl;
    cerr << "   ERROR: TDEMFac pointer to TDEMFactory previously set" << endl;
    cerr << "   stopping code " << endl;
    exit(0);
  }
  TDEMFac = TDEMFactory;
  setupTDEMFactory();
};

/****************** end of setSCTelescopeFactory **********/
void GReadTDEMStd::setupTDEMFactory() {

  bool debug = false;

  if (debug) {
    *oLog << "  -- GReadTDEMStd::setupTDEMFactory" << endl;
    //*oLog << "       spilotfile " << spilotfile << endl;
  }

  if (TDEMFac == 0) {
    cerr << " GReadTDEMStd doesn't have a TDEMFactory object" << endl;
    cerr << "exiting code" << endl;
    exit(0);
  }
  getReflCoeff();
  pi = new GPilot(spilotfile);
  // check for additional pilot files to append to this file in the pilotreader
  string flag = "PILOTF";
  pi->set_flag(flag);
  while (pi->get_line_vector(tokens) >=0) {
    string newPilotFile = tokens.at(0);
    pi->addPilotFile(newPilotFile);
  }
  flag = "TELTDEMSTD";
  pi->set_flag(flag);
  while (pi->get_line_vector(tokens) >=0) {

    int iStdOptNum = atoi(tokens.at(0).c_str());
    TDEMFac->mStdOptics[iStdOptNum] =  new TDEMStdOptics();
    opt = TDEMFac->mStdOptics[iStdOptNum];

    // set standard number (also available as map key)
    opt->stdNum = iStdOptNum;

    double fFocLgt = atof(tokens.at(1).c_str());
    opt->fF = fFocLgt;

    double fAvgTransitTime = atof(tokens.at(2).c_str());
    opt->fAvgTransitTime = fAvgTransitTime;

    double rotationOffset = atof(tokens.at(3).c_str());
    opt->fRotationOffset = rotationOffset;

    if (tokens.size() >= 2) {
      int iprintmode = atoi(tokens.at(4).c_str());
      opt->iPrtMode = iprintmode;
    }
  }
  // read PLATESCALE record ///////////////
  flag = "PLATESCALE"; 
  pi->set_flag(flag);

  while (pi->get_line_vector(tokens) >=0) {
    int iStdOptNum = atoi(tokens.at(0).c_str() );
    opt = TDEMFac->mStdOptics[iStdOptNum];   
    opt->fPlateScaleFactor = atof(tokens.at(1).c_str() );
  }

  // calculate plate scale factor if no record present.
  if ( (opt->fPlateScaleFactor) < 0.00001) {
    opt->fPlateScaleFactor = tan( 1.0*(TMath::DegToRad())) *
      opt->fF * 100.0;
  }  
 
  SafeDelete(pi);

  //if (iPrtMode > 0) {
  if (debug) {
    *oLog << endl;
    *oLog << "       printing all TDEM standard optics objects" << endl;
    for (TDEMFac->itmStdOp = TDEMFac->mStdOptics.begin();
    	 TDEMFac->itmStdOp != TDEMFac->mStdOptics.end();
    	 TDEMFac->itmStdOp++) {
      (*(TDEMFac->itmStdOp)).second->printTDEMStdOptics();
    }
  }

  if (debug) {
    *oLog << "  -- end of setupTDEMFactory" << endl;
  }

};

/******************** end of setupSCFactory ****************/

void GReadTDEMStd::getReflCoeff() {

  // wavelengths in the config. file have nm units
  // here convert to cm as required robast usage
  bool debug = false;

  if (debug) {
    *oLog << "  -- GReadTDEMStd::getReflCoeff() " << endl;
    *oLog << "       spilotfile " << spilotfile << endl;
  }

  ifstream inFile(spilotfile.c_str(),ios::in);
  if (! inFile) {
    cerr << "  -- GReadTDEMStd::getReflCoeff " << endl;
    cerr << "    could not open file: " << spilotfile << endl;
    exit(0);
  }

  string pilotline;
  while( getline(inFile,pilotline,'\n')) {
    vector<string> tokens1;
    GUtilityFuncts::tokenizer(pilotline,tokens1);
    if ( (tokens1.size() > 0) &&
         (tokens1.at(0) == "*")  &&
         (tokens1.at(1) == "RFCRV") )  {

      int index = atoi(tokens1.at(2).c_str());
      int number = atoi(tokens1.at(3).c_str());
      map<int, TGraph *>::iterator itGr;
      itGr =  TDEMFac->mGRefl->find(index);

      if (itGr!=TDEMFac->mGRefl->end()) {
        cerr << "trying to load reflection curve " << index << endl;
        cerr << "but curve already in the map: check pilot files " << endl;
        exit(0);
      }

      (*(TDEMFac->mGRefl))[index] = new TGraph(number);
      TGraph *grTmp = (*(TDEMFac->mGRefl))[index];

      // get ready to read and load reflection coefficients

      // read and store the reflection coefficients
      for (int i = 0;i<number;i++) {
        double wavel     = 0.0;
        double reflcoeff = 0.0;
        inFile >> wavel >> reflcoeff;
	// convert to cm from nm.
	wavel = wavel*1.0e-07;
        grTmp->SetPoint(i,wavel,reflcoeff);
      }
      getline(inFile,pilotline);// finish the end of the line
   
      if (debug) {
	*oLog << "       reading in reflectivity curve " << index 
	      << "   with " << number << " points" << endl;
	*oLog << "             converting from nm to cm for robast" << endl;
      }
      
      if (debug) {
        *oLog << "      print reflectance vectors for index: " << index 
             << "  number of points " << number << endl;
	*oLog << "             wavelengths stored in cm, not nm" << endl;
        for (int i=0;i<number;i++) {
	  double x = 0.0;
	  double y = 0.0;
	  grTmp->GetPoint(i,x,y);
	  *oLog << "          " << setw(4) << i << "   " 
		<< x/1.0e-07 << "     "
		<<  y << endl;
        }
        *oLog << endl;
      }        
    }
  }

  if (TDEMFac->mGRefl->size() == 0) {
    *oLog << "no reflection coeffient table found in config file" << endl;
    *oLog << "    STOPPING CODE" << endl;
    exit(0);
  }
  
};
