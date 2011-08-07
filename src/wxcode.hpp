/**
*
* wxcode.hpp
*
* - header for wx-based code editor
*
**/

#include <wx/wx.h>
#include <wx/stc/stc.h>
#include <wx/filename.h>
#include "wxpref.hpp"

#ifndef __MY1CODE_HPP__
#define __MY1CODE_HPP__


class my1CodeEdit : public wxStyledTextCtrl
{
private:
	wxWindow* mParent;
	bool mLockedLoad;
public:
	my1CodeEdit(wxWindow *parent, int id, wxString &fullname, my1Options &options);
	bool IsLockedLoad(void);
	void SetLockedLoad(bool aLocked=true);
	wxFileName mFullName;
	wxString GetPathName(void);
	wxString GetFileName(void);
	wxString GetFullName(void);
	wxString GetFileNoXT(void);
};

#endif
