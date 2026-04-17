/*
=============================================================================
    corsikaIOreader is a tool to read CORSIKA eventio files
    Copyright (C) 2004, 2013, 2019 Gernot Maier and Henrike Fleischhack

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
=============================================================================*/
/*! \class VIOHistograms
    \brief histogram filling class for CORSIKA cherenkov output (eventio files)

    \author
         Gernot Maier

    \date
         26/04/04
*/

#include "VIOHistograms.h"

VIOHistograms::VIOHistograms()
{
    hisList = new TList();
    nevent = 0;
    degrad = 45. / atan( 1. );
    
    bCORSIKA_coordinates = false;
    
    bSmallFile = false;
    bMuon = false;
}

void VIOHistograms::init( string i_outfile, bool iShort )
{
    fout = new TFile( i_outfile.c_str(), "RECREATE" );
    if( fout->IsZombie() )
    {
        cout << "error while opening root output file: " << i_outfile.c_str() << endl;
        exit( -1 );
    }
    if( bSmallFile )
    {
        cout << "\t\t smallfile output" << endl;
    }
    bShort = iShort;
    
    int xybin = 1000;
    double xmax = 1000.;
    double ymax = 1000.;
    double zemax = 30000.;
    if( bSmallFile )
    {
        xybin = 500;
        xmax = 500.;
        ymax = 500.;
    }
    else if( bMuon )
    {
        xybin = 500;
        xmax = 15.;
        ymax = 15.;
        zemax = 3000.;
    }
    char hname[200];
    char htitle[200];
    
    if( !bShort )
    {
    
        sprintf( hname, "hGLambda" );
        sprintf( htitle, "Cherenkov photon wavelength (no absorption/efficencies applied)" );
        hGLambda = new TH1D( hname, htitle, 400, 0., 800. );
        hGLambda->SetXTitle( "wavelength [nm]" );
        hisList->Add( hGLambda );
        
        sprintf( hname, "hBunch" );
        sprintf( htitle, "Cherenkov bunch size," );
        hBunch = new TH1D( hname, htitle, 600, 0., 6. );
        hBunch->SetXTitle( "bunch size" );
        hZem = new TH1D( "hZem", "height of Cherenkov bunch emission", 500, 0., zemax );
        hZem->SetXTitle( "height [m]" );
        sprintf( hname, "hT0" );
        sprintf( htitle, "Cherenkov bunch arrival times " );
        hT0 = new TH1D( hname, htitle, 2000, -1000., 1000. );
        hT0->SetXTitle( "arrival time [ns]" ),
            hT0->SetLineColor( 2 );
        hisList->Add( hT0 );
        
        sprintf( hname, "hGXY" );
        sprintf( htitle, "Cherenkov photon positions (no absorption/efficencies applied)" );
        hGXY = new TH2D( hname, htitle, xybin, -1. * xmax, xmax, xybin, -1. * ymax, ymax );
        hGXY->SetXTitle( "x [m] (north)" );
        hGXY->SetYTitle( "y [m] (west)" );
        hisList->Add( hGXY );
        
        sprintf( hname, "hGZeAz" );
        sprintf( htitle, "Cherenkov photon directions (no absorption/efficencies applied)" );
        hGZeAz = new TH2D( hname, htitle, 200, 0., 20., 720, 0., 360. );
        hGZeAz->SetXTitle( "photon zenith angle [deg]" );
        hGZeAz->SetYTitle( "photon azimuth angle [deg]" );
        hisList->Add( hGZeAz );
        
        hGProb = new TH1D( "hGProb", "event survival probability (no absorption/efficencies applied)", 100, 0., 1. );
        hGProb->SetXTitle( "propability" );
        hisList->Add( hGProb );
        
        hGZem = new TH1D( "hGZem", "Cherenkov photon emission height (no absorption/efficencies applied)", 500, 0., zemax );
        hGZem->SetXTitle( "height [m]" );
        hisList->Add( hGZem );
        
        sprintf( hname, "hSXY" );
        sprintf( htitle, "Cherenkov bunch positions (absorption/efficencies applied)" );
        hSXY = new TH2D( hname, htitle, xybin, -1. * xmax, xmax, xybin, -1. * ymax, ymax );
        hSXY->SetXTitle( "x [m] (north)" );
        hSXY->SetYTitle( "y [m] (west)" );
        hisList->Add( hSXY );
        
        sprintf( hname, "hSZeAz" );
        sprintf( htitle, "Cherenkov photon directions (absorption/efficencies applied)" );
        hSZeAz = new TH2D( hname, htitle, 200, 0., 20., 720, 0., 360. );
        hSZeAz->SetXTitle( "photon zenith angle [deg]" );
        hSZeAz->SetYTitle( "photon azimuth angle [deg]" );
        hisList->Add( hSZeAz );
        
        sprintf( hname, "hSLambda" );
        sprintf( htitle, "Cherenkov photon wavelength (absorption/efficencies applied)" );
        hSLambda = new TH1D( hname, htitle, 400, 0., 800. );
        hSLambda->SetXTitle( "wavelength [nm]" );
        hSLambda->SetLineColor( 2 );
        hisList->Add( hSLambda );
        
        hSProb = new TH1D( "hSProb", "event survival probability (absorption/efficencies applied)", 100, 0., 1. );
        hSProb->SetXTitle( "propability" );
        hSProb->SetLineColor( 2 );
        hisList->Add( hSProb );
        
        hSZem = new TH1D( "hSZem", "Cherenkov photon emission height (absorption/efficencies applied)", 500, 0., zemax );
        hSZem->SetXTitle( "height [m]" );
        hSZem->SetLineColor( 2 );
        hisList->Add( hSZem );
    }
    else
    {
        hBunch = 0;
        hT0 = 0;
        hZem = 0;
        hGXY = 0;
        hGZeAz = 0;
        hGLambda = 0;
        hGProb = 0;
        hGZem = 0;
        hSXY = 0;
        hSZeAz = 0;
        hSLambda = 0;
        hSProb = 0;
        hSZem = 0;
    }
    
    
    fTree = new TTree( "tcors", "CORSIKA results" );
    
    fTree->Branch( "eventNumber", &eventNumber, "eventNumber/I" );
    fTree->Branch( "arrayNumber", &arrayNumber, "arrayNumber/I" );
    fTree->Branch( "particleID", &particleID, "particleID/I" );
    fTree->Branch( "energy", &energy, "energy/D" );
    fTree->Branch( "startAlt", &startAlt, "startAlt/D" );
    fTree->Branch( "firstInt", &firstInt, "firstInt/D" );
    fTree->Branch( "xmax", &xmax, "xmax/D" );
    fTree->Branch( "emax", &emax, "emax/D" );
    fTree->Branch( "cmax", &cmax, "cmax/D" );
    fTree->Branch( "hmax", &hmax, "hmax/D" );
    fTree->Branch( "ze", &ze, "ze/D" );
    fTree->Branch( "az", &az, "az/D" );
    fTree->Branch( "date", &date, "date/I" );
    fTree->Branch( "runNumber", &runNumber, "runNumber/I" );
    fTree->Branch( "corsVersion", &corsVersion, "corsVersion/D" );
    fTree->Branch( "obsLevel", &obsLevel, "obsLevel/D" );
    fTree->Branch( "eslope", &eslope, "eslope/D" );
    fTree->Branch( "e0min", &e0min, "e0min/D" );
    fTree->Branch( "e0max", &e0max, "e0max/D" );
    fTree->Branch( "cutHad", &cutHad, "cutHad/D" );
    fTree->Branch( "cutMuon", &cutMuon, "cutMuon/D" );
    fTree->Branch( "cutEM", &cutEM, "cutEM/D" );
    fTree->Branch( "cutPhot", &cutPhot, "cutPhot/D" );
    fTree->Branch( "magX", &magX, "magX/D" );
    fTree->Branch( "magZ", &magZ, "magZ/D" );
    fTree->Branch( "bCher", &bCher, "bCher/I" );
    fTree->Branch( "cherBunch", &cherBunch, "cherBunch/D" );
    fTree->Branch( "cherLambdaMin", &cherLambdaMin, "cherLambdaMin/D" );
    fTree->Branch( "cherLambdaMax", &cherLambdaMax, "cherLambdaMax/D" );
    fTree->Branch( "telNumber", &telNumber, "telNumber/I" );
    fTree->Branch( "coreNumber", &coreNumber, "coreNumber/I" );
    fTree->Branch( "telXpos", telXpos, "telXpos[telNumber]/D" );
    fTree->Branch( "telYpos", telYpos, "telYpos[telNumber]/D" );
    fTree->Branch( "telZpos", telZpos, "telZpos[telNumber]/D" );
    fTree->Branch( "telR", telR, "telR[telNumber]/D" );
    fTree->Branch( "xCore", &xCore, "xCore/D" );
    fTree->Branch( "yCore", &yCore, "yCore/D" );
    fTree->Branch( "rCore", &rCore, "rCore/D" );
    // number of Cherenkov photons per telescope
    fTree->Branch( "NCp", NCp, "NCp[telNumber]/D" );
    
    if( !bShort )
    {
        if( !bSmallFile )
        {
            fTree->Branch( "hGLambda", "TH1D", &hGLambda, 32000, 0 );
        }
        fTree->Branch( "hSLambda", "TH1D", &hSLambda, 32000, 0 );
        if( !bSmallFile )
        {
            fTree->Branch( "hSZeAz", "TH2D", &hSZeAz, 32000, 0 );
        }
        
        fTree->Branch( "hBunch", "TH1D", &hBunch, 32000, 0 );
        fTree->Branch( "hZem", "TH1D", &hZem, 32000, 0 );
        fTree->Branch( "hT0", "TH1D", &hT0, 32000, 0 );
        
        if( !bSmallFile )
        {
            fTree->Branch( "hGXY", "TH2D", &hGXY, 32000, 0 );
        }
        if( !bSmallFile )
        {
            fTree->Branch( "hGZeAz", "TH2D", &hGZeAz, 32000, 0 );
        }
        if( !bSmallFile )
        {
            fTree->Branch( "hGProb", "TH1D", &hGProb, 32000, 0 );
        }
        if( !bSmallFile )
        {
            fTree->Branch( "hGZem", "TH1D", &hGZem, 32000, 0 );
        }
        
        fTree->Branch( "hSXY", "TH2D", &hSXY, 32000, 0 );
        if( !bSmallFile )
        {
            fTree->Branch( "hSProb", "TH1D", &hSProb, 32000, 0 );
        }
        fTree->Branch( "hSZem", "TH1D", &hSZem, 32000, 0 );
    }
    
}

