/**
*
* wxmain.hpp
*
* - header for main wx-based application
*
**/

#ifndef __MY1MAIN_HPP__
#define __MY1MAIN_HPP__

#include <wx/wx.h>
#include <wx/snglinst.h>

class my1App : public wxApp
{
	wxSingleInstanceChecker *my1Checker;
	virtual bool OnInit();
	virtual int OnExit();
};

#endif
