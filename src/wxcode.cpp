/**
*
* wxcode.cpp
*
* - implementation for wx-based code editor
*
**/

#include "wxcode.hpp"

my1CodeEdit::my1CodeEdit(wxWindow *parent, int id, wxString fullname)
	: wxStyledTextCtrl( parent, id, wxDefaultPosition, wxDefaultSize ),
		mFullName(fullname)
{
	mFullName.Normalize(); // just in case
	this->SetUseHorizontalScrollBar(false);
	this->SetEOLMode(2); // CRLF, CR, or LF=2?
	this->LoadFile(mFullName.GetFullPath());
}

wxString my1CodeEdit::GetPathName(void)
{
	return mFullName.GetPathWithSep();
}

wxString my1CodeEdit::GetFileName(void)
{
	return mFullName.GetFullName();
}

wxString my1CodeEdit::GetFullName(void)
{
	return mFullName.GetFullPath();
}

wxString my1CodeEdit::GetFileNoXT(void)
{
	return mFullName.GetName();
}