void VIOHistograms::newEvent( float* evth, telescope_array array, int i_array )
{
    if( nevent > 0 )
    {
        fTree->Fill();
    }
    
    if( !bShort )
    {
        hBunch->Reset();
        hZem->Reset();
        hT0->Reset();
        
        hSXY->Reset();
        hSZeAz->Reset();
        hSProb->Reset();
        hSLambda->Reset();
        hSZem->Reset();
        
        hGXY->Reset();
        hGZeAz->Reset();
        hGProb->Reset();
        hGLambda->Reset();
        hGZem->Reset();
        
        if( hCXYZ )
        {
            TH2D* it;
            for( int i = 0; i <= hCXYZ->GetLast(); i++ )
            {
                if( ( it = ( TH2D* )hCXYZ->At( i ) ) )
                {
                    it->Reset();
                }
            }
        }
    }
    
    eventNumber = ( int )evth[1];
    arrayNumber = ( int )i_array;
    particleID = ( int )evth[2];
    // avoid discrepancies between iotxt.root output and grisu file
    // (GM)   energy = evth[3];
    energy = array.shower_sim.energy * 1000.;
    startAlt = evth[4];
    firstInt = evth[6] * 0.01;
    xmax = array.shower_sim.xmax;
    emax = array.shower_sim.emax;
    cmax = array.shower_sim.cmax;
    hmax = array.shower_sim.hmax;
    ze = evth[10] * degrad;
    az  = evth[11] * degrad;
    date = ( int )evth[44];
    runNumber = ( int )evth[43];
    corsVersion = evth[45];
    obsLevel = evth[47] / 100.;        // [m]
    eslope = evth[57];
    e0min = evth[58];
    e0max = evth[59];
    cutHad = evth[60];
    cutMuon = evth[61];
    cutEM = evth[62];
    cutPhot = evth[63];
    magX = evth[70];
    magZ = evth[71];
    bCher = ( int )evth[76];
    cherBunch = evth[84];
    cherLambdaMin = evth[95];
    cherLambdaMax = evth[96];
    
    telNumber = ( int )array.ntel;
    coreNumber = ( int )array.narray;
    for( int i = 0; i < telNumber; i++ )
    {
        telXpos[i] = array.xtel[i] * 0.01;
        telYpos[i] = array.ytel[i] * 0.01;
        telZpos[i] = array.ztel[i] * 0.01;
        telR[i] = array.rtel[i] * 0.01;
        NCp[i] = 0.;
    }
    xCore = array.shower_sim.xcore;
    yCore = array.shower_sim.ycore;
    rCore = array.shower_sim.core_dist_3d;
    
    toff = array.toff;
    
    // number of Cherenkov photons (sum over all telescopes)
    
    nevent++;
}

