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
#include "wxpref.hpp"

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
	MY1ID_NEW,
	MY1ID_LOAD,
	MY1ID_SAVE,
	MY1ID_VIEW_INITPAGE,
	MY1ID_VIEW_INFOPANE,
	MY1ID_VIEW_CONSPANE,
	MY1ID_VIEW_LOGSPANE,
	MY1ID_VIEW_FILETOOL,
	MY1ID_VIEW_PROCTOOL,
	MY1ID_ASSEMBLE,
	MY1ID_OPTIONS,
	MY1ID_ABOUT,
	MY1ID_CLOSEPANE,
	MY1ID_FILETOOLBAR,
	MY1ID_PROCTOOLBAR,
	MY1ID_INFOPANEL,
	MY1ID_CONSPANEL,
	MY1ID_LOGSPANEL,
	MY1ID_CODEBOOK,
	MY1ID_LOGBOOK,
	MY1ID_STAT_TIMER,
	MY1ID_DUMMY
};

class my1Form : public wxFrame
{
private:
	my1Sim85 m8085;
	wxAuiManager mMainUI;
	my1Options mOptions;
	wxTimer* mDisplayTimer;
	wxAuiNotebook* mNoteBook;
	wxAuiToolBar* CreateFileToolBar(void);
	wxAuiToolBar* CreateProcToolBar(void);
	wxPanel* CreateMainPanel(wxWindow *parent=0x0);
	wxPanel* CreateInfoPanel(void);
	wxPanel* CreateConsPanel(void);
	wxPanel* CreateLogsPanel(void);
public:
	my1Form(const wxString &title);
	~my1Form();
	void OpenEdit(wxString&);
	void SaveEdit(wxWindow*);
	void ShowStatus(wxString&);
	void OnQuit(wxCommandEvent &event);
	void OnLoad(wxCommandEvent &event);
	void OnSave(wxCommandEvent &event);
	void OnMouseClick(wxMouseEvent &event);
	void OnClosePane(wxAuiManagerEvent &event);
	void OnShowInitPage(wxCommandEvent &event);
	void OnShowToolBar(wxCommandEvent &event);
	void OnShowPanel(wxCommandEvent &event);
	void OnCheckOptions(wxCommandEvent &event);
	void OnStatusTimer(wxTimerEvent &event);
	void OnPageChanged(wxAuiNotebookEvent &event);
};

#endif
