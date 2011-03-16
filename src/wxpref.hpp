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

#define MY1ID_PREF_SAVE   401
#define MY1ID_PREF_CANCEL 402
#define MY1ID_RBCHECK_1   421
#define MY1ID_RBCHECK_2   422
#define MY1ID_RBCHECK_3   423
#define MY1ID_TCCHECK_W   430
#define MY1ID_TCCHECK_H   431

struct my1Options
{
	int mChanged;
	int mWidth, mHeight;
	int mGridSize, mBankSize;
};

class my1OptionDialog : public wxDialog
{
private:
	my1Options &mParentOptions;
	my1Options mCurrentOptions;
public:
	my1OptionDialog(wxWindow *parent, const wxString &title, my1Options &options);
	void OnRBSizeCheck(wxCommandEvent &event);
	void OnOptSave(wxCommandEvent &event);
	void OnOptClose(wxCommandEvent &event);
	wxRadioButton *rbut_set1;
	wxRadioButton *rbut_set2;
	wxRadioButton *rbut_cust;
	wxTextCtrl *tc_custw;
	wxTextCtrl *tc_custh;
	wxStaticText *stext_gridsize;
	wxTextCtrl *tc_gridsize;
	wxStaticText *stext_banksize;
	wxTextCtrl *tc_banksize;
	wxButton *buttOK;
	wxButton *buttKO;
};

#endif
