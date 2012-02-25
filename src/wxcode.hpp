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
	wxString GetModFileName(const wxString&);
	wxString GetFullName(void);
	wxString GetFileNoXT(void);
	bool SaveEdit(const wxString&);
	void MarkLine(int,bool aMark=true);
	bool ExecLine(int);
	void ExecDone(void);
	bool IsBreakLine(void);
	void ToggleBreak(int);
	void OnCodeChanged(wxStyledTextEvent &event);
	void OnCodeMarked(wxStyledTextEvent &event);
};

#endif
