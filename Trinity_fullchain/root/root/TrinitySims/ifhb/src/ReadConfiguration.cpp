#include "ReadConfiguration.h"

using namespace IFHB;
ReadConfiguration::ReadConfiguration(std::string cfgFile){
	std::ifstream *cfgFileStream = new std::ifstream(cfgFile.c_str());
	std::string iline;

	fTelAltitude = 0;
	fTelLatitude = 0;
	fTelLongitude = 0;
	fTelPointingAngle = 0;
	fCameraWidth = 0;
	fCameraHeight = 0;
	fCameraAngularWidth = 0;
	fCameraAngularHeight = 0;
	iNShowers = 0;
	bIsPointSource = 0;
	fPointSourceZe = -1;
	fPointSourceAz = -1;

    dESampleStart=1e6;
    dESampleEnd=1e12;


	if(*cfgFileStream){
		while(getline(*cfgFileStream, iline)){
			if (iline.substr(0,1) == "*"){
				ReadLine(iline, cfgFileStream);
			} 
		}
	}
	else{
		std::cout<<"Read Configuration error: config file not found: "<< cfgFile <<std::endl;
		exit(-1);
	}

}

ReadConfiguration::~ReadConfiguration(){

}
std::string ReadConfiguration::BoolToStringAction(Bool_t stage_status){
	std::string statusText;
	if (stage_status){
		statusText = "YES";
	}
	else{
		statusText = "NO";
	}

	return statusText;
}
void ReadConfiguration::ReadCommandLine(int argc, char **argv){
	std::ifstream *inFileStream = NULL;
	std::string iline;
	for(int i = 1; i<argc; i++){
		iline="* ";
		if(i+1<argc){
			iline+=argv[i];
			iline+= " ";
			iline+= argv[++i];
		}
		else{
			std::cerr<<"Incomplete option passed in command line"<<std::endl;
		}
		ReadLine(iline, inFileStream);
	}
	delete inFileStream;

}
void ReadConfiguration::ReadLine(std::string iline, std::ifstream *cfgFileStream){
	std::string i_dump;
	std::string status;
	std::istringstream lineStream(iline);
	
	if (iline.find("TELALT") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>fTelAltitude;

		std::cout<<"Observatory height: "<< fTelAltitude<<std::endl;
	}
	else if (iline.find("TELPOINTING") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>fTelPointingAngle;

		std::cout<<"Observatory pointing: "<< fTelPointingAngle<<std::endl;
	}
	else if (iline.find("TELFOCALLENGTH") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>fTelFocalLength;

		std::cout<<"Telescope focal length: "<< fTelFocalLength<<std::endl;
	}
	else if (iline.find("TELLAT") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>fTelLatitude;

		std::cout<<"Observatory Latitude: "<< fTelLatitude<<std::endl;
	}
	else if (iline.find("TELLONG") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>fTelLongitude;

		std::cout<<"Observatory Longitude: "<< fTelLongitude<<std::endl;
	}
	else if (iline.find("TELCOLLECTIONAREA") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>fTelCollectionArea;

		std::cout<<"Telescope Collection Area: "<< fTelCollectionArea<<std::endl;
	}
	else if (iline.find("CAMANGULARWIDTH") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>fCameraAngularWidth;

		std::cout<<"Camera angular width: "<< fCameraAngularWidth<<std::endl;
	}
	else if (iline.find("CAMANGULARHEIGHT") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>fCameraAngularHeight;

		std::cout<<"Camera angular height: "<< fCameraAngularHeight<<std::endl;
	}
	else if (iline.find("CAMERAWIDTH") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>fCameraWidth;

		std::cout<<"Camera width: "<< fCameraWidth<<std::endl;
	}
	else if (iline.find("CAMERAHEIGHT") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>fCameraHeight;

		std::cout<<"Camera height: "<< fCameraHeight<<std::endl;
	}
	else if (iline.find("NSHOWERS") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>iNShowers;

		std::cout<<"Number of Showers: "<< iNShowers<<std::endl;
	}
	else if (iline.find("ISPOINTSOURCE") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>bIsPointSource;

		status = BoolToStringAction(bIsPointSource);

		std::cout<<"Point Source: "<< status<<std::endl;
	}
	else if (iline.find("ZENITHANGLE") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>fPointSourceZe;

		std::cout<<"Point source zenith angle: "<< fPointSourceZe<<std::endl;
	}
	else if (iline.find("AZIMUTHANGLE") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>fPointSourceAz;

		std::cout<<"Point source azimuth angle: "<< fPointSourceAz<<std::endl;
	}
	else if (iline.find("COLLECTIONSPHERERADIUS") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>fTelCollectionSphere;

		std::cout<<"Telescope collection sphere radius: "<< fTelCollectionSphere<<std::endl;
	}
	else if (iline.find("NEUTRINOENERGY") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>dNeutrinoEnergy;

		std::cout<<"Neutrino energy: "<< dNeutrinoEnergy<<std::endl;
	}else if (iline.find("COSMICRAYENERGY") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>dCosmicRayEnergy;

		std::cout<<"Cosmic Ray energy: "<< dCosmicRayEnergy<<std::endl;
	}
	else if(iline.find("ENERGYSPECTRUM") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>bIsNuEnergySpectrum;

		std::cout<<"Will sample from neutrino Energy Spectrum: "<< BoolToStringAction(bIsNuEnergySpectrum)<<std::endl;

	}
	else if (iline.find("FILEEMERGENCEANGLES") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>sFileNameEmergenceAngles;

		std::cout<<"Emergence angle file: "<< sFileNameEmergenceAngles<<std::endl;
	}
	else if (iline.find("FILEVIEWINGANGLEINVLUT") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>sFileNameViewingAngleInverse;

		std::cout<<"Emergence angle file: "<< sFileNameViewingAngleInverse<<std::endl;
	}else if (iline.find("FILEVIEWINGANGLELUT") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>sFileNameViewingAngle;

		std::cout<<"Emergence angle file: "<< sFileNameViewingAngle<<std::endl;
	}
	else if (iline.find("FILEENERGYFRACTIONS") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>sFileNameEnergyFractions;

		std::cout<<"Tau energy fraction file: "<< sFileNameEnergyFractions<<std::endl;
	}
	else if (iline.find("FILEENERGYDISTRIBUTION") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>sFileNameEnergyDistribution;

		std::cout<<"Tau energy distribution file: "<< sFileNameEnergyDistribution<<std::endl;
	}
	else if (iline.find("FILEEXITPROB") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>sFileNameProbExit;

		std::cout<<"Tau exit probability file: "<< sFileNameProbExit<<std::endl;
	}
	else if (iline.find("FILENUENERGY") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>sFileNameNuEnergy;

		std::cout<<"Tau exit probability file: "<< sFileNameProbExit<<std::endl;
	}
	else if (iline.find("FILESHOWERNMAX") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>sFileNameShowerNMax;

		std::cout<<"Shower Photons file: "<< sFileNameShowerNMax<<std::endl;
	}
	else if (iline.find("FILESHOWERXMAX") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>sFileNameShowerXMax;

		std::cout<<"Shower XMax file: "<< sFileNameShowerXMax<<std::endl;
	}
	else if (iline.find("FILESHOWERWIDTH") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>sFileNameShowerWidth;

		std::cout<<"Shower width file: "<< sFileNameShowerWidth<<std::endl;
	}
	else if (iline.find("FILEXSECTION") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>sFileNameXSection;

		std::cout<<"Particle cross section file: "<< sFileNameXSection<<std::endl;
	}
	else if(iline.find("ANGLEINCREASE") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>dAngleIncrease;

		std::cout<<"Area enhanced: "<< dAngleIncrease<<std::endl;

	}
	else if(iline.find("BSMMEANLIFE") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>dBSMMeanLifeMult;

		std::cout<<"BSM Factor: "<< dBSMMeanLifeMult<<std::endl;

	}
    else if(iline.find("ESAMPLESTRT") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>dESampleStart;

		std::cout<<"Energy Min [GeV]: "<< dESampleStart<<std::endl;

	}
    else if(iline.find("ESAMPLEEND") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>dESampleEnd;

		std::cout<<"Energy Max [GeV]: "<< dESampleEnd<<std::endl;

	}
	else if(iline.find("DRAW") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>bIsDraw;

		std::cout<<"Will Plot: "<< BoolToStringAction(bIsDraw)<<std::endl;

	}
	else if(iline.find("ANGLESHOWERINCREASE") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>dAngleShowerIncrease;

		std::cout<<"Angles enhanced: "<< dAngleShowerIncrease<<std::endl;
	}
	else if(iline.find("ISBELOWLIMB") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>bIsBelowLimb;

		std::cout<<"Below Limb: "<< BoolToStringAction(bIsBelowLimb)<<std::endl;
	}
	else if(iline.find("GAMMAINDEX") < iline.size()){
		lineStream>>i_dump; lineStream>>i_dump;
		lineStream>>dSpectralIndex;

		std::cout<<"Spectral index: "<< dSpectralIndex<<std::endl;
	}
}