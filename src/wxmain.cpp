/**
*
* wxmain.cpp
*
* - implementation for main wx-based application
*
**/

#include "wxmain.hpp"
#include "wxform.hpp"
#include "wx/splash.h"

#ifndef MY1APP_TITLE
#define MY1APP_TITLE "my1warez"
#endif

#include "../res/splash.xpm"

IMPLEMENT_APP(my1App)

my1App* my1AppPointer;

bool my1App::OnInit()
{
	// do a splash screen!
	wxBitmap my1SplashBMP = MACRO_WXBMP(splash);
	wxSplashScreen my1Splash(my1SplashBMP,
		wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_TIMEOUT,3000,
		NULL, -1, wxDefaultPosition, wxDefaultSize,
		wxBORDER_SIMPLE|wxSTAY_ON_TOP);
	wxYield();
	// check mutex!
	wxString cMutexName = wxT("."MY1APP_PROGNAME) + wxGetUserId();
	my1Checker = new wxSingleInstanceChecker;
	my1Checker->Create(cMutexName,wxGetUserHome());
	if(my1Checker->IsAnotherRunning())
	{
		wxLog* logger = new wxLogStream(&std::cout);
		wxLog::SetActiveTarget(logger);
		wxLogError(wxT("Already running... aborting."));
		delete my1Checker;
		my1Checker = 0x0;
		return false;
	}
	// okay to continue... do a splash screen!
	my1Form *form = new my1Form(wxT(MY1APP_TITLE));
	form->Show(true);
	this->SetTopWindow(form);
	my1AppPointer = this;
	return true;
}

int my1App::OnExit()
{
	delete my1Checker;
	my1Checker = 0x0;
	return 0;
}
