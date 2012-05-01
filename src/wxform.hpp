/**
*
* wxform.hpp
*
* - header for main wx-based form
*
**/

#ifndef __MY1FORM_HPP__
#define __MY1FORM_HPP__

#include "wx/wx.h"
#include "wx/arrstr.h" // command history list
#include "wx/grid.h" // miniviewer needs this
#include "wx/aui/aui.h" // duh!
#include "wxpref.hpp"
#include "my1sim85.hpp"

#define MY1APP_TITLE "MY1 SIM85"
#define MY1APP_PROGNAME "my1sim85"
#ifndef MY1APP_PROGVERS
#define MY1APP_PROGVERS "build"
#endif
#define PRINT_BPL_COUNT 0x10
#define CMD_HISTORY_COUNT 10

#define MY1ID_MAIN_OFFSET wxID_HIGHEST+1
#define MY1ID_DSEL_OFFSET wxID_HIGHEST+501
#define MY1ID_DEVC_OFFSET 0
#define MY1ID_DBIT_OFFSET 280
#define MY1ID_CBIT_OFFSET (MY1ID_DSEL_OFFSET+MY1ID_DBIT_OFFSET)
#define MY1ID_PORT_OFFSET 80
#define MY1ID_CPOT_OFFSET (MY1ID_DSEL_OFFSET+MY1ID_PORT_OFFSET)

enum {
	MY1ID_EXIT = MY1ID_MAIN_OFFSET,
	MY1ID_MAIN,
	MY1ID_NEW,
	MY1ID_LOAD,
	MY1ID_SAVE,
	MY1ID_SAVEAS,
	MY1ID_VIEW_REGSPANE,
	MY1ID_VIEW_INTRPANE,
	MY1ID_VIEW_CONSPANE,
	MY1ID_ASSEMBLE,
	MY1ID_SIMULATE,
	MY1ID_GENERATE,
	MY1ID_OPTIONS,
	MY1ID_ABOUT,
	MY1ID_FILETOOL,
	MY1ID_EDITTOOL,
	MY1ID_PROCTOOL,
	MY1ID_STAT_TIMER,
	MY1ID_SIMX_TIMER,
	MY1ID_CONSCOMM,
	MY1ID_CONSEXEC,
	MY1ID_SIMSEXEC,
	MY1ID_SIMSSTEP,
	MY1ID_SIMSINFO,
	MY1ID_SIMSPREV,
	MY1ID_SIMRESET,
	MY1ID_SIMSMIMV,
	MY1ID_SIMSBRKP,
	MY1ID_SIMSEXIT,
	MY1ID_BUILDINIT,
	MY1ID_BUILDRST,
	MY1ID_BUILDDEF,
	MY1ID_BUILDNFO,
	MY1ID_BUILDROM,
	MY1ID_BUILDRAM,
	MY1ID_BUILDPPI,
	MY1ID_BUILDOUT,
	MY1ID_CREATE_MINIMV,
	MY1ID_CREATE_DV7SEG,
	MY1ID_CREATE_DVKPAD,
	MY1ID_CREATE_DEVLED,
	MY1ID_CREATE_DEVSWI,
	MY1ID_DUMMY
};

struct my1BitSelect
{
	int mDevice;
	int mDevicePort;
	int mDeviceBit;
	int mDeviceAddr;
	void* mPointer;
	my1BitSelect():mDevice(0),mDevicePort(0),mDeviceBit(0),
		mDeviceAddr(0),mPointer(0x0){}
	my1BitSelect(int anIndex) { this->UseIndex(anIndex); }
	my1BitSelect(int anID, void* aPointer) { this->UseSystem(anID,aPointer); }
	void UseIndex(int anIndex)
	{
		mDeviceBit = anIndex%I8255_DATASIZE;
		anIndex = anIndex/I8255_DATASIZE;
		mDevicePort = anIndex%(I8255_SIZE-1);
		mDevice = anIndex/(I8255_SIZE-1);
		mDeviceAddr = 0;
		mPointer = 0x0;
	}
	void UseSystem(int anID,void* aPointer)
	{
		mDevice = -1;
		mDevicePort = -1;
		mDeviceBit = anID;
		mDeviceAddr = -1;
		mPointer = aPointer;
	}
	int GetIndex(void)
	{
		int cIndex = mDevice*(I8255_SIZE-1)*I8255_DATASIZE;
		cIndex += mDevicePort*I8255_DATASIZE;
		cIndex += mDeviceBit;
		return cIndex;
	}
	void operator=(my1BitSelect& aSelect)
	{
		mDevice = aSelect.mDevice;
		mDevicePort = aSelect.mDevicePort;
		mDeviceBit = aSelect.mDeviceBit;
		mDeviceAddr = aSelect.mDeviceAddr;
		mPointer = aSelect.mPointer;
	}
};

struct my1MiniViewer
{
	int mStart, mSize;
	my1Memory* pMemory;
	wxGrid* pGrid;
	my1MiniViewer* mNext;
	bool IsSelected(int anAddress)
	{
		if(anAddress>=this->mStart&&anAddress<this->mStart+this->mSize)
			return true;
		return false;
	}
};

