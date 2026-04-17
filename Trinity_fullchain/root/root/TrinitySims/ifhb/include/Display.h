#ifndef F_DISPLAY_H
#define F_DISPLAY_H

#include<TROOT.h>
#include<TCanvas.h>
#include<TH1.h>
#include<TProfile2D.h>
#include <Getline.h>
#include <TTimer.h>
#include <TApplication.h>
#include <TEveGeoShape.h>
#include <TEveManager.h>
#include <TEveArrow.h>
#include <TGeoSphere.h>
#include <TGeoShape.h>
namespace IFHB{
	/**
	 * \class Display
	 * \brief Class connecting ROOT Application and IFHB
	 * 
	 * Root uses TApplication to be able to perform interactive operations on plots
	 * The interconnection is done by invoking the TApplication and a timer.
	 * 
	 * It has functions to plot different types of histograms.
	 * */
	class Display{
	public:
		Display(int argc, char **argv);
		~Display();
		void ShowHistogram(TH1* h);
		void ShowHistogram(TH2* h);
		void TelescopeGeometry(TEveArrow *tLoc, TEveArrow *tPt);
	protected:
		TApplication *app;
		int color[5] = {1,2,3,4,6};
	};
}
#endif
