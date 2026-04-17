/*
VERSION4.0
30May2016
*/
/*! \brief GSegSCTelescopeFactory concrete class for creating ACT 
  Telescopes

Modified Factory design pattern where GTelescopeFactory provides 
the base class for DC and SC telescope concrete factories. 
GSegSCTelescopeFactory produces the SC telescopes.

*/
#ifndef GTDEMTELESCOPEFACTORY
#define GTDEMTELESCOPEFACTORY

// forward declarations
class TGraph;
class GPilot; 
class GTDEMTelescope;
class GTelescope;
class GReadTDEMStd;
struct mirrorSegmentDetails;

// move following declaration to GDefinition.h
// if a structure/variable used in more than one file
// put the declaration in GDefinition.h
/*
  struct mirrorSegmentDetails {
  Double_t rmin;
  Double_t rmax;
  Double_t margin;
  Double_t delPhi;
  Int_t reflect;
  Double_t posErrorX;
  Double_t posErrorY;
  Double_t posErrorZ;
  Double_t rotErrorPhi;
  Double_t rotErrorTheta;
  Double_t rotErrorPsi;
  Double_t roughness;
  Int_t bRead; // if 0, set from BASIC; if 1, set from CHANGE
};
*/

/*!  /brief SegSCStdOptics structure stores details of a standard 
     Davis-Cotton telescope
 */
struct TDEMStdOptics {

  ostream *oStr;
  Int_t iPrtMode;

  TelType stdType; 
  Int_t stdNum;
  Double_t fAvgTransitTime;
  Double_t fRotationOffset;
  Double_t fPlateScaleFactor;

  Double_t fF;     // Focal length

  TDEMStdOptics(); 

  ~TDEMStdOptics();

  // copy constructor
  TDEMStdOptics(const TDEMStdOptics &sco);

  void setPrintOptions(ostream *outStr, const int &prtMode) {
    oStr = outStr;
    iPrtMode = prtMode;
  };

  void printTDEMStdOptics();
  void printSegVector (const vector<mirrorSegmentDetails *> &vec);
};

////////////////////////////////////////////////////////////////

/*! \brief  GSegSCTelescopeFactory concrete class for constructing 
     SC telescope, inherits from GTelescopeFactory to implement
     factory method
 */
class GTDEMTelescopeFactory : public GTelescopeFactory {
 private:

  friend class GReadTDEMStd; 

  GReadTDEMStd *readTDEM;  //!< SegSC base reader
  bool printParameters;  //!< print parameters upon construction when true
  GPilot *pi;  //!< pilot reader pointer
  vector<string> tokens;  //!< string vector for GPilot use
  string sPilotEdit;  //!< pilot file string from edit record

  map<int,TDEMStdOptics*> mStdOptics;    //*< map of standard telescopes
  map<int,TDEMStdOptics*>::iterator itmStdOp;  //*< iterator for this map

  map<int, TGraph *> *mGRefl;  //*< mirror reflection coefficients map container
  map<int, TGraph *>::iterator itmGRefl; //*< iterator for above map container
  
  TDEMStdOptics *opt;  //*< working SegSCstdOptics structure for current telescope
  
  GTDEMTelescope *TDEMTel;  //*< pointer to working telescope
  int iNumTDEMTelMade;  
  
  /*! \brief editWorkingTelescope makes edits based on 
           pilotfile entries to telescope currently 
           under construction

           \param SCTel pointer to current telescope
   */

 public:

  /*!  GSegSCTelescopeFactory constructs a SC telescope from 
       standard telescopes obtained from reader. USE THIS CONSTRUCTOR

       \param dcReader SC telescope reader instance 
       \param editPilotFile pilotfile name containing telescope edit records
   */
  GTDEMTelescopeFactory(GReadTDEMStd &scReader,
		    const string &editPilotFile);
  /*! \brief makeTelescope constructs a SC telescope based on 
             instructions in the pilot file

             \param id telescope id within array
             \param std number of standard telescope
             \param xLoc x position within array (meters)
             \param yLoc y position within array (meters)
             \param zLoc z position within array (meters)
             \return GSegSCTelescope pointer to constructed telescope
  */

  ~GTDEMTelescopeFactory();

  GTDEMTelescope *makeTelescope(const int &id,
				 const int &std);
  
  /*! \brief printStdTelescope debug print based on prtMode

      \param iStd standard telescope id
      \param mode printmode (see SegSCStdOptics::printSegSCStdOptics
      in this file, default 0
      \param oStr output stream, default cout
   */
  void printStdTelescope(const int &iStd, const int &mode = 0,
                         ostream &oStr=cout);

  /*! \brief setPrintMode defines the print mode for 
          SegSCStdOptics::printSegSCStdOptics (documented in this file)

      \param oStr output stream, default cout
      \param prtMode  see SegSCStdOptics::printSegSCStdOptics earlier in this file
      \param prtMode  -1: print all standard telescopes
   */
  void setPrintMode(ostream &oStr=cout,const int prtMode=0);
};

#endif
