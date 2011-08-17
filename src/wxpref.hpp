/**
*
* wxpref.hpp
*
* - header for wx-based options dialog
*
**/

#ifndef __MY1PREF_HPP__
#define __MY1PREF_HPP__

#include <wx/wx.h>
#include <wx/notebook.h>

enum {
	MY1ID_PREF_SAVE = wxID_HIGHEST+401,
	MY1ID_PREF_CANCEL,
	MY1ID_PREF_VIEWWS,
	MY1ID_PREF_VIEWEOL,
	MY1ID_PREF_UNIXEOL,
	MY1ID_PREF_FREERUN,
	MY1ID_PREF_STARTADDR,
	MY1ID_PREF_DUMMY
};

struct my1Options
{
	bool mChanged;
	bool mEdit_ViewWS, mEdit_ViewEOL;
	bool mConv_UnixEOL;
	bool mSims_FreeRunning;
	int mSims_StartADDR;
	bool operator!=(my1Options& aOptions)
	{
		bool cChanged = true;
		if((mEdit_ViewWS==aOptions.mEdit_ViewWS)&&
			(mEdit_ViewEOL==aOptions.mEdit_ViewEOL)&&
			(mConv_UnixEOL==aOptions.mConv_UnixEOL)&&
			(mSims_StartADDR==aOptions.mSims_StartADDR))
			cChanged = false;
		return cChanged;
	}
};

class my1OptionDialog : public wxDialog
{
private:
	my1Options &mParentOptions;
	my1Options mCurrentOptions;
	wxNotebook *mPrefBook;
protected:
	wxPanel* CreateEditPanel(void);
	wxPanel* CreateSimsPanel(void);
public:
	my1OptionDialog(wxWindow *parent, const wxString &title, my1Options &options);
	void OnOptCheck(wxCommandEvent &event);
	void OnOptSave(wxCommandEvent &event);
	void OnOptClose(wxCommandEvent &event);
};

#endif
