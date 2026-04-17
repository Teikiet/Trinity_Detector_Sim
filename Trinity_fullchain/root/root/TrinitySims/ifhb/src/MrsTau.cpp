#include "MrsTau.h"

using namespace IFHB;



MrsTau::MrsTau(){
	probExit = 0;
	probDecay = 0;
	eNu = 0;
	eTau = 0;

	decayLengthFunction = new TF1("decayLengthFunction","TMath::Exp(-1.0*x/[0]) ",0,1e5);
}
MrsTau::MrsTau(ReadConfiguration *readConf){
	eNu = std::pow(10.0,readConf->dNeutrinoEnergy);
	probEnergyDistribution = new TGraph();
	decayLengthFunction = new TF1("decayLengthFunction","TMath::Exp(-1.0*x/[0]) ",0,1e5);
	

	showerNMax = new TGraph(readConf->sFileNameShowerNMax.c_str(),"%lg %lg",",");
	showerXMax = new TGraph(readConf->sFileNameShowerXMax.c_str(),"%lg %lg",",");
	showerWidth = new TGraph(readConf->sFileNameShowerWidth.c_str(),"%lg %lg",",");

	bsmFactor = readConf->dBSMMeanLifeMult;
}

MrsTau::~MrsTau(){
	delete probExitSurface;
	delete showerNMax;
	delete showerXMax;
	delete showerWidth;
	delete decayLengthFunction;
	delete rand;
	delete probEnergyDistribution;
}

double MrsTau::DecayLengthFunction(double *x, double *par){
	double meanDecayLength = par[0];
	return (TMath::Exp(-1.0*x[0]/meanDecayLength));
}

void MrsTau::LoadPExitFromFile(std::string fileNameProb, std::string fileNameEmergence, std::string fileNameEnergy){

}

double MrsTau::GetExitProbability(double angle, double energy){
	return 0;
}

std::string MrsTau::PrintCHASMEvent(){

	std::string outputInfo;

	outputInfo = std::to_string(eShower)+" "+std::to_string(nMax)+" "+std::to_string(xMax)+" "+std::to_string(x0)+" "+std::to_string(lambda);

	return outputInfo;


}

void MrsTau::SetNeutrinoEnergy(double neutrinoEnergy){
	eNu = neutrinoEnergy;
}

double MrsTau::GetNeutrinoEnergy(){
	return eNu;
}


void MrsTau::RunSimulation(double angleEmergence){
	double ranSample;
	do{
		ranSample  = rand->Uniform();
		//eTau = probEnergyDistribution->Eval(ranSample)*eNu;
		eTau = 0.8*eNu;
	}while(eTau == 0);
	
	double gamma = eTau/Constant::tauMass;
	double beta = TMath::Sqrt(1.0-(1.0/(gamma*gamma)));

	double meanDecayLength = gamma*beta*Constant::speedOfLight*Constant::tauMeanLife*bsmFactor;
	eShower = 0.5*eTau;
	//nPhotons = showerNPhotons->Eval(eTau*1.0e9); //Converts from GeV to eV
	//use Heitler model to approx. # charged particles at E_crit
	nMax = 0.31/TMath::Sqrt(log(eShower*1e9/83e6))*eShower*1e9/83e6;
	xMax = showerXMax->Eval(eTau*1.0e9);
	//do{
	decayLength = -1.0*meanDecayLength*TMath::Log(rand->Uniform());
	x0 = Utilities::ExpAtmColumnDepth(decayLength,angleEmergence,100);
	xMax += x0;
	//}while(x0>=xMax);
	//lambda = showerWidth->Eval(eTau*1.0e9);
	//lambda = (9.0/32.0)*(1.0/TMath::Log(2.0))*lambda*lambda/(xMax-x0);
	//using lambda = 70 for now, since we are unsure where Oscar gets this formulation from
	lambda = 70;
		
}

void MrsTau::SetTauEnergy(double tauEnergy){
	eTau = tauEnergy;
}

double MrsTau::GetTauEnergy(){
	return eTau;
}


double MrsTau::GetDecayLength(){
	return decayLength;
}

void MrsTau::SetTauEnergyDistributionInverse(std::vector<double> energyFraction, std::vector<double> prob){
	probEnergyDistribution->Set(0);

	for(int i = 0; i<energyFraction.size(); i++){
		probEnergyDistribution->SetPoint(probEnergyDistribution->GetN(),prob[i],energyFraction[i]);
	}
}

double MrsTau::SampleNeutrinoSpectrum(double eStart, double eEnd){

    double product = eStart*eEnd;
    double diff = eEnd - eStart;
    double quot = eStart/eEnd;


	double randEnergy = ((product)/diff)*((1.0-quot)/(1.0-(1.0-quot)*rand->Uniform()));
	double decade = TMath::Log10(randEnergy);
	double roundDecade = Utilities::RoundToQuarterDecade(decade);

	eNu = TMath::Power(10.0, (roundDecade));

	return roundDecade;


}

void MrsTau::SetRandomEngine(TRandom3 *rand){
	this->rand = rand;
}

double MrsTau::GetXMax(){
	return xMax;
}

double MrsTau::GetNMax(){
	return nMax;
}

double MrsTau::GetX0(){
	return x0;

}

double MrsTau::GetLambda(){
	return lambda;
}