/**
*
* wxmain.cpp
*
* - implementation for main wx-based application
*
**/

#include "wxmain.hpp"
#include "wxform.hpp"

#ifndef MY1APP_TITLE
#define MY1APP_TITLE "my1warez"
#endif

IMPLEMENT_APP(my1App)

my1App* my1AppPointer;

bool my1App::OnInit()
{
	my1Form *form = new my1Form(wxT(MY1APP_TITLE));
	form->Show(true);
	this->SetTopWindow(form);
	my1AppPointer = this;
	return true;
}
