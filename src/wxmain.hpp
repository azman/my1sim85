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

class my1App : public wxApp
{
	virtual bool OnInit();
};

extern my1App* my1AppPointer;

#endif
