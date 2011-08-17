/**
*
* wxcode.cpp
*
* - implementation for wx-based code editor
*
**/

#include "wxcode.hpp"
#include <wx/aui/auibook.h>

enum {
	MARGIN_LINE_NUMBERS,
	MARGIN_EXEC_STAT
};

enum {
	MARKER_EXEC_CURRENT,
	MARKER_EXEC_BREAK,
	MARKER_EXEC_LINEBREAK,
	MARKER_EXEC_OKAY
};

my1CodeEdit::my1CodeEdit(wxWindow *parent, int id, wxString &fullname, my1Options &options)
	: wxStyledTextCtrl( parent, id, wxDefaultPosition, wxDefaultSize ),
		mFullName(fullname)
{
	mParent = parent;
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
	this->SetMarginType(MARGIN_LINE_NUMBERS,wxSTC_MARGIN_NUMBER);
	this->SetMarginWidth(MARGIN_LINE_NUMBERS,45);
	this->SetMarginType(MARGIN_EXEC_STAT, wxSTC_MARGIN_SYMBOL);
	this->SetMarginWidth(MARGIN_EXEC_STAT, 15);
	this->SetMarginSensitive(MARGIN_EXEC_STAT, true);
	this->MarkerDefine(MARKER_EXEC_CURRENT,wxSTC_MARK_SHORTARROW);
	this->MarkerDefine(MARKER_EXEC_BREAK,wxSTC_MARK_CIRCLE,*wxWHITE,*wxRED);
	this->MarkerDefine(MARKER_EXEC_LINEBREAK,wxSTC_MARK_BACKGROUND,*wxWHITE,*wxRED);
	this->MarkerDefine(MARKER_EXEC_OKAY,wxSTC_MARK_EMPTY);
	wxFont cTestFont(10,wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	cTestFont.SetNativeFontInfoUserDesc(wxT("Monospace 10"));
	this->StyleSetFont(0,cTestFont);
	this->StyleSetForeground(wxSTC_STYLE_LINENUMBER,wxColour(10,10,10));
	this->StyleSetBackground(wxSTC_STYLE_LINENUMBER,wxColour(220,220,220));
	this->SetCaretLineBackAlpha(75);
	this->SetCaretLineBackground(*wxBLUE);
	this->SetModEventMask(wxSTC_MOD_INSERTTEXT|wxSTC_MOD_DELETETEXT); //|wxSTC_PERFORMED_USER
	this->GotoLine(0);
	mModifyChecked = false;
	this->Connect(id, wxEVT_STC_MODIFIED, wxStyledTextEventHandler(my1CodeEdit::OnCodeChanged));
	this->Connect(id, wxEVT_STC_MARGINCLICK, wxStyledTextEventHandler(my1CodeEdit::OnCodeMarked));
}

wxString my1CodeEdit::GetPathName(void)
{
	return mFullName.GetPathWithSep();
}

wxString my1CodeEdit::GetFileName(void)
{
	return mFullName.GetFullName();
}

wxString my1CodeEdit::GetModFileName(const wxString& aString)
{
	wxString cTest = mFullName.GetFullName();
	if(!cTest.Length()) cTest = wxT("*") + aString;
	else cTest = wxT("*") + cTest;
	return cTest;
}

wxString my1CodeEdit::GetFullName(void)
{
	return mFullName.GetFullPath();
}

wxString my1CodeEdit::GetFileNoXT(void)
{
	return mFullName.GetName();
}

bool my1CodeEdit::SaveEdit(const wxString& fullname)
{
	if(fullname.length())
	{
		mFullName = fullname;
		mFullName.Normalize(); // just in case
	}
	bool cStatus = this->SaveFile(this->GetFullName());
	if(cStatus&&mModifyChecked)
	{
		wxAuiNotebook* cNoteBook = (wxAuiNotebook*) mParent;
		int cSelect = cNoteBook->GetSelection();
		cNoteBook->SetPageText(cSelect,this->GetFileName());
		mModifyChecked = false;
	}
	return cStatus;
}

void my1CodeEdit::MarkLine(int aLine, bool aMark)
{
	if(aMark)
		this->MarkerAdd(aLine,MARKER_EXEC_OKAY);
	else
		this->MarkerDelete(aLine,MARKER_EXEC_OKAY);
}

void my1CodeEdit::ExecMode(void)
{
	this->SetCaretLineVisible(true);
}

bool my1CodeEdit::ExecLine(int aLine)
{
	this->MarkerDelete(this->GetCurrentLine(),MARKER_EXEC_CURRENT);
	this->GotoLine(aLine);
	this->MarkerAdd(this->GetCurrentLine(),MARKER_EXEC_CURRENT);
	this->SetSTCFocus(true);
	return this->IsBreakLine();
}

void my1CodeEdit::ExecDone(void)
{
	this->SetCaretLineVisible(false);
	this->MarkerDeleteAll(MARKER_EXEC_CURRENT);
}

bool my1CodeEdit::IsBreakLine(void)
{
	bool cBreak = false;
	if(this->MarkerGet(this->GetCurrentLine())&(0x1<<MARKER_EXEC_BREAK))
		cBreak = true;
	return cBreak;
}

void my1CodeEdit::ToggleBreak(int aLine)
{
	if(this->MarkerGet(aLine)&(0x1<<MARKER_EXEC_BREAK))
	{
		this->MarkerDelete(aLine,MARKER_EXEC_BREAK);
		this->MarkerDelete(aLine,MARKER_EXEC_LINEBREAK);
	}
	else // if(this->MarkerGet(aLine)&(0x1<<MARKER_EXEC_OKAY))
	{
		this->MarkerAdd(aLine,MARKER_EXEC_LINEBREAK);
		this->MarkerAdd(aLine,MARKER_EXEC_BREAK);
	}
}

void my1CodeEdit::OnCodeChanged(wxStyledTextEvent &event)
{
	if(mModifyChecked) return;
	if(this->GetModify())
	{
		wxAuiNotebook* cNoteBook = (wxAuiNotebook*) mParent;
		int cSelect = cNoteBook->GetSelection();
		cNoteBook->SetPageText(cSelect,this->GetModFileName(cNoteBook->GetPageText(cSelect)));
		mModifyChecked = true;
	}
}

void my1CodeEdit::OnCodeMarked(wxStyledTextEvent &event)
{
	int cLine = this->LineFromPosition(event.GetPosition());
	this->ToggleBreak(cLine);
}
