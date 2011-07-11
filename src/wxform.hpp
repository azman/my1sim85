/**
*
* wxform.hpp
*
* - header for main wx-based form
*
**/

#ifndef __MY1FORM_HPP__
#define __MY1FORM_HPP__

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include "my1sim85.hpp"

#define MY1APP_TITLE "MY1 SIM85"
#ifdef __WXMSW__
    #define USE_XPM_BITMAPS 0
	#define MY1APP_ICON "apps"
#else
    #define USE_XPM_BITMAPS 1
	#define MY1APP_ICON "res/apps.xpm"
#endif

enum {
	MY1ID_EXIT = wxID_HIGHEST+1,
	MY1ID_CLEAR,
	MY1ID_LOAD,
	MY1ID_SAVE,
	MY1ID_GENERATE,
	MY1ID_OPTIONS,
	MY1ID_TOOL_CUSTOMIZE,
	MY1ID_CODEPAGE_CHANGE,
	MY1ID_CODE_PANEL,
	MY1ID_CODE_BOOK,
	MY1ID_CODE_EDIT
};

class my1Form : public wxFrame
{
private:
	my1Sim85 m8085;
    long mNoteStyle, mNoteTheme;
	wxAuiManager mFaceMan;
	wxWindow* GetCodeBook(void);
	wxWindow* GetCodeEdit(wxWindow*); 
	wxWindow* GetChildID(wxWindow*, int); 
	wxString HTMLIntroduction(void);
public:
	my1Form(const wxString &title);
	~my1Form();
	void OnQuit(wxCommandEvent &event);
	void OnClear(wxCommandEvent &event);
	void OnLoad(wxCommandEvent &event);
	void OnSave(wxCommandEvent &event);
	void OnGenerate(wxCommandEvent &event);
	void OnCheckOptions(wxCommandEvent &event);
	void OnPaint(wxPaintEvent &event);
	void OnMouseClick(wxMouseEvent &event);
	void OnMouseMove(wxMouseEvent &event);
	void OnMouseLeave(wxMouseEvent &event);
};

#endif
