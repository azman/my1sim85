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

#ifndef __MY1CODE_HPP__
#define __MY1CODE_HPP__


class my1CodeEdit : public wxStyledTextCtrl
{
private:
	wxWindow* mParent;
public:
	my1CodeEdit(wxWindow *parent, int id, wxString &fullname);
	wxFileName mFullName;
	wxString GetPathName(void);
	wxString GetFileName(void);
	wxString GetFullName(void);
	wxString GetFileNoXT(void);
};

#endif
