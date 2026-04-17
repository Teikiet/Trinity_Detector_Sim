#include <iostream>
#include <fstream>
#include <TMath.h>
#include <TGraph.h>
#include <TCanvas.h>

int AngleLUTGeneration(std::string fileOut, double alt){

	//std::ofstream outFile(fileOut);

	double R_EARTH = 6371;//km
	double propAlt = (R_EARTH+alt)/R_EARTH;

	double horizon_alpha = TMath::ACos(1/propAlt)*180/TMath::Pi();//deg
	double step = 0.001; //deg
	double nSamples = (horizon_alpha - 0)/step;
	std::cout<<nSamples<<std::endl;
	double MAX_TILT_alpha = -10.0*TMath::Pi()/180.0;


	std::vector<double> alpha = std::vector<double>((int)nSamples,0);
	std::vector<double> beta = std::vector<double>((int)nSamples,0);
	double sin2Beta = 0;
	// //outFile<<"#Angle from vertical(Rad) Alpha(Rad)"<<endl;
	TGraph *gr = new TGraph();
	TGraph *grEm = new TGraph();
	double emAngle = 0;
	horizon_alpha=0;
	for(int i = 0 ; i<(int)nSamples; i++){
		alpha[i]=step*(i+1)*TMath::Pi()/180.0;
		sin2Beta = (0.5)*(1-TMath::Cos(2*alpha[i]))/(1+propAlt*propAlt-2*propAlt*TMath::Cos(alpha[i]));
		beta[i] = TMath::ASin(TMath::Sqrt(sin2Beta));
		emAngle =90- alpha[i]*180/3.1416 - beta[i]*180/3.1416;
		//cout<<alpha[i]*180/TMath::Pi()<<" "<<beta[i]*180/TMath::Pi()<<endl;
		//outFile<<std::setprecision(12)<<alpha[i]<<" "<<std::setprecision(12)<<beta[i]<<endl;
		gr->SetPoint(gr->GetN(),(beta[i]*180.0/3.1416)-84.2,(alpha[i]*180.0/3.1416)-horizon_alpha);
		grEm->SetPoint(gr->GetN()-1, alpha[i]*180/3.1416-horizon_alpha, emAngle);
	}
	
	TCanvas *c = new TCanvas("c","",800,800);
	c->Divide(2,1);
	c->cd(1);
	gr->Draw("ALP");
	c->cd(2);
	grEm->Draw("ALP");

	//outFile.close();
	//cout<<"Done"<<endl;
	return 0;
}