#include "PasteToMachine.h"

using namespace IFHB;

PTM::PTM(){
	dNeutrinoEnergy = 0;
	dAreaSimulated = 0;
	dPAreaSimulated = 0;
	dHorizonAngle = 0;

	vTelescopePosition = std::vector<double>(3,0);
	vTelescopePointing = std::vector<double>(3,0);
	vViewingPolar = std::vector<double>(2,0);
	vViewingLong = std::vector<double>(2,0);

	vRotMatrix = std::vector<std::vector<double>>(3,std::vector<double>(3,0));

	bIsPointSource = false;
	dSourceAz = -1.0;
	dSourceZe = -1.0;

	dEmergenceBeta = 0;
	dEmergenceAz = 0;
	dEShower = 0;
	dETau = 0;
	dXMax = 0;
	dX0 = 0;
	dLambda = 0;
	dPExit = 0;

	vShowerLocation = std::vector<double>(3,0);

	dOffsetRot = 0;
	dOffsetTilt = 0;
	dCollectionSphereR = 0;

}

PTM::~PTM(){
	
}

std::string PTM::GetOutputString(){
	std::string outStr = "";

	outStr += GetStrPolarAngle() + " ";
	outStr += GetStrAzimuthAngle() + " ";
	outStr += GetStrCHASMTelPos() + " ";
	outStr += GetStrCollectionSphereR() + " ";
	outStr += GetStrXMax() + " ";
	outStr += GetStrNMax() + " ";
	outStr += GetStrX0() + " ";
	outStr += GetStrLambda() + " ";
	outStr += GetStrEShower() + " ";
	outStr += GetStrGrOpTelPos() + " ";
	outStr += GetStrGrOpRot();

	return outStr;

}

std::string PTM::GetEASCSString(){
	std::string outStr = "";
	outStr += GetStrEShower() + " ";
	outStr += GetStrX0() + " ";

	return outStr;
}

std::string PTM::GetStrPolarAngle(){
	return std::to_string((90.0 - dEmergenceBeta)*TMath::Pi()/180.0);
}

std::string PTM::GetStrAzimuthAngle(){
	return std::to_string(dEmergenceAz*TMath::Pi()/180.0);
}

std::string PTM::GetStrCHASMTelPos(){
	std::string posStr;
	std::vector<double> pos = Utilities::VectorSubtraction(vShowerLocation,vTelescopePosition);
	pos = Utilities::VectorRotation(pos, vRotMatrix);

	posStr = std::to_string(pos[0]*1e3) +" " + std::to_string(pos[1]*1e3)+" "+std::to_string(pos[2]*1e3);

	return posStr;
}

std::string PTM::GetStrCollectionSphereR(){
	return std::to_string(dCollectionSphereR);
}

std::string PTM::GetStrXMax(){
	return std::to_string(dXMax);
}

std::string PTM::GetStrNMax(){
	return std::to_string(dNMax);
}

std::string PTM::GetStrX0(){
	return std::to_string(dX0);
}

std::string PTM::GetStrLambda(){
	return std::to_string(dLambda);
}

std::string PTM::GetStrEShower(){
	return std::to_string(dEShower);
}

std::string PTM::GetStrGrOpTelPos(){
	std::string posStr;

	posStr = std::to_string(0) +" " + std::to_string(0)+" "+std::to_string(-2.68);

	return posStr;
}

std::string PTM::GetStrGrOpRot(){
	std::string rot;
	std::vector<double> pos = Utilities::VectorRotation(vTelescopePointing, vRotMatrix);
	double cosZ =  pos[2]/(Utilities::VectorMagnitude(pos));
	double relPointingAngle = TMath::ACos(cosZ);
	rot = std::to_string(90)+ " " + std::to_string(-1.0*(180-(relPointingAngle)*180/TMath::Pi()));
	return rot;
}