void VIOHistograms::fillBunch( bunch i_bunch, double itime )
{
    if( !bShort )
    {
        hBunch->Fill( i_bunch.photons );
        hT0->Fill( itime );
        hZem->Fill( i_bunch.zem * 0.01 );
    }
}

/*!
    in corsika coordinates
*/
void VIOHistograms::fillGenerated( bunch ph, double prob )
{
    if( !bShort )
    {
        double az = 0.;
        double ze = 0.;
        
        az = atan2( ph.cy, ph.cx );
        az = redang( az );
        if( ph.cy != 0. )
        {
            ze = asin( ph.cy / sin( az ) );
        }
        else
        {
            ze = asin( ph.cy / cos( az ) );
        }
        ze *= degrad;
        az *= degrad;
        if( az < 0 )
        {
            az = 360. - az;
        }
        
        if( !bCORSIKA_coordinates )
        {
            hGXY->Fill( ph.x, ph.y );
        }
        else
        {
            hGXY->Fill( ph.y, -ph.x );
        }
        hGZeAz->Fill( ze, az );
        hGProb->Fill( prob );
        hGLambda->Fill( ph.lambda );
        hGZem->Fill( ph.zem );
    }
}

void VIOHistograms::fillSurvived( bunch ph, double prob )
{
    if( !bShort )
    {
        double az = 0.;
        double ze = 0.;
        
        az = atan2( ph.cy, ph.cx );
        az = redang( az );
        if( ph.cy != 0. )
        {
            ze = asin( ph.cy / sin( az ) );
        }
        else
        {
            ze = asin( ph.cy / cos( az ) );
        }
        ze *= degrad;
        az *= degrad;
        if( az < 0 )
        {
            az = 360. - az;
        }
        
        if( !bCORSIKA_coordinates )
        {
            hSXY->Fill( ph.x, ph.y );
        }
        else
        {
            hSXY->Fill( ph.y, -ph.x );
        }
        hSZeAz->Fill( ze, az );
        hSProb->Fill( prob );
        hSLambda->Fill( ph.lambda );
        hSZem->Fill( ph.zem );
        
        if( !hCXYZ )
        {
            return;
        }
        
        // fill xyz histograms
        double xp, yp;
        TH2D* it;
        for( unsigned int i = 0; i < fXYZlevelsHeight.size(); i++ )
        {
            if( fXYZlevelsHeight[i] > ph.zem )
            {
                continue;
            }
            xp = ph.x + fXYZlevelsHeight[i] * ph.cx;
            yp = ph.y + fXYZlevelsHeight[i] * ph.cy;
            
            it = ( TH2D* )hCXYZ->At( i );
            if( it )
            {
                if( !bCORSIKA_coordinates )
                {
                    it->Fill( xp, yp );
                }
                else
                {
                    it->Fill( yp, -xp );
                }
            }
        }
        
    }
}

