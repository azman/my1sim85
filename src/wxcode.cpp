/**
*
* wxcode.cpp
*
* - implementation for wx-based code editor
*
**/

#include "wxcode.hpp"

my1CodeEdit::my1CodeEdit(wxWindow *parent, int id, wxString &fullname, my1Options &options)
	: wxStyledTextCtrl( parent, id, wxDefaultPosition, wxDefaultSize ),
		mFullName(fullname)
{
	mParent = parent;
	mLockedLoad = false;
	if(fullname.length())
	{
		mFullName.Normalize(); // just in case
		this->LoadFile(mFullName.GetFullPath());
	}
	//this->SetLexer(wxSTC_LEX_ASM); // cannot get monospaced font with this?
	this->SetUseHorizontalScrollBar(false);
	this->SetEOLMode(2); // CRLF, CR, or LF=2?
	this->SetViewEOL(options.mEdit_ViewEOL);
	this->SetViewWhiteSpace(options.mEdit_ViewWS?1:0); // in, visible, visible outside indentation=2?
	this->SetMarginType(0,wxSTC_MARGIN_NUMBER);
	this->SetMarginWidth(0,40);
	wxFont cTestFont(10,wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	cTestFont.SetNativeFontInfoUserDesc(wxT("Monospace 10"));
	this->StyleSetFont(0,cTestFont);
	this->StyleSetForeground(wxSTC_STYLE_LINENUMBER,wxColour(75,75,75));
	this->StyleSetBackground(wxSTC_STYLE_LINENUMBER,wxColour(220,220,220));
	this->GotoLine(0);
}

bool my1CodeEdit::IsLockedLoad(void)
{
	return mLockedLoad;
}

void my1CodeEdit::SetLockedLoad(bool aLocked)
{
	mLockedLoad = aLocked;
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
