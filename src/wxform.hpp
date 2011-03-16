/**
*
* wxform.hpp
*
* - header for main wx-based form
*
**/

#include <wx/wx.h>

#ifndef __MY1FORM_HPP__
#define __MY1FORM_HPP__

#include "wxeditor.hpp"

#define MY1APP_TITLE "MY1 SIM85"
#ifdef __WXMSW__
    #define USE_XPM_BITMAPS 0
	#define MY1APP_ICON "apps"
#else
    #define USE_XPM_BITMAPS 1
	#define MY1APP_ICON "bitmaps/apps.xpm"
#endif

#define MY1ID_EXIT     100
#define MY1ID_CLEAR    101
#define MY1ID_LOAD     102
#define MY1ID_SAVE     103
#define MY1ID_GENERATE 104
#define MY1ID_OPTIONS  105

class my1Form : public wxFrame
{
public:
	my1Form(const wxString &title);
	void OnQuit(wxCommandEvent &event);
	void OnClear(wxCommandEvent &event);
	void OnLoad(wxCommandEvent &event);
	void OnSave(wxCommandEvent &event);
	void OnGenerate(wxCommandEvent &event);
	void OnCheckOptions(wxCommandEvent &event);
private:
	my1Editor *mEditor;
};

#endif
