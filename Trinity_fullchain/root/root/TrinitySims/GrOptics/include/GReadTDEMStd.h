/*
VERSION4.0
30May2016
*/
/*! \brief  GReadSegSCStd class: concrete class for reading 
      standard telescope configurations for use by telescope factory

 */

#ifndef GREADTDEMSTD
#define GREADTDEMSTD

class GPilot; 
class GTDEMTelescopeFactory;

class GReadTDEMStd {

  // set factory as a friend or the reverse?
 private: 

  // used by all telescope types for various printing modes
  //ostream *oPrtStrm;
  //int iPrtMode;

  GTDEMTelescopeFactory *TDEMFac;

  // for reading pilot file, can be appended files
  string spilotfile;   //!< pilot file
  GPilot *pi;        //!< pilot file reader
  vector<string> tokens; //!< vector of pilot record string tokens
  string flagline;       //!< record flag for pilot file reading

  //map<int, vector<double> > *mVReflWaveLgts;
  //map<int, vector<double> > *mVCoeffs;

  int iStdNum;  //!< active telescope number used in filling entries

  TDEMStdOptics *opt;  //!< working SCStdOptics from DCFac.

  void setupTDEMFactory();

  void getReflCoeff();
  
 public:

  GReadTDEMStd(const string &pilotfile,GTDEMTelescopeFactory *TDEMFactory );

  GReadTDEMStd(const string &pilotfile);

  ~GReadTDEMStd();

  void setTDEMTelescopeFactory(GTDEMTelescopeFactory *TDEMFactory);  

  //void setPrintMode(ostream &oStr=cout,const int prtMode=0);

};

#endif
