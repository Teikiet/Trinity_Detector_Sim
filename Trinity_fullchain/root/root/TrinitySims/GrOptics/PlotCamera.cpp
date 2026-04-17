#include <TH1.h>
#include <TH2.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TMath.h>

void DrawMUSICBoundaries()
{
        TBox *b = new TBox(-48.5,-48.5,-36.5,-24.5);
        b->SetFillStyle(0);
        b->SetLineColor(kRed);
        for(int i=1; i<32; i++)
        {
                TBox *bn = (TBox*)b->Clone();
                bn->SetX1((i%8)*12-48.5);
                bn->SetX2((i%8)*12-36.5);
                bn->SetY1((i/8)*24-48.5);
                bn->SetY2((i/8)*24-24.5);
                bn->Draw();
        }
}

void PlotCamera(const char* filename, const char* outname)
{
        TCanvas *canvas = new TCanvas("Display","Event Display",2500,1000);
        canvas->Divide(2,1);
        canvas->cd(1)->SetRightMargin(0.15);
        canvas->cd(2)->SetRightMargin(0.15);
        int nResMult = 100;
        TH1F *hPhotonX = new TH1F("hist photonX","PhotonX Distribution", 192, -96, 96);
        hPhotonX->GetXaxis()->SetTitle("Photon X Position (mm)");
        hPhotonX->GetYaxis()->SetTitle("Number of Photons");
        hPhotonX->SetStats(0);

        TH1F *hPhotonY = new TH1F("hist photonY","PhotonY Distribution", 96, -48, 48);
        hPhotonY->GetXaxis()->SetTitle("Photon Y Position (mm)");
        hPhotonY->GetYaxis()->SetTitle("Number of Photons");
        hPhotonY->SetStats(0);

        TH1F *hPhotonDcosX = new TH1F("hist photonDcosX","PhotonDcosX Distribution", 100, -1, 1);
        hPhotonDcosX->GetXaxis()->SetTitle("PhotonDcosX");
        hPhotonDcosX->GetYaxis()->SetTitle("Number of Photons");
        hPhotonDcosX->SetStats(0);

        TH1F *hPhotonDcosY = new TH1F("hist photonDcosY","PhotonDcosY Distribution", 100, -1, 1);
        hPhotonDcosY->GetXaxis()->SetTitle("PhotonDcosY");
        hPhotonDcosY->GetYaxis()->SetTitle("Number of Photons");
        hPhotonDcosY->SetStats(0);

        TH1F *hPhotonZ = new TH1F("hist photonZ","Photon Incidence Angle Distribution", 180, 0, 90);
        hPhotonZ->GetXaxis()->SetTitle("Photon Incidence Angle (Deg)");
        hPhotonZ->GetYaxis()->SetTitle("Number of Photons");
        hPhotonZ->SetStats(0);

        TH2F* hPhotonDist1x = new TH2F("hist1x", "Camera View, incident photons, 1mm bins", 96, -48.5, 47.5, 96, -48.5, 47.5);
        hPhotonDist1x->GetXaxis()->SetTitle("X (mm)");
        hPhotonDist1x->GetYaxis()->SetTitle("Y (mm)");

        TH2F* hPhotonDist1x6mm = new TH2F("hist1x", "Camera View, incident photons, 6mm bins", 16, -48.5, 47.5, 16, -48.5, 47.5);
        hPhotonDist1x->GetXaxis()->SetTitle("X (mm)");
        hPhotonDist1x->GetYaxis()->SetTitle("Y (mm)");

        TH2F* hPhotonDist4x = new TH2F("hist4x", "Camera View (4x)", 32*nResMult, -96.5, 95.5, 16*nResMult, -48.5, 47.5);
        hPhotonDist4x->GetXaxis()->SetTitle("X (mm)");
        hPhotonDist4x->GetYaxis()->SetTitle("Y (mm)");
        hPhotonDist4x->SetStats(0);

        float RadtoDeg = 57.2958;
        std::vector<float>* vPhotonX = 0;
        std::vector<float>* vPhotonY = 0;
        std::vector<float>* vPhotonDcosX = 0;
        std::vector<float>* vPhotonDcosY = 0;

        TFile* file = TFile::Open(filename, "READ");
        TTree* tree = (TTree*)file->Get("T1");
        tree->SetBranchAddress("photonX", &vPhotonX);
        tree->SetBranchAddress("photonY", &vPhotonY);
        tree->SetBranchAddress("photonDcosX", &vPhotonDcosX);
        tree->SetBranchAddress("photonDcosY", &vPhotonDcosY);

        tree->GetEntry(0);

        for (size_t j = 0; j < vPhotonX->size(); j++)
        {
                hPhotonX->Fill((*vPhotonX)[j]);
                hPhotonY->Fill((*vPhotonY)[j]);
                hPhotonDcosX->Fill((*vPhotonDcosX)[j]);
                hPhotonDcosY->Fill((*vPhotonDcosY)[j]);
                hPhotonZ->Fill(TMath::ASin(sqrt(pow((*vPhotonDcosX)[j],2)+pow((*vPhotonDcosY)[j],2)))*RadtoDeg);
                hPhotonDist1x->Fill((*vPhotonX)[j], (*vPhotonY)[j]);
                hPhotonDist1x6mm->Fill((*vPhotonX)[j], (*vPhotonY)[j]);
                hPhotonDist4x->Fill((*vPhotonX)[j], (*vPhotonY)[j]);
        }
        canvas->cd(1);
        hPhotonDist1x->SetMarkerSize(200.0);
        hPhotonDist1x->SetStats(0);
        hPhotonDist1x->Draw("colz");
        DrawMUSICBoundaries();
        canvas->cd(2);
        hPhotonDist1x6mm->SetMarkerSize(200.0);
        hPhotonDist1x6mm->SetStats(0);
        hPhotonDist1x6mm->Draw("colz");
        DrawMUSICBoundaries();
        file->Close();
        canvas->Print(outname);
}