class my1Form : public wxFrame
{
private:
	friend class my1CodeEdit;
	bool mSimulationRunning, mSimulationStepping;
	double mSimulationCycle, mSimulationCycleDefault; // smallest time res?
	unsigned long mSimulationDelay; // in microsec!
	bool mSimulationMode, mBuildMode;
	my1Sim85 m8085;
	my1SimObject mFlagLink[I8085_BIT_COUNT];
	my1MiniViewer *mFirstViewer;
	wxArrayString mCmdHistory;
	int mCmdHistIndex;
	wxAuiManager mMainUI;
	my1Options mOptions;
	wxTimer *mDisplayTimer;
	wxTimer *mSimExecTimer;
	wxAuiNotebook *mNoteBook;
	wxTextCtrl *mConsole;
	wxTextCtrl *mCommand;
	wxMenu *mDevicePopupMenu;
	wxMenu *mDevicePortMenu;
	wxStreamToTextRedirector *mRedirector;
	wxPanel *mPortPanel, *mLEDPanel, *mSWIPanel;
public:
	my1Form(const wxString& title);
	~my1Form();
	void CalculateSimCycle(void);
	bool ScaleSimCycle(double);
	double GetSimCycle(void);
	unsigned long GetSimDelay(void); // in microsec!
	void SimulationMode(bool aGo=true);
	void BuildMode(bool aGo=true);
	bool GetUniqueName(wxString&);
	bool LinkPanelToPort(wxPanel*,int);
protected:
	wxAuiToolBar* CreateFileToolBar(void);
	wxAuiToolBar* CreateEditToolBar(void);
	wxAuiToolBar* CreateProcToolBar(void);
	//wxBoxSizer* CreateFLAGView(wxWindow*,const wxString&,int);
	//wxBoxSizer* CreateREGSView(wxWindow*,const wxString&,int);
	wxPanel* CreateMainPanel(wxWindow*);
	wxPanel* CreateRegsPanel(void);
	wxPanel* CreateInterruptPanel(void);
	wxPanel* CreateConsPanel(void);
	wxPanel* CreateSimsPanel(void);
	wxPanel* CreateBuildPanel(void);
	wxPanel* CreateConsolePanel(wxWindow*);
	wxPanel* CreateMemoryPanel(wxWindow*);
	wxPanel* CreateMemoryGridPanel(wxWindow*,int,int,int,wxGrid**);
	wxPanel* CreateMemoryMiniPanel(int anAddress=-1);
	wxPanel* CreateDevice7SegPanel(void);
	wxPanel* CreateDeviceKPadPanel(void);
	wxPanel* CreateDeviceLEDPanel(void);
	wxPanel* CreateDeviceSWIPanel(void);
public:
	void OpenEdit(wxString&);
	void SaveEdit(wxWindow*, bool aSaveAs=false);
	void ShowStatus(wxString&);
	void OnQuit(wxCommandEvent &event);
	void OnNew(wxCommandEvent &event);
	void OnLoad(wxCommandEvent &event);
	void OnSave(wxCommandEvent &event);
	void OnAbout(wxCommandEvent &event);
	void OnAssemble(wxCommandEvent &event);
	void OnSimulate(wxCommandEvent &event);
	void OnGenerate(wxCommandEvent &event);
public:
	void PrintValueDEC(int,int,bool aNewline=false);
	void PrintValueHEX(int,int,bool aNewline=false);
	void PrintMessage(const wxString&,bool aNewline=false);
	void PrintTaggedMessage(const wxString&,const wxString&);
	void PrintInfoMessage(const wxString&);
	void PrintErrorMessage(const wxString&);
	void PrintAddressMessage(const wxString&,unsigned long);
	void PrintMemoryContent(aword, int aSize=PRINT_BPL_COUNT);
	void PrintPeripheralInfo(void);
	void PrintSimInfo(void);
	void PrintHelp(void);
	void PrintUnknownCommand(const wxString&);
	void PrintUnknownParameter(const wxString&,const wxString&);
public:
	void OnCheckConsole(wxKeyEvent &event);
	void OnExecuteConsole(wxCommandEvent &event);
	void OnSimulationPick(wxCommandEvent &event);
	void OnSimulationInfo(wxCommandEvent &event);
	void OnSimulationExit(wxCommandEvent &event);
	int GetBuildAddress(const wxString&);
	void OnBuildSelect(wxCommandEvent &event);
	void OnClosePane(wxAuiManagerEvent &event);
	void OnShowPanel(wxCommandEvent &event);
	void OnCheckOptions(wxCommandEvent &event);
	void OnStatusTimer(wxTimerEvent &event);
	void OnSimExeTimer(wxTimerEvent &event);
	void OnPageChanging(wxAuiNotebookEvent &event);
	void OnPageChanged(wxAuiNotebookEvent &event);
	void OnPageClosing(wxAuiNotebookEvent &event);
	void OnBITPanelClick(wxMouseEvent &event);
	void OnBITPortClick(wxCommandEvent &event);
public:
	my1BitIO* GetDeviceBit(my1BitSelect&);
	void UpdateDeviceBit(bool unLink=false);
	wxMenu* GetDevicePopupMenu(void);
	void ResetDevicePopupMenu(bool unLink=false);
	wxMenu* GetDevicePortMenu(void);
	void SimUpdateFLAG(void*);
	my1SimObject& FlagLink(int);
public: // 'wrapper' function
	bool SystemDefault(void);
	bool SystemReset(void);
	bool AddROM(int aStart=I2764_INIT);
	bool AddRAM(int aStart=I6264_INIT);
	bool AddPPI(int aStart=I8255_INIT);
public:
	static void SimUpdateREG(void*);
	static void SimUpdateMEM(void*);
	static void SimDoUpdate(void*);
	static void SimDoDelay(void*,int);
};

#endif
