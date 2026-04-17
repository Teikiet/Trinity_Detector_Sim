/*
VERSION1.0
20ebruary2025
*/
/*  Modified from GSegSCTelescope.cpp

    Akira Okumura 

        and

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
#include <memory>

using namespace std;

#include "TGeoSphere.h"
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TMatrixD.h"
#include "TMath.h"
#include "Math/Vector3D.h"
#include "TRandom3.h"
#include "TPolyLine3D.h"
#include "TString.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TGeoCompositeShape.h"

#include "TGeoManager.h"
#include "TGeoBBox.h"
#include "TGeoMaterial.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include "TGeoShape.h"
#include "TGeoNode.h"
#include "TGeoMedium.h"
#include "TGeoMatrix.h"
#include "TGeoCone.h"
#include "AGlassCatalog.h"
#include "TView.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TVector3.h"
#include "TGeoPgon.h"
#include "TVersionCheck.h"
#include "TGraph.h"
#include "TBrowser.h"
#include "TF1.h"
#include "TStyle.h"

#include "ABorderSurfaceCondition.h"
#include "AGlassCatalog.h"
#include "ALens.h"
#include "AMirror.h"
#include "AObscuration.h"
#include "AOpticsManager.h"
#include "ARay.h"
#include "ARayArray.h"
#include "ARayShooter.h"

#include "GUtilityFuncts.h"
#include "GDefinition.h"

#include "GTelescope.h"
#include "GTDEMTelescope.h"

#define DEBUG(x) *oLog << #x << " = " << x << endl
#define DEBUGW(x) *oLog << "         " << #x << " = " << x << endl

// define useful units
static const double cm = AOpticsManager::cm();
static const double mm = AOpticsManager::mm();
static const double um = AOpticsManager::um();
static const double nm = AOpticsManager::nm();
static const double  m = AOpticsManager::m();

GTDEMTelescope::GTDEMTelescope() {
  
  bool debug = false;
  if (debug) {
    *oLog << "  -- GTDEMTelescope::GTDEMTelescope() " << endl;
  }
  initialize();
  };
/********************** end of GTDEMTelescope *****************/
GTDEMTelescope::~GTDEMTelescope() {

  bool debug=false;
  if (debug) {
    *oLog << "  -- GTDEMTelescope::~GTDEMTelescope " << endl;
  }
  if (fManager !=0) {
    gGeoManager = fManager;
    SafeDelete(fManager);
  }

  if (hisF != 0) SafeDelete(hisF);
 
  map<int, TGraph *>::iterator itmGRefl; 
  for (itmGRefl=mGRefl->begin();
       itmGRefl!=mGRefl->end(); itmGRefl++) {
    SafeDelete(itmGRefl->second ); 
  }
  SafeDelete(mGRefl);
  SafeDelete(ray);
};
/********************** end of ~GTDEMTelescope *****************/

void GTDEMTelescope::buildTelescope()
{
  bool debug = false;
  // fix units
  fF = 148.5 * cm;
  fTelRadius = 1.0 * m; //placeholder

  if (debug) {
    *oLog << "  -- GTDEMTelescope::buildTelescope" << endl;
  }
  gGeoManager = 0;
  fManager = new AOpticsManager("manager","The optics manager of TDEM");
  //fManager->SetVisLevel(5);// should be 0 or 1
  fManager->SetNsegments(50);
  fManager->DisableFresnelReflection(1);

  // Make dummy material
  TGeoMaterial* mat = new TGeoMaterial("mat", 0, 0, 0);
  mat->SetTransparency(70); // needed in OpenGL view, 70
  new TGeoMedium("med", 1, mat);

  // Make the world
  TGeoBBox* worldbox = new TGeoBBox("worldbox", fTX*m, fTY*m, fTZ*m);
  AOpticalComponent* world = new AOpticalComponent("world", worldbox);
  fManager->SetTopVolume(world);
  fManager->SetTopVisible(0); 
  //fManager->SetTopVisible(.9);

  addMirrors();
  addCamera();
  addArms();
  addDoor();

  //The entrance window MUST be added prior to the camera in order to properly compute
  //the focal plane offset introduced by the window's refraction (not elegant, I know...) 
  //addEntranceWindow();

  closeGeometry();

  printTelescope();
  //testPerformance();

  return;
};
/*************************************************************************************/

