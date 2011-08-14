/**
*
* wxcode.hpp
*
* - header for wx-based code editor
*
**/

#ifndef __MY1CODE_HPP__
#define __MY1CODE_HPP__

#include <wx/wx.h>
#include <wx/stc/stc.h>
#include <wx/filename.h>
#include "wxpref.hpp"

class my1CodeEdit : public wxStyledTextCtrl
{
private:
	wxWindow* mParent;
	bool mModifyChecked;
public:
	my1CodeEdit(wxWindow *parent, int id, wxString &fullname, my1Options &options);
	wxFileName mFullName;
	wxString GetPathName(void);
	wxString GetFileName(void);
	wxString GetModFileName(void);
	wxString GetFullName(void);
	wxString GetFileNoXT(void);
	bool SaveEdit(void);
	void MarkLine(int,bool aMark=true);
	void ExecMode(void);
	bool ExecLine(int);
	void ExecDone(void);
	void ToggleBreak(int);
	void OnCodeChanged(wxStyledTextEvent &event);
	void OnCodeMarked(wxStyledTextEvent &event);
};

#endif
