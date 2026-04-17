#include "Camera.h"

using namespace IFHB;

Camera::Camera(){
	dCameraWidth = 0;
	dCameraHeight = 0;
	dCameraAngularWidth = 0;
	dCameraAngularHeight = 0;
}
Camera::Camera(ReadConfiguration *readConfig){
	dCameraWidth = readConfig->fCameraWidth;
	dCameraHeight = readConfig->fCameraHeight;
	dCameraAngularWidth = readConfig->fCameraAngularWidth;
	dCameraAngularHeight = readConfig->fCameraAngularHeight;
	
}
Camera::~Camera(){
	
}

void Camera::SetViewingLimit(double dPointing){
	dCameraVViewLimUp = GetVerticalViewingAngle(dCameraAngularHeight/2.0,dPointing);
	dCameraVViewLimLow = GetVerticalViewingAngle(-1.0*dCameraAngularHeight/2.0,dPointing);
	dCameraHViewLimUp = GetHorizontalViewingAngle(dCameraAngularWidth/2.0, dPointing);
	dCameraHViewLimLow = GetHorizontalViewingAngle(-1.0*dCameraAngularWidth/2.0, dPointing);
}
double Camera::GetVerticalViewingAngle(double dFov, double dPointing){
	double dPointingRads = (dFov+dPointing)*TMath::Pi()/180.0;

	return dPointingRads;
}
double Camera::GetHorizontalViewingAngle(double dFov, double dPointing){
	double dPointingRads = (dFov+dPointing)*TMath::Pi()/180.0;

	return dPointingRads;
}
double Camera::GetGroundProjection(double phi1, double phi2, double theta1, double theta2){
	double area = TMath::Power(Constant::REarth,2.0)*Utilities::SphericalSolidAngle(phi1,phi2,theta1,theta2);

	return area;
}
double Camera::GetPerpendicularProjection(double alpha, double altitude){
	double verticalFOV = dCameraAngularHeight*TMath::Pi()/180.0;
	double horizontalFOV = dCameraAngularWidth*TMath::Pi()/180.0;;
	double distanceLowerFOV = Utilities::DistanceToSurface(alpha, altitude);
	double width = distanceLowerFOV*std::sin(verticalFOV/2.0);
	double length = distanceLowerFOV*std::cos(verticalFOV/2.0)*std::tan(horizontalFOV/2.0);

	return 4.0*length*width;
}
std::vector<double> Camera::GetVerticalViewingLimits(){
	std::vector<double> vertLim = {dCameraVViewLimLow,dCameraVViewLimUp};
	return vertLim;
}
std::vector<double> Camera::GetHorizontalViewingLimits(){
	std::vector<double> horLim = {dCameraHViewLimLow, dCameraHViewLimUp};
	return horLim;
}

double Camera::GetAngularHeight(){
	return dCameraAngularHeight*TMath::Pi()/180.0;;
}

double Camera::GetAngularWidth(){
	return dCameraAngularWidth*TMath::Pi()/180.0;;
}

double Camera::CalculateLongitudeLimit(double altitude, double alphaVertical, double alphaHorizontal, double viewingLatLim, double fovBoost){

	
	double rHorizontal = Utilities::CosineLaw(altitude/1000.0, Constant::REarth,alphaHorizontal);
	double rVertical = Utilities::CosineLaw(altitude/1000.0, Constant::REarth,alphaVertical);
	double lineSegment = Utilities::CosineLaw(rHorizontal,rVertical,((dCameraAngularWidth/2.0)+fovBoost)*(TMath::Pi()/180.0));
	double cosLong = 1.0-(lineSegment*lineSegment)/(2.0*TMath::Power((Constant::REarth*TMath::Sin(viewingLatLim)),2.0));

	return TMath::ACos(cosLong);
}