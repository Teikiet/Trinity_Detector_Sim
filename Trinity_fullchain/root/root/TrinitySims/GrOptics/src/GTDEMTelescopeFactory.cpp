/*
VERSION1.0
20February2025
*/
/*!  Modified from GSegSCTelescopeFactory.cpp

     Charlie Duke
     Grinnell College
     May 2011

 */
/*
    Jordan Bogdan
    Georgia Institute of Technology
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
#include <assert.h>

#include "TROOT.h"
#include "TGraph.h"
#include "TMatrixD.h"
#include "TMath.h"
#include "Math/Vector3D.h"

using namespace std;

#include "GDefinition.h"

#include "GPilot.h"
#include "GUtilityFuncts.h"

#include "AOpticsManager.h" 
#include "GTelescope.h"
#include "GTDEMTelescope.h"
#include "GSCTelescope.h"
#include "GTelescopeFactory.h"
#include "GSCTelescopeFactory.h"
#include "GTDEMTelescopeFactory.h"

#include "GReadTDEMStd.h"

#define DEBUG(x) *oLog << #x << " = " << x << endl
#define DEBUGS(x) *oLog << "       "<< #x << " = " << x << endl

// define useful units
static const double cm = AOpticsManager::cm();
static const double mm = AOpticsManager::mm();
static const double um = AOpticsManager::um();
static const double nm = AOpticsManager::nm();
static const double  m = AOpticsManager::m();

/*!  /brief TDEMStdOptics structure stores details of a standard 
     Davis-Cotton telescope
 */
TDEMStdOptics::TDEMStdOptics() {

  bool debug = false;
  if (debug) {
    *oLog << "  -- TDEMStdOptics::TDEMStdOptics " << endl;
  }

  iPrtMode = 0;
  oStr     = oLog;

  stdType = TDEM; 
  stdNum = 0;
  fAvgTransitTime = 0.0;
  fRotationOffset = 0.0;
  fPlateScaleFactor = 0.0;
  
  fF = 0.0;     // Focal length
  //
};
/************** end of TDEMStdOptics ***********************/

TDEMStdOptics::TDEMStdOptics(const TDEMStdOptics &sco) {

  (void) sco; // unused
  bool debug = false;
  if (debug) {
    *oLog << "  -- TDEMStdOptics::TDEMStdOptics " << endl;
  }
  

};

/************** end of TDEMStdOptics ***********************/
TDEMStdOptics::~TDEMStdOptics() {
  bool debug = false;
  if (debug) {
    *oLog << "  -- TDEMStdOptics::~TDEMStdOptics" << endl;
  }
};
/********************* end of ~TDEMStdOptics *****************/

void TDEMStdOptics::printTDEMStdOptics() {

  bool debug = false;
  if (debug) {
    *oLog << "  -- TDEMStdOptics::printTDEMStdOptics " << endl;
  }
  if (iPrtMode == 0) return;
  *oLog << " PRINTMODE iPrtMode = " << iPrtMode << endl;
  *oLog << "    TDEMStdOptics::printTDEMStdOptics() " << endl;
  *oLog << "        stdType " << stdType << endl;
  *oLog << "        telType " << getTelType(stdType) << endl;
  *oLog << "        stdNum  " << stdNum << endl;
  *oLog << "        fF      " << fF << endl;
  *oLog << "        fPlateScaleFactor " <<  fPlateScaleFactor << endl;
  *oLog << "        fAvgTransitTime " << fAvgTransitTime << endl;
  *oLog << "        fRotationOffset " << fRotationOffset << endl;

  *oLog << "       iPrtMode " << iPrtMode << endl;
    
};
/************** end of printTDEMStdOptics ***********************/
void TDEMStdOptics::printSegVector (const vector<mirrorSegmentDetails *> &vec) {
  *oLog << "        num rmin    rmax marg dPhi refl rough errXYZ rotErrABC"
        << endl;
  int numElem = vec.size();
  for (int i = 0;i<numElem; i++) {
    mirrorSegmentDetails *t = vec[i];
    *oLog << "         " << i+1 << "  " 
          << t->rmin << "  " << t->rmax << "  " << t->margin << "  " 
          << t->delPhi << "  " << t->reflect << "   " << t->roughness
          << "      " << t->posErrorX
          << " " << t->posErrorY << " " << t->posErrorZ << "    " 
          << t->rotErrorPhi << " " << t->rotErrorTheta << " " << t->rotErrorPsi
          << endl;
  }

};
//////////////////////////////////////////////////////////////////////////

