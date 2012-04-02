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

// wxSTC_LEX_ASM KEYWORD LIST INDEX (from src/stc/scintilla/lexasm.cxx)
// 0 - wxSTC_ASM_CPUINSTRUCTION?
// 1 - wxSTC_ASM_MATHINSTRUCTION?
// 2 - wxSTC_ASM_REGISTER
// 3 - wxSTC_ASM_DIRECTIVE
// 4 - wxSTC_ASM_DIRECTIVEOPERAND
// 5 - wxSTC_ASM_EXTINSTRUCTION
#define KEYWORD_DIRECTIVE 0
#define KEYWORD_INSTRUCTION 1

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
	this->SetEditStyle(DEFAULT_FONT_SIZE);
	this->SetUseHorizontalScrollBar(false);
	this->SetEOLMode(2); // CRLF, CR, or LF=2?
	this->SetViewEOL(options.mEdit_ViewEOL);
	this->SetViewWhiteSpace(options.mEdit_ViewWS?1:0); // in, visible, visible outside indentation=2?
	this->SetMarginType(MARGIN_LINE_NUMBERS,wxSTC_MARGIN_NUMBER);
	this->SetMarginWidth(MARGIN_LINE_NUMBERS,45);
	this->SetMarginSensitive(MARGIN_LINE_NUMBERS, true);
	this->SetMarginType(MARGIN_EXEC_STAT, wxSTC_MARGIN_SYMBOL);
	this->SetMarginWidth(MARGIN_EXEC_STAT, 15);
	this->SetMarginSensitive(MARGIN_EXEC_STAT, true);
	this->MarkerDefine(MARKER_EXEC_CURRENT,wxSTC_MARK_SHORTARROW);
	this->MarkerDefine(MARKER_EXEC_BREAK,wxSTC_MARK_CIRCLE,*wxWHITE,*wxRED);
	this->MarkerDefine(MARKER_EXEC_LINEBREAK,wxSTC_MARK_BACKGROUND,*wxWHITE,*wxRED);
	this->MarkerDefine(MARKER_EXEC_OKAY,wxSTC_MARK_EMPTY);
	this->SetCaretLineBackAlpha(75);
	this->SetCaretLineBackground(*wxBLUE);
	this->SetModEventMask(wxSTC_MOD_INSERTTEXT|wxSTC_MOD_DELETETEXT); //|wxSTC_PERFORMED_USER
	this->GotoLine(0);
	mModifyChecked = false;
	mLargeFont = false;
	this->Connect(id, wxEVT_STC_MODIFIED, wxStyledTextEventHandler(my1CodeEdit::OnCodeChanged));
	this->Connect(id, wxEVT_STC_MARGINCLICK, wxStyledTextEventHandler(my1CodeEdit::OnCodeMarked));
	this->Connect(id, wxEVT_STC_DOUBLECLICK, wxStyledTextEventHandler(my1CodeEdit::OnMouseClick));
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
	if(cStatus)
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