void VIOHistograms::fillNPhotons( int iTel, double iphotons )
{
    if( iTel < telNumber )
    {
        NCp[iTel] = iphotons;
    }
}

void VIOHistograms::terminate()
{
    if( fTree )
    {
        fTree->Fill();    // write last event
    }
    
    if( fout )
    {
        fout->cd();
        cout << "writing results (cherenkov histograms) in file: " << fout->GetName() << endl;
        fTree->Write();
        
        // write all histograms outside of tree if there is only one event in the tree
        if( fTree->GetEntries() == 1 )
        {
            hisList->Write();
        }
        fout->Close();
    }
    else
    {
        cout << "VIOHistograms::terminate() error writing to output file" << endl;
        cout << endl;
    }
}

//! reduce large angle to intervall 0, 2*pi
double VIOHistograms::redang( double iangle )
{
    if( iangle >= 0 )
    {
        iangle = iangle - int( iangle / ( 2. * M_PI ) ) * 2. * M_PI;
    }
    else
    {
        iangle = 2. * M_PI + iangle + int( iangle / ( 2. * M_PI ) ) * 2. * M_PI;
    }
    
    return iangle;
}

void VIOHistograms::transformCoord( double& az, double& x, double& y )
{
    az = redang( 1.5 * M_PI - redang( az ) );
    
    double i_y = y;
    y = -1.* x;
    x = -1.* i_y;
}