void GTDEMTelescope::addDoor() {
  bool debug = false;
  if (debug) {
    *oLog << " -- GTDEMTelescope::addDoor" << endl;
  }
  Double_t doorWidth = 1 * m;
  Double_t doorHeight = 1 * m;
  TGeoBBox* wallBox = new TGeoBBox("wallBox",5 * m,5 * m,5 * cm);
  TGeoBBox* doorBox = new TGeoBBox("doorBox",doorWidth,doorHeight,5 * cm);
  TGeoRotation* wallRot = new TGeoRotation("wallRot",1.56,0.,0.);
  wallRot->RegisterYourself();
  TGeoCombiTrans* wallPos = new TGeoCombiTrans("wallPos",0.,0.,2. * m,wallRot);
  wallPos->RegisterYourself();
  TGeoCombiTrans* doorPos = new TGeoCombiTrans("doorPos",0.,0.,2. * m,wallRot);
  doorPos->RegisterYourself();
  TGeoCompositeShape* wallDoor = new TGeoCompositeShape("wallDoor","wallBox:wallPos - doorBox:doorPos");
  AObscuration* wall = new AObscuration("wall",wallDoor);
  wall->SetLineColor(1);
  fManager->GetTopVolume()->AddNode(wall,1);
}

void GTDEMTelescope::addArms() {
  bool debug = false;
  if (debug) {
    *oLog << " -- GTDEMTelescope::addArms" << endl;
  }
  Double_t kArmAngle = 30. * TMath::Pi() / 180;
  Double_t kArmWidth = 50 * mm / 2;
  Double_t kArmLength = 1565 * mm / 2;
  Double_t kHandLength = 200 * mm / 2 + kArmWidth * TMath::Cos(kArmAngle);
  Double_t kArmYOffset = 1082 * mm - kArmLength * TMath::Sin(kArmAngle);
  Double_t kArmZOffset = kArmLength * TMath::Cos(kArmAngle) + 195 * mm;
  TGeoBBox* armBox = new TGeoBBox("armBox",kArmWidth,kArmWidth,kArmLength);
  TGeoBBox* handBox = new TGeoBBox("handBox",kArmWidth,kHandLength,kArmWidth);
  //TGeoTranslation* armTrans = new TGeoTranslation("armTrans",0,kArmYOffset,kArmLength);
  //armTrans->RegisterYourself();
  TGeoRotation* armRot = new TGeoRotation("armRot",0,kArmAngle*180/TMath::Pi(),0);
  //armRot->RegisterYourself();
  TGeoCombiTrans* armRT = new TGeoCombiTrans("armRT",0.,kArmYOffset,kArmZOffset,armRot);
  armRT->RegisterYourself();
  Double_t kHandYOffset = kArmYOffset - kArmLength * TMath::Sin(kArmAngle) - kHandLength + kArmWidth * TMath::Cos(kArmAngle);
  Double_t kHandZOffset = 2 * kArmLength * TMath::Cos(kArmAngle) - kArmWidth * (1 - TMath::Sin(kArmAngle)) + 195 * mm;
  TGeoTranslation* handTrans = new TGeoTranslation("handTrans",0,kHandYOffset,kHandZOffset);
  handTrans->RegisterYourself();
  TGeoCompositeShape* armComposite = new TGeoCompositeShape("armComposite","armBox:armRT+handBox:handTrans");
  AObscuration* handArm = new AObscuration("handArm",armComposite);
  handArm->SetLineColor(1);
  fManager->GetTopVolume()->AddNode(handArm, 1);
  fManager->GetTopVolume()->AddNode(handArm, 1, new TGeoRotation("",0.,0.,135.));
  fManager->GetTopVolume()->AddNode(handArm, 1, new TGeoRotation("",0.,0.,-135.));
}