GTDEMTelescopeFactory::
GTDEMTelescopeFactory(GReadTDEMStd &scReader,
		       const string &editPilotFile) {

  printParameters = false;
  bool debug = false;
  sPilotEdit = editPilotFile;
  if (debug) {
    *oLog << "  -- GTDEMTelescopeFactory Constructor:  " << sPilotEdit << endl;
  }
  iNumTDEMTelMade = 0;
  readTDEM = 0;
  pi = 0;
  sPilotEdit = "";
  mGRefl = 0;
  opt = 0;
  TDEMTel = 0;

  // make the reflectivity map 
  mGRefl = new map<int, TGraph *>;

  readTDEM = &(scReader);
  readTDEM->setTDEMTelescopeFactory(this);
  sPilotEdit = editPilotFile;
  pi = new GPilot(sPilotEdit);
  if (debug) {
    *oLog << "    -- end of GTDEMTelescopeFactory constructor" << endl;
  }
  
};

/************** end of GTDEMTelescopeFactory ***********************/

GTDEMTelescopeFactory::~GTDEMTelescopeFactory() {
 
  bool debug = false;
  if (debug) {
    *oLog << "  -- GTDEMTelescopeFactory::~GTDEMTelescopeFactory" << endl;
  }

  for (itmStdOp=mStdOptics.begin();
     itmStdOp!=mStdOptics.end(); itmStdOp++) {
    // WHY CAN'T I DELETE THIS CLASS? thinks I have already deleted the class?
    // was created in reader: SCFac->mStdOptics[i] = new TDEMStdOptics();
    //TDEMStdOptics *tmpSeg;
    //tmpSeg = itmStdOp->second;
    //*oLog << "tmpSeg->stdNum " <<tmpSeg->stdNum << endl;
    //delete tmpSeg;
    SafeDelete( itmStdOp->second );
    //*oLog << " (itmStdOp->second)->stdNum " <<  (itmStdOp->second)->stdNum << endl;
    //*oLog << "itmStdOp->second " << itmStdOp->second << endl;
  }
   for (itmGRefl=mGRefl->begin();
    itmGRefl!=mGRefl->end(); itmGRefl++) {
     SafeDelete(itmGRefl->second ); 
   }
  SafeDelete(mGRefl);
  SafeDelete(pi);
  SafeDelete(readTDEM);
 
};
/************** end of ~GTDEMTelescopeFactory ***********************/

GTDEMTelescope* GTDEMTelescopeFactory::makeTelescope(const int &id,
                                                     const int &std) {
  
  int debug = true;
  if (debug) {
    *oLog << " -- GTDEMTelescopeFactory::makeTelescope" << endl;
    *oLog << "      telID  = " << id << endl;
    *oLog << "      telStd = " << std << endl;
  }

  Int_t idTel = id;
  Int_t iStdID = std;
  iNumTDEMTelMade++; // increment number of TDEM telescopes made by factory
  
  // get parameters for this telescope
  itmStdOp = mStdOptics.find(iStdID);
  assert(itmStdOp != mStdOptics.end());
  // make pointer to working stdoptics structure, easier typing
  opt = mStdOptics[iStdID];

  // make the telescope
  TDEMTel = new GTDEMTelescope;
  TDEMTel->iPrtMode = opt->iPrtMode;

  // move over all reflection coefficients (the entire map)
  TDEMTel->setReflCoeffMap(mGRefl);
  
  TDEMTel->setTelID(idTel);
  TDEMTel->setStdID(iStdID);
  
  TDEMTel->eTelType = opt->stdType;
  TDEMTel->fAvgTransitTime = opt->fAvgTransitTime;
  TDEMTel->fRotationOffset = opt->fRotationOffset;
  TDEMTel->fPlateScaleFactor = opt->fPlateScaleFactor;

  // general telescope parameters
  //TDEMTel->fF = (opt->fF);

  //

  TDEMTel->buildTelescope();

  return TDEMTel;
};
/************** end of makeTelescope ***********************/

void GTDEMTelescopeFactory::printStdTelescope(const int &iStd, 
                                               const int &mode,ostream &oStr) {
  (void) iStd;  // unused
  (void) mode; // unused
  oStr << "unused oStr in parameter list" << endl;
  // DO NOT USE.
  bool debug = false;
  if (debug) {
    *oLog << " -- GTDEMTelescopeFactory::printStdTelescope" << endl;
  }
  
};
/************** end of :printStdTelescope ***********************/

void GTDEMTelescopeFactory::setPrintMode(ostream &oStr,
                                          const int prtMode) {
  oStr << "unused ostream in parameter list" << endl;
  (void) prtMode; // unused
  bool debug = false;
  if (debug) {
    *oLog << " -- GTDEMTelescopeFactory::setPrintMode" << endl;
  }
 
}; 
/************** end of setPrintMode  ***********************/



