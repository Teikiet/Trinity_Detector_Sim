#include "Telescope.h"

using namespace IFHB;
Telescope::Telescope(){
	std::vector<double> pos = std::vector<double>(3);
	std::vector<double> vPointing = std::vector<double>(3);
	vPointingCenter=std::vector<double>(3,0);
	dAltitude = 0;
	dPointingAngle = 0;
	dFocalLength = 0;
	dLat = 0;
	dLong = 0;
	dCollectionArea = 0;
	dCollectionSphereRadius = 0;
	pos = std::vector<double>(3,0);
}
Telescope::Telescope(ReadConfiguration *readConfig){
	pos = std::vector<double>(3,0);
	vPointing = std::vector<double>(3,0);
	vPointingCenter=std::vector<double>(3,0);
	dAltitude = readConfig->fTelAltitude;
	dPointingAngle = readConfig->fTelPointingAngle;
	dLat = readConfig->fTelLatitude;
	dLong = readConfig->fTelLongitude;
	dCollectionArea = readConfig->fTelCollectionArea;
	dCollectionSphereRadius = readConfig->fTelCollectionSphere;
	dFocalLength = readConfig->fTelFocalLength;
}
Telescope::~Telescope(){

}
void Telescope::SetPosition(){
	pos[0] = (IFHB::Constant::REarth+dAltitude/1000.0) * TMath::Sin(this->GetLatitude())*TMath::Cos(this->GetLongitude());
	pos[1] = (IFHB::Constant::REarth+dAltitude/1000.0) * TMath::Sin(this->GetLatitude())*TMath::Sin(this->GetLongitude());
	pos[2] = (IFHB::Constant::REarth+dAltitude/1000.0) * TMath::Cos(this->GetLatitude());
}

void Telescope::SetPointingVector(){
	

	double angleZ = dPointingAngle*TMath::Pi()/180 - this->GetLatitude();

	vPointing[0] =  -1.0*TMath::Sin(angleZ)*TMath::Cos(this->GetLongitude());
	vPointing[1] = -1.0*TMath::Sin(angleZ)*TMath::Sin(this->GetLongitude());


	vPointing[2] = TMath::Cos(angleZ);

}

double Telescope::CalculateAngleOffset(){
	double offset = dFocalLength*TMath::Tan(dPointingAngle*TMath::Pi()/180.0);
	return offset*1e-3;
}

std::vector<double> Telescope::GetPosition(){
	return pos;
}

double Telescope::GetPointingAngle(){
	return dPointingAngle;
}

double Telescope::GetAltitude(){
	return dAltitude;
}

double Telescope::GetLatitude(){
	return dLat*TMath::Pi()/180.0;
}

double Telescope::GetLongitude(){
	return dLong*TMath::Pi()/180.0;
}

double Telescope::GetCorrespondingLat(double lat){
	return TMath::ACos(TMath::Cos(lat)*(Constant::REarth/(Constant::REarth+dAltitude/1000.0)));
}

std::vector<double> Telescope::GetTelescopePointing(){
	return vPointing;
}

std::vector<double> Telescope::GetPointingCenter(){
	return vPointingCenter;
}

void Telescope::SetPointingCenter(double lat, double lon){
	vPointingCenter[0] = (IFHB::Constant::REarth) * TMath::Sin(lat)*TMath::Cos(lon);
	vPointingCenter[1] = (IFHB::Constant::REarth) * TMath::Sin(lat)*TMath::Sin(lon);
	vPointingCenter[2] = (IFHB::Constant::REarth) * TMath::Cos(lat);
}