void GTDEMTelescope::addCamera() {
  bool debug = false;

  if (debug) {
    *oLog << "  --  GTDEMTelescope::addCamera" << endl;
  }
  Double_t kCameraX = 16 * 6 * mm / 2;
  Double_t kCameraY = 16 * 6 * mm / 2;
  Double_t kCameraT = 0.001 * m;
  Double_t kCameraOffsetBox = 14.32 * mm;
  Double_t kCameraBoxX = 160 * mm / 2; 
  Double_t kCameraBoxY = 156 * mm / 2;
  Double_t kCameraBoxZ = 195 * mm / 2;
  Double_t kCameraBoxT = 6.35 * mm;
  // Make a square focal plane
  TGeoBBox* camPlane = new TGeoBBox("camPlane",kCameraX,kCameraY,kCameraT);
  AFocalSurface* focalPlane = new AFocalSurface("focalPlane", camPlane);
  fManager->GetTopVolume()->AddNode(focalPlane, 1, new TGeoTranslation(0, 0, fF + 1 * mm));
  // Make a camera box
  TGeoBBox* blockCameraBox1 = new TGeoBBox("blockCameraBox1",kCameraBoxX,kCameraBoxY,kCameraBoxZ);
  TGeoBBox* blockCameraBox2 = new TGeoBBox("blockCameraBox2",kCameraBoxX-kCameraBoxT,kCameraBoxY-kCameraBoxT,kCameraBoxZ-(kCameraBoxT/2));
  TGeoTranslation* transZ1 = new TGeoTranslation("transZ1", 0, 0, fF + kCameraBoxZ - kCameraOffsetBox);
  transZ1->RegisterYourself();
  TGeoTranslation* transZ2 = new TGeoTranslation("transZ2", 0, 0, fF + kCameraBoxZ - kCameraOffsetBox - kCameraBoxT);
  transZ2->RegisterYourself();
  TGeoCompositeShape* boxComposite = new TGeoCompositeShape("boxComposite", "blockCameraBox1:transZ1-blockCameraBox2:transZ2");
  AObscuration* cameraBox = new AObscuration("cameraBox", boxComposite);
  cameraBox->SetLineColor(1);
  fManager->GetTopVolume()->AddNode(cameraBox, 1);
};
/*******************************************************************/
void GTDEMTelescope::addMirrors() {

  gGeoManager = fManager;
  
  bool debug = false;
  if (debug) {
    *oLog << "  --  GTDEMTelescope::addMirrors" << endl;
  }

  Double_t kMirrorR = fF * 2;
  Double_t kMirrorD = 15 * cm;
  Double_t kMirrorT = 0.01 * mm;

  Double_t mirrorTheta = TMath::ASin(kMirrorD / 2. / kMirrorR) * TMath::RadToDeg();
  TGeoSphere* mirSphere = new TGeoSphere("mirSphere",kMirrorR,kMirrorR + kMirrorT,180. - mirrorTheta,180.);
  AMirror* mir = new AMirror("mirror",mirSphere);
  mir->SetLineColor(iMirrorColor);
  TGraph * graph = makeReflectivityGraph(iReflect);
  mir->SetReflectance(std::shared_ptr<TGraph> (graph)); // graph owned by AMirror (and deleted)

  const int kNMirror = 84;
  Double_t xy[kNMirror][3] = {
    {30.2514, -8.001, 3.3274},
    {30.2514, 8.001, 3.3274},
    {44.1198, -16.002, 7.62},
    {44.1198, 0, 6.7056},
    {44.1198, 16.002, 7.62},
    {57.9628, -24.003, 13.8938},
    {57.9628, -8.001, 12.0142},
    {57.9628, 8.001, 12.0142},
    {57.9628, 24.003, 13.8938},
    {71.8312, -32.004, 22.5298},
    {71.8312, -16.002, 19.5072},
    { 71.8312, 0, 18.5166},
    {71.8312, 16.002, 19.5072},
    {71.8312, 32.004, 22.5298},
    {22.054769255679297, 22.197980900044563, 3.3274},
    {8.19663074432071, 30.198980900044567, 3.3274},
    {35.91803851135859, 30.20786760988867, 7.62},
    {22.059900000000003, 38.20886760988867, 6.7056},
    {8.201761488641417, 46.209867609888676, 7.62},
    {49.76860776703789, 38.19575727447665, 13.8938},
    {35.9104692556793, 46.19675727447665, 12.0142},
    {22.052330744320713, 54.19775727447666, 12.0142},
    {8.194192232962127, 62.19875727447666, 13.8938},
    {63.63187702271718, 46.20564398432076, 22.5298},
    {49.77373851135859, 54.20664398432076, 19.5072},
    {35.9156, 62.20764398432076, 18.5166},
    {22.05746148864142, 70.20864398432076, 19.5072},
    {8.199322977282833, 78.20964398432076, 22.5298},
    {-8.196630744320698, 30.19898090004457, 3.3274},
    {-22.05476925567929, 22.197980900044573, 3.3274},
    {-8.201761488641399, 46.20986760988868, 7.62},
    {-22.0599, 38.208867609888685, 6.7056},
    {-35.918038511358574, 30.207867609888687, 7.62},
    {-8.194192232962102, 62.19875727447666, 13.8938},
    {-22.05233074432069, 54.197757274476665, 12.0142},
    {-35.91046925567928, 46.196757274476674, 12.0142},
    {-49.76860776703787, 38.19575727447668, 13.8938},
    {-8.199322977282804, 78.20964398432076, 22.5298},
    {-22.057461488641394, 70.20864398432077, 19.5072},
    {-35.9156, 62.207643984320775, 18.5166},
    {-49.77373851135857, 54.20664398432078, 19.5072},
    {-63.63187702271716, 46.20564398432079, 22.5298},
    {-30.2514, 8.001, 3.3274},
    {-30.2514, -8.001, 3.3274},
    {-44.1198, 16.002, 7.62},
    {-44.1198, 0, 6.7056},
    {-44.1198, -16.002, 7.62},
    {-57.9628, 24.003, 13.8938},
    {-57.9628, 8.001, 12.0142},
    {-57.9628, -8.0001, 12.0142},
    {-57.9628, -24.003, 13.8938},
    {-71.8312, 32.004, 22.5298},
    {-71.8312, 16.002, 19.5072},
    {-71.8312, 0, 18.5166},
    {-71.8312, -16.002, 19.5072},
    {-71.8312, -32.004, 22.5298},
    {-22.054769255679304, -22.197980900044556, 3.3274},
    {-8.196630744320723, -30.19898090004456, 3.3274},
    {-35.9180385113586, -30.20786760988866, 7.62},
    {-22.0599, -38.208867609888664, 6.7056},
    {-8.201761488641438, -46.20986760988867, 7.62},
    {-49.7686077670379, -38.195757274476634, 13.8938},
    {-35.91046925567932, -46.196757274476646, 12.0142},
    {-22.052330744320734, -54.19775727447665, 12.0142},
    {-8.194192232962152, -62.19875727447666, 13.8938},
    {-63.63187702271719, -46.20564398432073, 22.5298},
    {-49.77373851135861, -54.20664398432074, 19.5072},
    {-35.9156, -62.20764398432075, 18.5166},
    {-22.057461488641444, -70.20864398432076, 19.5072},
    {-8.199322977282861, -78.20964398432076, 22.5298},
    {8.196630744320707, -30.198980900044564, 3.3274},
    {22.054769255679293, -22.197980900044566, 3.3274},
    {8.201761488641413, -46.20986760988867, 7.62},
    {22.0599, -38.20886760988867, 6.7056},
    {35.91803851135859, -30.207867609888673, 7.62},
    {8.19419223296212, -62.198757274476655, 13.8938},
    {22.052330744320706, -54.19775727447666, 12.0142},
    {35.910469255679295, -46.19675727447665, 12.0142},
    {49.76860776703788, -38.195757274476655, 13.8938},
    {8.199322977282826, -78.20964398432076, 22.5298},
    {22.05746148864141, -70.20864398432076, 19.5072},
    {35.9156, -62.20764398432076, 18.5166},
    {49.77373851135859, -54.20664398432076, 19.5072},
    {63.63187702271717, -46.20564398432076, 22.5298}
  };

  for (int i = 0; i < kNMirror; i++){
    Double_t x = xy[i][0] * cm;
    Double_t y = xy[i][1] * cm;
    Double_t z = xy[i][2] * cm;
    Double_t r2d = TMath::RadToDeg();
    Double_t r2 = TMath::Power(x,2) + TMath::Power(y,2);
    TGeoTranslation* trans = new TGeoTranslation(Form("mirTrans%d", i),x,y,z);
    trans->RegisterYourself();
    Double_t phi = (TMath::ATan2(y,x)) * r2d;
	  Double_t theta = (TMath::ATan2(TMath::Sqrt(r2),2 * fF - z)) * r2d;
	  TGeoRotation* rot = new TGeoRotation("",phi - 90.,theta,0);
    TGeoCombiTrans* combi = new TGeoCombiTrans(*trans,*rot);
    combi->RegisterYourself();
    // Added to finish mirror rotation
    TGeoTranslation* transZ = new TGeoTranslation(0, 0, kMirrorR);
    TGeoHMatrix* hmat = new TGeoHMatrix((*combi) * (*transZ));
    
    fManager->GetTopVolume()->AddNode(mir, 1, hmat);
  }
};
/*******************************************************************/
void GTDEMTelescope::closeGeometry()  {
  gGeoManager = fManager;
  fManager->CloseGeometry();
};
/****************************************************************************************/

