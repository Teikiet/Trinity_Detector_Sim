/*
VERSION1.0
20February2025
*/

#ifndef GTDEMTELESCOPE
#define GTDEMTELESCOPE

#include "TGraph.h"
#include "TH1.h"

#include "Math/Vector3Dfwd.h"
#include "Math/GenVector/Rotation3Dfwd.h"
#include "Math/GenVector/RotationXfwd.h"
#include "Math/GenVector/RotationYfwd.h"
#include "Math/GenVector/RotationZfwd.h"

#include "AOpticsManager.h"

// forward declarations
enum TelType;
struct mirrorSegmentDetails;
class TFile;
class TTree;
class ARay;
class TGraph;

/*! GSCTelescope concrete class for ACT telescopes
  inherits from GTelescope.

  GSCTelescope provides the concrete class for a Schwarzschild-Couder
  telescope.  Inherits from GTelescope base class. This class is based on 
  Akira Okamura's NewSCT.C root script included in the GrOptics/scripts/SegSC
  directory
*/
class GTDEMTelescope : public GTelescope {

  friend class GTDEMTelescopeFactory;
 
  AOpticsManager* fManager;

  ARay *ray;
  TFile *hisF;
  TTree *hisT;

  //!< top volume dimensions
  Double_t fTX; 
  Double_t fTY;
  Double_t fTZ;

  int iTelID;  //*< telescope id in the array.
  int iStdID;  //*< telescope standard number from the telescope factory

  double fAvgTransitTime;
  Double_t fPlateScaleFactor;
  Double_t fPlateScaleFactor10;

  double fphotonInjectLoc[3];
  double fphotonInjectDir[3];
  double fphotWaveLgt;
  Double_t fphotonToTopVolTime;

  Double_t fInjectLoc[3];
  Double_t fInjectDir[3];
  Double_t fInjectTime;  // only sent to history file
  Double_t fInjectLambda;
 
  // general telescope parameters
  Double_t fF;     //*< Focal length
  Double_t fTelRadius;

  // colors of elements in gl picture (set in initialize method)
  Int_t iMirrorColor;
  Int_t iObscurationColor;

  map<int, TGraph *> *mGRefl;
  Int_t iReflect;

  string historyFileName; /*!< name of photon history file, 
                            if "", no history written */
  string historyTreeName;
  bool bPhotonHistoryFlag; //*< if false, no photon history file written

  Double_t fLocLast[3];
  Double_t fDirLast[3];
  Double_t fTimeLast;
  int iHistoryOption;
  Int_t fStatusLast;
  Int_t fNPoints;
  Double_t fInitialInjectLoc[3];

  Double_t fRotationOffset;

  //

  bool bRayPlotModeFlag;
  enum RayPlotType eRayPlotType;

  TelType eTelType; //!< telescope type enum (here value is TDEM)

  void makePhotonHistoryBranches();

  void fillPhotonHistory();

  void initializePhotonHistoryParms();

  void movePositionToTopOfTopVol();

  void initialize();

  void addCamera();

  void addMirrors();

  void addArms();
  
  void addDoor();

 public:

  /*! GSCTelescope constructor
   */
  GTDEMTelescope();

  /*! GSCTelescope destructor 
   */
  ~GTDEMTelescope();

  /*! Build an ideal SC telescope. More
      realistic design and parameter reader should be implemented.
      See http://jelley.wustl.edu/actwiki/images/0/05/VVV-OSdefinitions_v2.pdf
   */
  void buildTelescope();

  void addEntranceWindow();

  void closeGeometry();

  /*! injectPhoton for ray tracing through telescope.

    \param photonLocT  photon ground location relative to telescope (tel.coor)
    \param photonDirT  photon dirCosines in telescope coor. 
    \param photWaveLgt photon wavelength in nm
  */
  void injectPhoton(const ROOT::Math::XYZVector &photonLocT,
                    const ROOT::Math::XYZVector &photonDirT,
                    const double &photWaveLgt);

  /*! \brief getCameraPhotonLocation gets camera location following ray tracing. 
         RETURN FALSE IN CASE PHOTON DOESN'T REACH CAMERA

         \param x  photon camera x location (CHANGE TO POINTER)
         \param y  photon camera y location (CHANGE TO POINTER)
         \param z  photon camera z location (CHANGE TO POINTER)
	 \return true if photon reaches focal surface
  */
  bool getCameraPhotonLocation(ROOT::Math::XYZVector *photonLoc,
                               ROOT::Math::XYZVector *photonDcos,
                               double *photonTime, bool bnchFirstPh);
  
  /*! select a print option
    
    \param oStr output stream
    \param prtMode print mode
  */
  void printTelescope();

  void drawTelescope(const int &option = 0);

  /*! \brief setPrintMode used for setting printing details
    
    \param oStr output stream
    \param prtMode pring mode, fix up later
  */
  void setPrintMode(ostream &oStr=cout,const int prtMode=0);
   
  /*! \brief 
    
   */ 
  void setPhotonHistory(const string &rootFile,const string &treeName,
                        const int &option = 0);

  void writePhotonHistory();

  double getAvgTransitTime() {
    return fAvgTransitTime;
  }

  void setTelID(const int &telID) {
    iTelID = telID; };
  
  void setStdID(const int &stdID) {
    iStdID = stdID; };
 
  void setReflCoeffMap(map<int, TGraph *> *mGr);

  double getTelescopeRadius() {
    return fTelRadius;  // max radius of primary
  }

  double getFocalLength() {
    return fF;
  }

  double getPlateScaleFactor() {
    return fPlateScaleFactor;
  }

  double getIdealTransitTime() {
    //double tm = ( ( 2*fZs - fZp -fZf )* fFocLgt / TMath::C()) * 1.0e09;
    //return tm;
    return fAvgTransitTime;
  }

  TGraph * makeReflectivityGraph(const Int_t &irefl);

  void testPerformance();

  void CloseGeometry(); 

  AOpticsManager *getManager() const { return fManager;};

  void setRayPlotMode(const enum RayPlotType &eRayPlot) {
    bRayPlotModeFlag = true;
    eRayPlotType = eRayPlot;    
    bool debug = true;
    if (debug) {
      *oLog << "  -- GTDEMTelescope::setRayPlotMode " << endl;
      *oLog << "bRayPlotModeFlag eRayPlotType " 
            <<  bRayPlotModeFlag << "  " << eRayPlotType  << endl;
    }
    
  };
};


#endif
