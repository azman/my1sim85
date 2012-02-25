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
#include <wx/stc/stc.h>
#include "my1sim85.hpp"
#include "wxpref.hpp"

#define MY1APP_TITLE "MY1 SIM85"
#define MY1APP_PROGNAME "my1sim85"
#define MY1APP_PROGVERS "0.1.0"
#define PRINT_BPL_COUNT 0x10

enum {
	MY1ID_EXIT = wxID_HIGHEST+1,
	MY1ID_MAIN,
	MY1ID_NEW,
	MY1ID_LOAD,
	MY1ID_SAVE,
	MY1ID_VIEW_INITPAGE,
	MY1ID_VIEW_INFOPANE,
	MY1ID_VIEW_LOGSPANE,
	MY1ID_ASSEMBLE,
	MY1ID_OPTIONS,
	MY1ID_ABOUT,
	MY1ID_FILETOOL,
	MY1ID_EDITTOOL,
	MY1ID_PROCTOOL,
	MY1ID_INFOPANEL,
	MY1ID_SIMSPANEL,
	MY1ID_LOGSPANEL,
	MY1ID_CODEBOOK,
	MY1ID_LOGBOOK,
	MY1ID_STAT_TIMER,
	MY1ID_SIMX_TIMER,
	MY1ID_LEDPANEL,
	MY1ID_SWIPANEL,
	MY1ID_INFOBOOK,
	MY1ID_CONSCOMM,
	MY1ID_CONSEXEC,
	MY1ID_SIMSEXEC,
	MY1ID_SIMSSTEP,
	MY1ID_SIMSINFO,
	MY1ID_SIMSEXIT,
	MY1ID_DUMMY
};

class my1Form : public wxFrame
{
private:
	friend class my1CodeEdit;
	bool mSimulationMode;
	bool mSimulationRunning, mSimulationStepping;
	double mSimulationCycle, mSimulationCycleDefault; // smallest time resolution?
	my1Sim85 m8085;
	my1SimObject mFlagLink[I8085_BIT_COUNT];
	wxAuiManager mMainUI;
	my1Options mOptions;
	wxTimer* mDisplayTimer;
	wxTimer* mSimExecTimer;
	wxAuiNotebook *mNoteBook;
	wxTextCtrl *mConsole;
	wxTextCtrl *mCommand;
	wxAuiToolBar* CreateFileToolBar(void);
	wxAuiToolBar* CreateEditToolBar(void);
	wxAuiToolBar* CreateProcToolBar(void);
	wxPanel* CreateMainPanel(wxWindow *parent=0x0);
	wxPanel* CreateInfoPanel(void);
	wxPanel* CreateSimsPanel(void);
	wxPanel* CreateLogsPanel(void);
	wxBoxSizer* CreateFLAGView(wxWindow*,const wxString&,int);
	wxBoxSizer* CreateREGView(wxWindow*,const wxString&,int);
	wxBoxSizer* CreateLEDView(wxWindow*,const wxString&,int);
	wxBoxSizer* CreateSWIView(wxWindow*,const wxString&,int);
	wxPanel* CreateREGPanel(wxWindow*);
	wxPanel* CreateDEVPanel(wxWindow*);
	wxPanel* CreateMEMPanel(wxWindow*);
public:
	my1Form(const wxString& title);
	~my1Form();
	void CalculateSimCycle(void);
	bool ScaleSimCycle(double);
	double GetSimCycle(void);
	void SimulationMode(bool aGo=true);
	void OpenEdit(wxString&);
	void SaveEdit(wxWindow*);
	void ShowStatus(wxString&);
	void OnQuit(wxCommandEvent &event);
	void OnNew(wxCommandEvent &event);
	void OnLoad(wxCommandEvent &event);
	void OnSave(wxCommandEvent &event);
	void OnAbout(wxCommandEvent &event);
	void OnAssemble(wxCommandEvent &event);
	void PrintMemoryContent(aword, int aSize=PRINT_BPL_COUNT);
	void PrintPeripheralInfo(void);
	void PrintSimInfo(void);
	void PrintConsoleMessage(const wxString&);
	void PrintSimChangeStart(unsigned long,bool anError=false);
	void PrintBuildAdd(const wxString&, unsigned long);
	void PrintHelp(void);
	void PrintUnknownCommand(const wxString&);
	void PrintUnknownParameter(const wxString&,const wxString&);
	void OnExecuteConsole(wxCommandEvent &event);
	void OnSimulate(wxCommandEvent &event);
	void OnSimulationInfo(wxCommandEvent &event);
	void OnSimulationExit(wxCommandEvent &event);
	void OnClosePane(wxAuiManagerEvent &event);
	void OnShowInitPage(wxCommandEvent &event);
	void OnShowToolBar(wxCommandEvent &event);
	void OnShowPanel(wxCommandEvent &event);
	void OnCheckOptions(wxCommandEvent &event);
	void OnStatusTimer(wxTimerEvent &event);
	void OnSimExeTimer(wxTimerEvent &event);
	void OnPageChanged(wxAuiNotebookEvent &event);
	void OnPageClosing(wxAuiNotebookEvent &event);
	void SimUpdateFLAG(void*);
	static void SimUpdateREG(void*);
	static void SimDoUpdate(void*);
	static void SimDoDelay(void*,int);
};

#endif