void GTDEMTelescope::injectPhoton(const ROOT::Math::XYZVector &photonLocT,
                                const ROOT::Math::XYZVector &photonDirT,
				const double &photWaveLgt) {  
  gGeoManager = fManager;

  bool debug = true;
  if (debug) {
    *oLog << " -- GTDEMTelescope::injectPhoton " << endl;
    *oLog << "       photonLocT  ";
    GUtilityFuncts::printGenVector(photonLocT); *oLog << endl;
    *oLog << "       photonDirT  ";
    GUtilityFuncts::printGenVector(photonDirT); *oLog << endl;
    *oLog << "      bPhotonHistoryFlag " << bPhotonHistoryFlag << endl;
  }
  photonLocT.GetCoordinates(fInitialInjectLoc);
  photonLocT.GetCoordinates(fphotonInjectLoc);
  photonDirT.GetCoordinates(fphotonInjectDir); 

  // convert to cm as required for robast
  fphotWaveLgt = photWaveLgt*nm;

  // move base of injection location vector to center of primary mirror
  // (add rotation offset)
  fphotonInjectLoc[2] = fphotonInjectLoc[2] + fRotationOffset;

  movePositionToTopOfTopVol();
 
  // initialize photonHistory parameters if necessary
  if (bPhotonHistoryFlag) {
    initializePhotonHistoryParms();    
  }
  
  // Assuming that three arguments are given in units of (m), (m), (nm)
  double t = 0;
  double x  = fphotonInjectLoc[0];
  double y  = fphotonInjectLoc[1];
  double z  = fphotonInjectLoc[2];
  double dx = fphotonInjectDir[0];
  double dy = fphotonInjectDir[1];
  double dz = fphotonInjectDir[2];

  SafeDelete(ray);
  // ray = new ARay(0, fphotWaveLgt, x*m, y*m, z*m, t, dx, dy, dz);
  cout << "x: " << x*m << " y: " << y*m << " z: " << z*m << " dx: " << dx << " dy: " << dy << " dz: " << dz << endl; 
  ray = new ARay(0, fphotWaveLgt, (x)*m, (y)*m, z*m, t, dx, dy, dz);

  gGeoManager = fManager;

  fManager->TraceNonSequential(*ray);
 
  // Here you can get the traced result
  double new_xyzt[4];
  ray->GetLastPoint(new_xyzt);
  fNPoints = ray->GetNpoints();
  double new_x = new_xyzt[0];
  double new_y = new_xyzt[1];
  double new_z = new_xyzt[2];
  double new_t = new_xyzt[3];
  if (debug) {
    *oLog << " from GetLastPoint " << new_x << " " << new_y 
	  << " " << new_z << " " << new_t << endl;
  }

};
/********************** end of injectPhoton *****************/
void GTDEMTelescope::movePositionToTopOfTopVol() {

  gGeoManager = fManager;

  bool debug = false;
  if (debug) {
    *oLog << "  -- GTDEMTelescope::movePositionToTopOfTopVol " << endl;
    *oLog << "        position prior to move to top ";
    *oLog << fphotonInjectLoc[0] << "  " << fphotonInjectLoc[1] << "  " 
	  << fphotonInjectLoc[2] << endl;
  }

  Double_t rfx = fphotonInjectLoc[0];
  Double_t rfy = fphotonInjectLoc[1];
  Double_t rfz = fphotonInjectLoc[2];

  Double_t Z = fTZ; // just inside top volume

  Double_t dl = fphotonInjectDir[0];
  Double_t dm = fphotonInjectDir[1];
  Double_t dn = fphotonInjectDir[2];

  Double_t Rx = rfx - (rfz - Z)*dl/dn;
  Double_t Ry = rfy - (rfz - Z)*dm/dn;
  Double_t Rz = Z;

  fphotonInjectLoc[0] = Rx;
  fphotonInjectLoc[1] = Ry;
  fphotonInjectLoc[2] = Rz;

  // distance traveled from top to inject location
  Double_t dist = (rfz - Z)/dn;
  fphotonToTopVolTime = - dist/(TMath::C());
  if (debug) {
    *oLog << "        TopVolPos in focal point coor.  ";
    for (int i = 0;i<3;i++) {
      *oLog << fphotonInjectLoc[i] << " ";
    }
    *oLog << endl;
    *oLog << "        distance from top to inject loc " << dist << endl;
    *oLog << "        fphotonToTopVolTime " << fphotonToTopVolTime << endl;
    *oLog << endl;
  }
  
};
//****************************************************
bool GTDEMTelescope::getCameraPhotonLocation(ROOT::Math::XYZVector *photonLoc,
                                           ROOT::Math::XYZVector *photonDcos,
                                           double *photonTime, bool bnchFirstPh) {
  
  gGeoManager = fManager;

  bool debug = false;
  if (debug) {
    *oLog << "  -- GTDEMTelescope::getCameraPhotonLocation " << endl;
  }

  photonLoc->SetCoordinates(0.0,0.0,0.0);
  photonDcos->SetCoordinates(0.0,0.0,-1.0);
  *photonTime = 10.0;
  
  //return true;
  double x[4],dir[3];
  for (int i = 0;i<3;i++) {
    x[i] = 0.0;
    dir[i] = 0.0;
  }
  x[3] = 0.0;

  ray->GetLastPoint(x);
  ray->GetDirection(dir);
  fStatusLast = -1;

  // convert distances to mm from cm.
  for (int i = 0;i<3;i++) {
    x[i] = x[i] * 10.0;
  }
  photonLoc->SetCoordinates(x[0],x[1],x[2]);
  photonDcos->SetCoordinates(dir[0],dir[1],dir[2]);
  *photonTime = (x[3] + fphotonToTopVolTime)*1.0e09;
 
  enum {kRun, kStop, kExit, kFocus, kSuspend, kAbsorb};
  
  if (ray->IsExited()) fStatusLast = kExit;
  else if (ray->IsFocused()) {
    fStatusLast = kFocus;
  }
  else if (ray->IsStopped()) fStatusLast = kStop;
  else if (ray->IsSuspended()) fStatusLast = kSuspend;
  else if (ray->IsAbsorbed() ) fStatusLast = kAbsorb;
  
  if (debug) {
    *oLog << "       GetLastPOint " << endl;
    for (int i = 0; i < 3; i++) {
      *oLog << "            " << i << "  loc " << x[i] << endl;
      *oLog << "            " << i << "  dir " << dir[i] << endl;
    }
    *oLog <<   "            time " << x[3] << endl;
    *oLog <<   "            fStatus " << fStatusLast << "  ";
 
    if (fStatusLast == kRun) {
      *oLog << "  kRun " << endl;
    }
    else if (fStatusLast == kStop) {
      *oLog << "  kStop " << endl;
    }
     else if (fStatusLast == kExit) {
      *oLog << "  kExit " << endl;
    }
    else if (fStatusLast == kStop) {
      *oLog << "  kStop " << endl;
    }
    else if (fStatusLast == kFocus) {
      *oLog << "  kFocus " << endl;
    }
    else if (fStatusLast == kSuspend) {
      *oLog << "  kSuspend " << endl;
    }
        else if (fStatusLast == kAbsorb) {
      *oLog << "  kAbsorb " << endl;
    }

   *oLog << endl;
    *oLog <<   "            *photonTime " << *photonTime << endl;
  }

  if (bPhotonHistoryFlag) {
    for (int i = 0;i<3;i++) {
      fLocLast[i] = x[i];
      fDirLast[i] = dir[i];
    }
    
    fTimeLast = (x[3] + fphotonToTopVolTime)*1.0e09;
    fillPhotonHistory();
  }

  // the following will draw polylines that end up on the camera/focal plane.
  // you have to instantiate app and do a app.run in grOptics.cpp by 
  // uncommenting these lines in grOptics (shortly after the start of main
  // shortly before the end of main.
  // This also works with the testtel option; however, you have to set
  // nPhotons to a small number, e.g. 10 or less in GArrayTel.cpp (in 
  // the testTelescope method. The TPolyLine3D::Print("all") will print
  // the start of the line, the intermediate points, and the end of the 
  // line.  A good way to test the code.

  // fStatusLast can be 0 through 5.

  if (bRayPlotModeFlag && bnchFirstPh) {
    // draw the telescope only once.
    static int idraw = 1;
    if (idraw) {
      //fManager->GetTopVolume()->Draw("ogl");
      drawTelescope(0);
      idraw = 0;
    }
    // do we draw the ray.
    if ( ( (eRayPlotType == FOCUSONLY) && (fStatusLast == 3) ) ||
         (eRayPlotType == ALLSURFACES) ) {
      *oLog << " ready to draw polyline" << endl;
      *oLog << "      ray->GetNpoints(): " << ray->GetNpoints() << endl;
      TPolyLine3D *pol = ray->MakePolyLine3D();
      for (Int_t ii = 0 ;ii < pol->GetN(); ii++ ) {
	*oLog << ii << "  " << (pol->GetP())[ii] << endl;
      }
      pol->Print("all");
      *oLog << " fStatusLast " << fStatusLast << "  ";
      if (fStatusLast == 0) {
	*oLog << "kRun" << endl;
      }
      else if (fStatusLast == 1) {
	*oLog << "kStop" << endl;
      }
      else if (fStatusLast == 2) {
	*oLog << "kExit" << endl;
      }
      else if (fStatusLast == 3) {
	*oLog << "kFocus" << endl;
      }
      else if (fStatusLast == 4) {
	*oLog << "kSuspend" << endl;
      }
      else if (fStatusLast == 5) {
	*oLog << "kAbsorb" << endl;
      }
      else if (fStatusLast == 1) {
	*oLog << "kStop" << endl;
      }
      else {
	*oLog << " can't interpret fStatusLast" << endl;
      }
      *oLog << endl;
      
      pol->SetLineColor(2);
      pol->Draw();
      gPad->Update();
    }
  }

  return ray->IsFocused();
};
/********************** end of getCameraPhotonLocation *****************/

