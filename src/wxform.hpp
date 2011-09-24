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
#ifdef __WXMSW__
	#define USE_XPM_BITMAPS 0
	#define MACRO_WXBMP(bmp) wxBITMAP(bmp)
	#define MACRO_WXICO(bmp) wxICON(bmp)
#else
	#define USE_XPM_BITMAPS 1
	#define MACRO_WXBMP(bmp) wxBitmap(bmp##_xpm)
	#define MACRO_WXICO(bmp) wxIcon(bmp##_xpm)
#endif

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
	MY1ID_SIM_TIMER,
	MY1ID_LEDPANEL,
	MY1ID_SWIPANEL,
	MY1ID_INFOBOOK,
	MY1ID_CONSCOMM,
	MY1ID_CONSEXEC,
	MY1ID_SIMSEXEC,
	MY1ID_SIMSSTEP,
	MY1ID_SIMSINFO,
	MY1ID_SIMSEXIT,
	MY1ID_REGB_VAL,
	MY1ID_REGC_VAL,
	MY1ID_REGD_VAL,
	MY1ID_REGE_VAL,
	MY1ID_REGH_VAL,
	MY1ID_REGL_VAL,
	MY1ID_REGA_VAL,
	MY1ID_REGF_VAL,
	MY1ID_REGPC_VAL,
	MY1ID_REGSP_VAL,
	MY1ID_LED0_VAL,
	MY1ID_LED1_VAL,
	MY1ID_LED2_VAL,
	MY1ID_LED3_VAL,
	MY1ID_LED4_VAL,
	MY1ID_LED5_VAL,
	MY1ID_LED6_VAL,
	MY1ID_LED7_VAL,
	MY1ID_SWI0_VAL,
	MY1ID_SWI1_VAL,
	MY1ID_SWI2_VAL,
	MY1ID_SWI3_VAL,
	MY1ID_SWI4_VAL,
	MY1ID_SWI5_VAL,
	MY1ID_SWI6_VAL,
	MY1ID_SWI7_VAL,
	MY1ID_DUMMY
};

class my1Form : public wxFrame
{
private:
	friend class my1CodeEdit;
	bool mSimulationRun;
	double mSimulationCycle, mSimulationCycleDefault; // smallest time resolution?
	my1Sim85 m8085;
	wxAuiManager mMainUI;
	my1Options mOptions;
	wxTimer* mDisplayTimer;
	wxTimer* mSimulationTimer;
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
	wxBoxSizer* CreateREGView(wxWindow*,const wxString&,int,bool aReg16=false);
	wxBoxSizer* CreateLEDView(wxWindow*,const wxString&,int);
	wxBoxSizer* CreateSWIView(wxWindow*,const wxString&,int);
	wxPanel* CreateREGPanel(wxWindow*);
	wxPanel* CreateDEVPanel(wxWindow*);
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
	void UpdateRegValue(wxWindow*, int, bool aReg16=false);
	void UpdateLEDValue(wxWindow*, int);
	void UpdateFromSWI(wxWindow*, int);
	void OnQuit(wxCommandEvent &event);
	void OnNew(wxCommandEvent &event);
	void OnLoad(wxCommandEvent &event);
	void OnSave(wxCommandEvent &event);
	void OnAbout(wxCommandEvent &event);
	void OnAssemble(wxCommandEvent &event);
	void PrintPeripheralInfo(void);
	void PrintSimInfo(void);
	void PrintConsoleMessage(const wxString&);
	void PrintSimChangeStart(unsigned long,bool anError=false);
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
	void OnSimTimer(wxTimerEvent &event);
	void OnPageChanged(wxAuiNotebookEvent &event);
	void OnPageClosing(wxAuiNotebookEvent &event);
	static void SimDoUpdate(void*);
	static void SimDoDelay(void*);
	static void SimDoSwitch(void*);
};

#endif
