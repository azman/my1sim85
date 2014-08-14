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
	MY1ID_PREF_RUNINFO,
	MY1ID_PREF_STOPINT,
	MY1ID_PREF_STOPHLT,
	MY1ID_PREF_STARTADDR,
	MY1ID_PREF_DOLIST,
	MY1ID_PREF_DUMMY
};

struct my1Options
{
	bool mChanged;
	bool mComp_DoList;
	bool mEdit_ViewWS, mEdit_ViewEOL;
	bool mConv_UnixEOL;
	bool mSims_ShowRunInfo;
	bool mSims_PauseOnINTR;
	bool mSims_PauseOnHALT;
	int mSims_StartADDR;
	bool operator!=(my1Options& aOptions)
	{
		bool cChanged = true;
		if((mEdit_ViewWS==aOptions.mEdit_ViewWS)&&
			(mEdit_ViewEOL==aOptions.mEdit_ViewEOL)&&
			(mConv_UnixEOL==aOptions.mConv_UnixEOL)&&
			(mSims_ShowRunInfo==aOptions.mSims_ShowRunInfo)&&
			(mSims_PauseOnINTR==aOptions.mSims_PauseOnINTR)&&
			(mSims_PauseOnHALT==aOptions.mSims_PauseOnHALT)&&
			(mSims_StartADDR==aOptions.mSims_StartADDR)&&
			(mComp_DoList==aOptions.mComp_DoList))
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
	wxPanel* CreateCompPanel(void);
public:
	my1OptionDialog(wxWindow *parent, const wxString &title, my1Options &options);
	void OnOptCheck(wxCommandEvent &event);
	void OnOptSave(wxCommandEvent &event);
	void OnOptClose(wxCommandEvent &event);
};

#endif
