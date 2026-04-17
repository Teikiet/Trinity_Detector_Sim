#include "MrCosmic.h"

using namespace IFHB;

MrCosmic::MrCosmic(){
	distOFI = 0;
	xSection = 0;
	gammaIndex=0;
	pos=std::vector<double>(3,0);
	angleAlpha=0;
	angleBeta=0;
	maxDist=0;

}

MrCosmic::MrCosmic(ReadConfiguration *readConf){
	distOFI=0;
	xSection=0;
	gammaIndex=readConf->dSpectralIndex;
	pos=std::vector<double>(3,0);
	angleAlpha=0;
	angleBeta=0;
	maxDist=0;
	energy=readConf->dCosmicRayEnergy;
	LoadXSectionFromFile(readConf->sFileNameXSection);
}

MrCosmic::~MrCosmic(){
	delete grXSection;
}

void MrCosmic::SetRandomEngine(TRandom3 *rand){
	this->rand = rand;
}

void MrCosmic::SampleCosmicSpectrum(double eStart, double eEnd){
	double diff = TMath::Power(eEnd,1.0-gammaIndex) - TMath::Power(eStart,1.0-gammaIndex);
	double ranSample = rand->Uniform()*diff + TMath::Power(eStart,1.0-gammaIndex);

	energy = TMath::Power(ranSample,1.0/(1.0-gammaIndex));
}

void MrCosmic::RunSimulation(){
	xSection = grXSection->Eval(energy*1e9)*Constant::mbToCm2;
	double meanInteractionLength = 1.0/(Constant::avogadroN * xSection/Constant::atmosphericMWeight);
	do{
		distOFI = -1.0*meanInteractionLength*TMath::Log(rand->Uniform());
	}while(distOFI>maxDist);
	
}

double MrCosmic::GetDistOFI(){
	return distOFI;
}

void MrCosmic::LoadXSectionFromFile(std::string filename){
	grXSection = new TGraph(filename.c_str(),"%lg %lg",",");
}

void MrCosmic::SetPosition(double polar, double phi){
	pos[0] = (Constant::REarth+Constant::atmosphericTop) * TMath::Sin(polar)*TMath::Cos(phi);
	pos[1] = (Constant::REarth+Constant::atmosphericTop) * TMath::Sin(polar)*TMath::Sin(phi);
	pos[2] = (Constant::REarth+Constant::atmosphericTop) * TMath::Cos(polar);

}

void MrCosmic::SetEnergy(double energy){
	this->energy = energy;
}

std::vector<double> MrCosmic::GetPosition(){
	return pos;
}

double MrCosmic::GetEnergy(){
	return energy;
}

void MrCosmic::SetMaxDist(std::vector<double> telPos, double anglePositions, double angleTraj){
	double len = Utilities::CosineLaw(Utilities::VectorMagnitude(telPos),Utilities::VectorMagnitude(this->pos),anglePositions);
	maxDist = Utilities::ExpAtmColumnDepth(len,angleTraj,100,Constant::atmosphericTop);
}