/////
// fill xy histograms for different heights
//
//    only photons which actually reach the ground are tracked
///

void VIOHistograms::setXYZlevelsThickness( double istart, double idiff )
{
    fXYZlevelsThickness.clear();
    
    int nlevel = ( int )( ( 1005. - istart ) / idiff );
    
    for( int i = 0; i <= nlevel; i++ )
    {
        fXYZlevelsThickness.push_back( istart + i * idiff );
    }
}

void VIOHistograms::initXYZhistograms()
{
    if( bShort )
    {
        return;
    }
    
    if( !fout || fout->IsZombie() )
    {
        cout << "VIOHistograms::initXYZhistograms() error, output file not defined" << endl;
        exit( 0 );
    }
    
    fout->cd();
    
    int xybin = 500;
    double xmax = 500.;
    double ymax = 500.;
    char hname[200];
    char htitle[200];
    
    // level thickness not set, assume as standard 20gm/cm2 diff from 5 gm/cm2
    if( fXYZlevelsHeight.size() != fXYZlevelsThickness.size() )
    {
        setXYZlevelsThickness( 5., 40. );
        if( fXYZlevelsHeight.size() != fXYZlevelsThickness.size() )
        {
            cout << "VIOHistograms::initXYZhistograms() problems with definition of levels" << endl;
            exit( 0 );
        }
    }
    
    hCXYZ = new TClonesArray( "TH2D", ( int )fXYZlevelsHeight.size() );
    hCXYZ->BypassStreamer();
    TClonesArray& iCXYZ = *hCXYZ;
    
    for( unsigned int i = 0; i < fXYZlevelsHeight.size(); i++ )
    {
        sprintf( hname, "hSXY_%d", ( int )fXYZlevelsThickness[i] );
        sprintf( htitle, "Cherenkov bunch positions at %d gm/cm2 (%.2f m) (absorption/efficencies applied)", ( int )fXYZlevelsThickness[i], fXYZlevelsHeight[i] );
        
        new( iCXYZ[i] ) TH2D( hname, htitle, xybin, -1. * xmax, xmax, xybin, -1. * ymax, ymax );
        TH2D* iT = ( TH2D* )iCXYZ[i];
        iT->SetXTitle( "x [m] (north)" );
        iT->SetYTitle( "y [m] (west)" );
    }
    
    fTree->Branch( "hCXYZ", &hCXYZ, 256000, 0 );
}