void GTDEMTelescope::printTelescope() {

  bool debug = false;
  if (debug) {
    *oLog << " -- GTDEMTelescope::printTelescope" << endl;
    *oLog << "      iPrtMode " << iPrtMode << endl;
  }

  if (iPrtMode > 0) {
    *oLog << "   in GTDEMTelescope::printTelescope() " << endl;
    *oLog << "      fF " << fF << endl;
    *oLog << "      fPlateScaleFactor " << fPlateScaleFactor << endl;
    *oLog << "      fAvgTransitTime   " << fAvgTransitTime << endl;
    *oLog << "      fRotationOffset   " << fRotationOffset << endl;
  }

  if (debug) *oLog << "exiting GTDEMTelescope::printTelescope" << endl; 
};
/********************** end of printTelescope *****************/

void GTDEMTelescope::drawTelescope(const int &option) {

  bool debug = false;
  gGeoManager = fManager;

  if (debug) {
    *oLog << "  -- GTDEMTelescope::drawTelescope" << endl; 
    *oLog << "       option: "  << option << endl;
    if (option > 2) {
      *oLog << "valid options are 0, 1, or 2 " << endl;
    }
    *oLog << "  -- Checking overlaps" << endl; 
    fManager->GetTopVolume()->CheckOverlaps();
    fManager->PrintOverlaps();
  }
  
  if ( (option == 0) || (option == 2) ){
    //gStyle->SetCanvasPreferGL(1);
    TCanvas *cTelescope = new TCanvas("cTelescope","cTelescope",300,300);
    cTelescope->cd();
    if (debug) *oLog << "   ready to draw: option " << option << endl;
    fManager->GetTopVolume()->Draw();//"ogl");
    if (debug) *oLog << "   finished drawing " << endl;
    //gGeoManager->GetTopVolume()->Draw("x3d");
  }
};
/********************** end of drawTelescope *****************/

