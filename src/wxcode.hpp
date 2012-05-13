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

#define DEFAULT_FONT_SIZE 10
#define LARGE_FONT_SIZE 17

class my1CodeEdit : public wxStyledTextCtrl
{
private:
	wxWindow* mParent;
	bool mModifyChecked;
	bool mLargeFont;
	bool mShowLine;
	int mPrevLine;
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
	void ExecLine(int,bool aMark=true);
	void ExecDone(void);
	bool IsBreakLine(int aLine=-1);
	void ToggleBreak(int);
	void SetFont(wxFont&);
	void SetFontSize(int);
	void SetKeywordColor(void);
	void SetEditStyle(int);
	void ShowLine(bool);
	void OnCodeChanged(wxStyledTextEvent &event);
	void OnCodeMarked(wxStyledTextEvent &event);
	void OnMouseClick(wxStyledTextEvent &event);
};

#endif
