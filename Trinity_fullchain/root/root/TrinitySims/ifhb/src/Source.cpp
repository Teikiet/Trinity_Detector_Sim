#include "Source.h"

using namespace IFHB;

Source::Source(){
	zenithAngle = -1;
	azimuthAngle = -1;
	betaEmergenceAngle = 0;
	phiEmergenceAngle = 0;

	latitude = 0;
	longitude = 0;
	pos = std::vector<double>(3,0);
	rotM = std::vector<std::vector<double>>(3,std::vector<double>(3,0));
	shwrDir = std::vector<double>(3,0);
}

Source::Source(ReadConfiguration *readConfig){
	pos = std::vector<double>(3,0);
	rotM = std::vector<std::vector<double>>(3,std::vector<double>(3,0));
	shwrDir = std::vector<double>(3,0);
	if(readConfig->bIsPointSource){
		zenithAngle = readConfig->fPointSourceZe - readConfig->fTelLatitude;
		azimuthAngle = readConfig->fPointSourceAz;// - readConfig->fTelLongitude;
		CalculateShowerDir();
	}
	else{
		zenithAngle = -1;
		azimuthAngle = -1;
	}
}

Source::~Source(){
	delete rand;
}

void Source::SetRandomEngine(TRandom3 *rand){
	this->rand = rand;
}

void Source::RunGeographicSimulation(double latLimLow, double latLimHigh, double longLimLow, double longLimHigh){

	latitude = rand->Uniform(latLimLow,latLimHigh);
	longitude = rand->Uniform(longLimLow,longLimHigh);

	CalculateRotMatrix();
}

void Source::RunDiffuseSimulation(double betaLimLow, double betaLimHigh, double phiLimLow, double phiLimHigh){
	
	betaEmergenceAngle = rand->Uniform(betaLimLow,betaLimHigh);
	phiEmergenceAngle = rand->Uniform(phiLimLow, phiLimHigh);

	if(phiEmergenceAngle<0){
		phiEmergenceAngle=2*TMath::Pi()+phiEmergenceAngle;
	}
	while(phiEmergenceAngle>TMath::Pi()*2){
		phiEmergenceAngle -= TMath::Pi()*2;
	}
}


void Source::CalculateShowerToTel(std::vector<double> telPos){
	shwrDir = Utilities::VectorSubtraction(pos, telPos);
}

void Source::CalculateShowerDir(){
	double elevDir;
	double longitudeDir;

	elevDir = this->GetZenithAngle() - TMath::Pi()/2.0;
	
	longitudeDir = this->GetAzimuthAngle() - TMath::Pi();

	shwrDir[0] = TMath::Cos(elevDir)*TMath::Cos(longitudeDir);
	shwrDir[1] = TMath::Cos(elevDir)*TMath::Sin(longitudeDir);
	shwrDir[2] = TMath::Sin(elevDir);

	
}

void Source::CalculateElevationAngle(){
	double elevAngle = 0;

	std::vector<double> rotShwrDir = std::vector<double>(3,0);
	rotShwrDir = Utilities::VectorRotation(shwrDir, rotM);

	double sinElev = rotShwrDir[2]/Utilities::VectorMagnitude(rotShwrDir);
	elevAngle = TMath::ASin(sinElev);

	betaEmergenceAngle = elevAngle;

}

void Source::CalculateAzimuthAngle(){
	double azAngle = 0;
	std::vector<double> rotShwrDir = std::vector<double>(3,0);
	rotShwrDir = Utilities::VectorRotation(shwrDir, rotM);
	double cosAz = rotShwrDir[0]/(Utilities::VectorMagnitude(rotShwrDir)*TMath::Cos(betaEmergenceAngle));

	azAngle = TMath::ACos(cosAz);

	if(rotShwrDir[1] < 0){
		azAngle = 2.0*TMath::Pi() - azAngle;
	}
	phiEmergenceAngle = azAngle;

}

void Source::CalculateRotMatrix(){
	rotM[0][0] = -1.0*TMath::Cos(-1.0*latitude)*TMath::Cos(-1.0*longitude);
	rotM[0][1] = TMath::Cos(-1.0*latitude)*TMath::Sin(-1.0*longitude);
	rotM[0][2] = -1.0*TMath::Sin(-1.0*latitude);

	rotM[1][0] = -1.0*TMath::Sin(-1.0*longitude);
	rotM[1][1] = -1.0*TMath::Cos(-1.0*longitude);
	rotM[1][2] = 0.0;

	rotM[2][0] = -1.0*TMath::Sin(-1.0*latitude)*TMath::Cos(-1.0*longitude);
	rotM[2][1] = TMath::Sin(-1.0*latitude)*TMath::Sin(-1.0*longitude);
	rotM[2][2] = TMath::Cos(-1.0*latitude);
}

double Source::GetZenithAngle(){
	return zenithAngle*TMath::Pi()/180.0;
}

double Source::GetAzimuthAngle(){
	return azimuthAngle*TMath::Pi()/180.0;
}


void Source::SetPosition(){
	pos[0] = (IFHB::Constant::REarth) * TMath::Sin(latitude)*TMath::Cos(longitude);
	pos[1] = (IFHB::Constant::REarth) * TMath::Sin(latitude)*TMath::Sin(longitude);
	pos[2] = (IFHB::Constant::REarth) * TMath::Cos(latitude);
}

std::vector<double> Source::GetPosition(){
	SetPosition();
	return pos;
}

void Source::SetEmergenceAngles(double betaEmergence, double phiEmergence){
	betaEmergenceAngle = betaEmergence;
	phiEmergenceAngle = phiEmergence;
}

double Source::GetLatitude(){
	return latitude;
}

double Source::GetLongitude(){
	return longitude;
}

double Source::GetEmergenceBeta(){
	return betaEmergenceAngle;
}

double Source::GetEmergenceAzimuth(){
	return phiEmergenceAngle;
}

std::vector<std::vector<double>> Source::GetRotationMatrix(){
	return  rotM;
}