void GTDEMTelescope::setPrintMode(ostream &oStr,const int prtMode) {
  bool debug = false;
  iPrtMode = prtMode;

  if (debug) {
    *oLog << " -- GTDEMTelescope::setPrintMode " << iPrtMode << endl;
  } 
};
/********************** end of setPrintMode *****************/

void GTDEMTelescope::setPhotonHistory(const string &rootFile,
                                       const string &treeName,
                                       const int &option) {
  bool debug = false;
  bPhotonHistoryFlag = true;

  historyFileName = rootFile;
  historyTreeName = treeName;
  iHistoryOption = option;
  
  if (debug) {
    *oLog << "  -- setPhotonHistory " << endl;
    *oLog << "        historyFileName / rootTree " <<  historyFileName 
          << "  " << historyTreeName << " /option " << option 
          << endl;
  }
  // add telID to historyFileName
  //   make string to insert
  stringstream osTmp;
  osTmp << "_Tel" << iTelID;
  string strInsert = osTmp.str();

  // find insertion point, append if . isn't found
  size_t idx = historyFileName.rfind('.');
  
  if (idx != string::npos) {
    historyFileName.insert(idx,strInsert);
    if (debug) *oLog << historyFileName << endl;
  }
  else {
    historyFileName = historyFileName + strInsert;
  }

  if (debug) {
    *oLog << "     opening photon history file / tree:  " 
          << historyFileName << " / " << historyTreeName << endl;
  }
  hisF = new TFile(historyFileName.c_str(),"RECREATE");
  hisT = new TTree(historyTreeName.c_str(),historyTreeName.c_str());

  makePhotonHistoryBranches();
  initializePhotonHistoryParms();
  
};
/********************** end of setPhotonHistory *****************/