bool my1CodeEdit::ExecLine(int aLine)
{
	this->SetCaretLineVisible(true);
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

void my1CodeEdit::SetFont(wxFont& aFont)
{
	// set pre-defined style
	this->StyleSetFont(wxSTC_STYLE_DEFAULT,aFont);
	this->StyleSetFont(wxSTC_STYLE_LINENUMBER,aFont);
	this->StyleClearAll();
	this->SetLexer(wxSTC_LEX_ASM);
	// set styles related to lexer
	this->StyleSetFont(wxSTC_ASM_DEFAULT,aFont);
	this->StyleSetFont(wxSTC_ASM_COMMENT,aFont);
	this->StyleSetFont(wxSTC_ASM_NUMBER,aFont);
	this->StyleSetFont(wxSTC_ASM_STRING,aFont);
	this->StyleSetFont(wxSTC_ASM_OPERATOR,aFont);
	this->StyleSetFont(wxSTC_ASM_IDENTIFIER,aFont);
	this->StyleSetFont(wxSTC_ASM_CPUINSTRUCTION,aFont);
	this->StyleSetFont(wxSTC_ASM_MATHINSTRUCTION,aFont);
	this->StyleSetFont(wxSTC_ASM_REGISTER,aFont);
	this->StyleSetFont(wxSTC_ASM_DIRECTIVE,aFont);
	this->StyleSetFont(wxSTC_ASM_DIRECTIVEOPERAND,aFont);
	this->StyleSetFont(wxSTC_ASM_COMMENTBLOCK,aFont);
	this->StyleSetFont(wxSTC_ASM_CHARACTER,aFont);
	this->StyleSetFont(wxSTC_ASM_STRINGEOL,aFont);
	this->StyleSetFont(wxSTC_ASM_EXTINSTRUCTION,aFont);
}

void my1CodeEdit::SetFontSize(int aSize)
{
	this->SetLexer(wxSTC_LEX_NULL);
	// set size for pre-defined style
	this->StyleSetSize(wxSTC_STYLE_DEFAULT,aSize);
	this->StyleSetSize(wxSTC_STYLE_LINENUMBER,aSize);
	this->SetLexer(wxSTC_LEX_ASM);
	// set size for styles related to lexer
	this->StyleSetSize(wxSTC_ASM_DEFAULT,aSize);
	this->StyleSetSize(wxSTC_ASM_COMMENT,aSize);
	this->StyleSetSize(wxSTC_ASM_NUMBER,aSize);
	this->StyleSetSize(wxSTC_ASM_STRING,aSize);
	this->StyleSetSize(wxSTC_ASM_OPERATOR,aSize);
	this->StyleSetSize(wxSTC_ASM_IDENTIFIER,aSize);
	this->StyleSetSize(wxSTC_ASM_CPUINSTRUCTION,aSize);
	this->StyleSetSize(wxSTC_ASM_MATHINSTRUCTION,aSize);
	this->StyleSetSize(wxSTC_ASM_REGISTER,aSize);
	this->StyleSetSize(wxSTC_ASM_DIRECTIVE,aSize);
	this->StyleSetSize(wxSTC_ASM_DIRECTIVEOPERAND,aSize);
	this->StyleSetSize(wxSTC_ASM_COMMENTBLOCK,aSize);
	this->StyleSetSize(wxSTC_ASM_CHARACTER,aSize);
	this->StyleSetSize(wxSTC_ASM_STRINGEOL,aSize);
	this->StyleSetSize(wxSTC_ASM_EXTINSTRUCTION,aSize);
}

void my1CodeEdit::SetKeywordColor(void)
{
	// assign desired colors
	wxColour cColorBlack = wxColour(0,0,0);
	wxColour cColorDirective = wxColour(0,0,180);
	wxColour cColorKeyword = wxColour(0,0,255);
	wxColour cColorArgument = wxColour(120,120,255);
	wxColour cColorComment = wxColour(120,120,120);
	wxColour cColorMarginFore = wxColour(10,10,10);
	wxColour cColorMarginBack = wxColour(220,220,220);
	// set color for pre-defined style
	this->StyleSetForeground(wxSTC_STYLE_DEFAULT,cColorBlack);
	this->StyleSetForeground(wxSTC_STYLE_LINENUMBER,cColorMarginFore);
	this->StyleSetBackground(wxSTC_STYLE_LINENUMBER,cColorMarginBack);
	// set color for styles related to lexer
	this->StyleSetForeground(wxSTC_ASM_DEFAULT,cColorBlack);
	this->StyleSetForeground(wxSTC_ASM_COMMENT,cColorComment);
	this->StyleSetForeground(wxSTC_ASM_NUMBER,cColorArgument);
	this->StyleSetForeground(wxSTC_ASM_STRING,cColorArgument);
	this->StyleSetForeground(wxSTC_ASM_OPERATOR,cColorBlack);
	this->StyleSetForeground(wxSTC_ASM_IDENTIFIER,cColorBlack);
	this->StyleSetForeground(wxSTC_ASM_CPUINSTRUCTION,cColorDirective);
	this->StyleSetForeground(wxSTC_ASM_MATHINSTRUCTION,cColorKeyword);
	this->StyleSetForeground(wxSTC_ASM_REGISTER,cColorBlack);
	this->StyleSetForeground(wxSTC_ASM_DIRECTIVE,cColorBlack);
	this->StyleSetForeground(wxSTC_ASM_DIRECTIVEOPERAND,cColorBlack);
	this->StyleSetForeground(wxSTC_ASM_COMMENTBLOCK,cColorComment);
	this->StyleSetForeground(wxSTC_ASM_CHARACTER,cColorBlack);
	this->StyleSetForeground(wxSTC_ASM_STRINGEOL,cColorBlack);
	this->StyleSetForeground(wxSTC_ASM_EXTINSTRUCTION,cColorBlack);
	// special bold request
	this->StyleSetBold(wxSTC_ASM_CPUINSTRUCTION,true);
}

void my1CodeEdit::SetEditStyle(int aSize)
{
	// prepare lexer keywords
	wxString cDirective = wxT("org end equ dfb db dfs ds");
	wxString cOpCode = wxT("mov mvi lxi out in cma cmc stc di ei");
	cOpCode += wxT(" lda ldax lhld pchl shld sphl sta stax xchg xthl");
	cOpCode += wxT(" add adc adi aci sub sbb sui sbi");
	cOpCode += wxT(" ana ani cmp cpi ora ori xra xri");
	cOpCode += wxT(" call cc cz cp cpe cnc cnz cm cpo");
	cOpCode += wxT(" ret rc rz rp rpe rnc rnz rm rpo");
	cOpCode += wxT(" jmp jc jz jp jpe jnc jnz jm jpo");
	cOpCode += wxT(" dad daa dcr dcx inr inx ral rrc rar rlc");
	cOpCode += wxT(" hlt nop pop push rim sim rst");
	// font type/size
	wxFont cFont(aSize,wxFONTFAMILY_TELETYPE,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	this->SetFont(cFont);
	this->SetFontSize(aSize);
	this->SetKeywordColor();
	// set lexer keywords
	this->SetKeyWords(KEYWORD_DIRECTIVE,cDirective);
	this->SetKeyWords(KEYWORD_INSTRUCTION,cOpCode); 
}

void my1CodeEdit::OnCodeChanged(wxStyledTextEvent &event)
{
	if(mModifyChecked) return;
	if(this->GetModify())
	{
		wxAuiNotebook* cNoteBook = (wxAuiNotebook*) mParent;
		int cSelect = cNoteBook->GetSelection();
		cNoteBook->SetPageText(cSelect,
			this->GetModFileName(cNoteBook->GetPageText(cSelect)));
		mModifyChecked = true;
	}
}

void my1CodeEdit::OnCodeMarked(wxStyledTextEvent &event)
{
	int cLine = this->LineFromPosition(event.GetPosition());
#ifdef DO_MINGW
//	cLine--; // NEED THIS ON WIN32-VBOX BUT NOT ON REAL WIN32!??
#endif
	this->ToggleBreak(cLine);
}

void my1CodeEdit::OnMouseClick(wxStyledTextEvent &event)
{
	if(event.GetEventType()==wxEVT_STC_DOUBLECLICK)
	{
		mLargeFont = !mLargeFont;
		if(mLargeFont)
			this->SetFontSize(LARGE_FONT_SIZE);
		else
			this->SetFontSize(DEFAULT_FONT_SIZE);
		this->SetSelection(0,0);
	}
}
