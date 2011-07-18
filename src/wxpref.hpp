/**
*
* wxpref.hpp
*
* - header for wx-based options dialog
*
**/

#include <wx/wx.h>

#ifndef __MY1PREF_HPP__
#define __MY1PREF_HPP__

enum {
	MY1ID_PREF_SAVE = wxID_HIGHEST+401,
	MY1ID_PREF_CANCEL,
	MY1ID_PREF_VIEWWS,
	MY1ID_PREF_VIEWEOL,
	MY1ID_PREF_DUMMY
};

struct my1Options
{
	bool mChanged;
	bool mEdit_ViewWS, mEdit_ViewEOL;
};

class my1OptionDialog : public wxDialog
{
private:
	my1Options &mParentOptions;
	my1Options mCurrentOptions;
public:
	my1OptionDialog(wxWindow *parent, const wxString &title, my1Options &options);
	void OnOptCheck(wxCommandEvent &event);
	void OnOptSave(wxCommandEvent &event);
	void OnOptClose(wxCommandEvent &event);
};

#endif