void GTDEMTelescope::makePhotonHistoryBranches() {

  hisF->cd();
  bool debug = false;
  if (debug) {
    *oLog << "  -- makePhotonHistoryBranches " << endl;
  }
  hisT->Branch("status",&fStatusLast,"status/I");
  hisT->Branch("nPoints",&fNPoints,"nPoints/I");
  hisT->Branch("plateSFac",&fPlateScaleFactor10,"plateSFac/D");
  hisT->Branch("injectX",&fInitialInjectLoc[0],"injectX/D");
  hisT->Branch("injectY",&fInitialInjectLoc[1],"injectY/D");
  hisT->Branch("injectZ",&fInitialInjectLoc[2],"injectZ/D");
  hisT->Branch("injectDirX",&fphotonInjectDir[0],"injectDirX/D");
  hisT->Branch("injectDirY",&fphotonInjectDir[1],"injectDirY/D");
  hisT->Branch("xLast",&fLocLast[0],"xLast/D");
  hisT->Branch("yLast",&fLocLast[1],"yLast/D");
  hisT->Branch("zLast",&fLocLast[2],"zLast/D");
  hisT->Branch("xLastDir",&fDirLast[0],"xLastDir/D");
  hisT->Branch("yLastDir",&fDirLast[1],"yLastDir/D");
  hisT->Branch("zLastDir",&fDirLast[2],"zLastDir/D");
  hisT->Branch("timeLast",&fTimeLast,"timeLast/D");
};
/************************* end of makePhotonHistoryBranches *****/

void GTDEMTelescope::fillPhotonHistory() {

  bool debug = false;
  if (debug) {
    *oLog << "  -- fillPhotonHistory " << endl;
  }
  hisF->cd();
  hisT->Fill();
};
/************************* end of fillPhotonHistory *****/

void GTDEMTelescope::initializePhotonHistoryParms() {
  
  fPlateScaleFactor10 = fPlateScaleFactor * 10.0;
  bool debug = false;
  if (debug) {
    *oLog << "in initializePhotonHistoryParms" << endl;
    *oLog << "      " << fPlateScaleFactor << "  " << fPlateScaleFactor10 << endl;
  }
};
/************************* end of initializePhotonHistoryParms *****/

void GTDEMTelescope::writePhotonHistory() {

  bool debug = false;
  if (debug) {
    *oLog << "  -- in GTDEMTelescope::writePhotonHistory " << endl;
  }
  if (hisF != 0) {
    hisF->cd();  
    hisT->Write();
    // deleting the file will close the file;
    // the file owns the tree and will delete it
    SafeDelete(hisF);
    hisF = 0;
  }
 
};
/************************* end of writePhotonHistory *****/
void GTDEMTelescope::setReflCoeffMap(map<int, TGraph *> *mGr) {

  bool debug = false;
  if (debug) {
    *oLog << "  -- GTDEMTelescope::setReflCoeffMap" << endl;
    *oLog << "       mGr->size() " << mGr->size() << endl;
  }
  mGRefl = new map<int, TGraph *>;

  // make a copy of the map
  map<int, TGraph *>::iterator iter;
  for (iter = mGr->begin();iter != mGr->end(); iter++) {
    Int_t id = iter->first;
    if (debug) {
      *oLog << " building new graph " << iter->first << endl;
    }
    TGraph *tmpOld = iter->second;
    TGraph *tmpNew = new TGraph(*tmpOld);
    (*mGRefl)[id] = tmpNew;
  }
  if (debug) {
    *oLog << "      size of mGRefl " << mGRefl->size() << endl;
  }
};
/************************* end of setReflCoeffMap *****/

TGraph * GTDEMTelescope::makeReflectivityGraph(const Int_t &irefl) {

  bool debug = false;
  if (debug) {
    *oLog << "  -- GTDEMTelescope::makeReflectivityGraph" << endl;
    *oLog << "    size of mGRefl " << mGRefl->size() << endl;
  }
  Int_t id = irefl;
  TGraph *tmpNew;
  tmpNew = 0;

  map<int, TGraph *>::iterator iter;
  if ( (iter = mGRefl->find(irefl) ) != mGRefl->end() ) {
    TGraph *tmpOld = iter->second;
    tmpNew = new TGraph(*tmpOld);
    if (debug) {
      *oLog << "     ready to print TGraph: reflect curve = " << id << endl;
      tmpNew->Print();
    }

  }
  return tmpNew;
};
/************************* end of makeReflectivityGraph *****/

void GTDEMTelescope::initialize() {
  bool debug = false;

  if (debug) {
    *oLog << "  -- GTDEMTelescope::initialization " << endl;
  }

  fManager = 0;
  iPrtMode = 0;

  ray = 0;
  hisF = 0;
  hisT = 0;

  iTelID = 0;
  iStdID = 0;

  fTX = 15.0;  // set to 15 later, after confirming code
  fTY = 15.0;
  fTZ = 15.0;


  fAvgTransitTime     = 0.0;
  fPlateScaleFactor   = 0.0;
  fPlateScaleFactor10 = 0.0;
  fphotWaveLgt        = 0.0;
  fphotonToTopVolTime = 0.0;
  fInjectTime         = 0.0; 
  fInjectLambda       = 0.0;
  fF       = 0.0; 

  historyFileName = "";
  fTimeLast      = 0.0;
  iHistoryOption = 0;
  fStatusLast    = 0;
  fNPoints       = 0;
  fRotationOffset= 0.0;

  bPhotonHistoryFlag = false;

  bRayPlotModeFlag = false;
  eRayPlotType     = FOCUSONLY;  

  eTelType = TDEM;

  // set gl picture colors: black/1, red/2/, green/3,blue/4, brown/28/49
  iMirrorColor   = 38;
  iObscurationColor   = 1;

  for (int i = 0;i<3;i++) {
    fphotonInjectLoc[i] = 0.0;
    fphotonInjectDir[i] = 0.0;
    fInjectLoc[i] = 0.0;
    fInjectDir[i] = 0.0;
    fLocLast[i]   = 0.0;
    fDirLast[i]   = 0.0;
    fInitialInjectLoc[i] = 0.0;
  }
};
/************************* end of writePhotonHistory *****/

void GTDEMTelescope::CloseGeometry() {

  bool debug = false;
  if (debug) {
    *oLog << "  -- GTDEMTelescope::CloseGeometry" << endl;
  }
  gGeoManager = fManager;
  fManager->CloseGeometry();
}; 
/************************* end of CloseGeometry *****/
