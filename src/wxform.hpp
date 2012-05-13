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
#include "wxmain.hpp"

#define MACRO_WXBMP(bmp) wxBitmap(bmp##_xpm)
#define MACRO_WXICO(bmp) wxIcon(bmp##_xpm)

#define MY1APP_TITLE "MY1 SIM85"
#define MY1APP_PROGNAME "my1sim85"
#ifndef MY1APP_PROGVERS
#define MY1APP_PROGVERS "build"
#endif
#define PRINT_BPL_COUNT 0x10
#define CMD_HISTORY_COUNT 10

#define MY1ID_MAIN_OFFSET (wxID_HIGHEST+1)
#define MY1ID_DSEL_OFFSET (MY1ID_MAIN_OFFSET+500)
#define MY1ID_CPOT_OFFSET (MY1ID_DSEL_OFFSET+80)
#define MY1ID_CBIT_OFFSET (MY1ID_DSEL_OFFSET+280)
#define MY1ID_8085_OFFSET (MY1ID_DSEL_OFFSET+1600)

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
	MY1ID_WHATSNEW,
	MY1ID_FILETOOL,
	MY1ID_EDITTOOL,
	MY1ID_PROCTOOL,
	MY1ID_DEVCTOOL,
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
	MY1ID_BUILDRST,
	MY1ID_BUILDDEF,
	MY1ID_BUILDNFO,
	MY1ID_BUILDROM,
	MY1ID_BUILDRAM,
	MY1ID_BUILDPPI,
	MY1ID_CREATE_MINIMV,
	MY1ID_CREATE_DV7SEG,
	MY1ID_CREATE_DVKPAD,
	MY1ID_CREATE_DEVLED,
	MY1ID_CREATE_DEVSWI,
	MY1ID_CREATE_DEVBUT,
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
	bool mSimulationMode, mSimulationRunning, mSimulationStepping;
	my1Sim85 m8085;
	my1SimObject mFlagLink[I8085_BIT_COUNT];
	my1MiniViewer *mFirstViewer;
	my1App* myApp;
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
	wxGrid *mMemoryGrid;
	wxStreamToTextRedirector *mRedirector;
	wxString mThisPath;
	wxPanel *mPortPanel, *mLEDPanel, *mSWIPanel;
public:
	my1Form(const wxString& title, const my1App* p_app);
	~my1Form();
	void SimulationMode(bool aGo=true);
	void BuildMode(bool aGo=true);
	bool GetUniqueName(wxString&);
	bool LinkPanelToPort(wxPanel*,int);
protected:
	wxAuiToolBar* CreateFileToolBar(void);
	wxAuiToolBar* CreateEditToolBar(void);
	wxAuiToolBar* CreateProcToolBar(void);
	wxAuiToolBar* CreateDevcToolBar(void);
	wxPanel* CreateMainPanel(wxWindow*);
	wxPanel* CreateRegsPanel(void);
	wxPanel* CreateInterruptPanel(void);
	wxPanel* CreateConsPanel(void);
	wxPanel* CreateSimsPanel(void);
	wxPanel* CreateConsolePanel(wxWindow*);
	wxPanel* CreateMemoryPanel(wxWindow*);
	wxPanel* CreateMemoryGridPanel(wxWindow*,int,int,int,wxGrid**);
	wxPanel* CreateMemoryMiniPanel(int anAddress=-1);
	wxPanel* CreateDevice7SegPanel(void);
	wxPanel* CreateDeviceKPadPanel(void);
	wxPanel* CreateDeviceLEDPanel(void);
	wxPanel* CreateDeviceSWIPanel(void);
	wxPanel* CreateDeviceBUTPanel(void);
public:
	void OpenEdit(wxString&);
	void SaveEdit(wxWindow*, bool aSaveAs=false);
	void ShowStatus(wxString&);
	void OnFormClose(wxCloseEvent &event);
	void OnQuit(wxCommandEvent &event);
	void OnNew(wxCommandEvent &event);
	void OnLoad(wxCommandEvent &event);
	void OnSave(wxCommandEvent &event);
	void OnAbout(wxCommandEvent &event);
	void OnWhatsNew(wxCommandEvent &event);
	void OnAssemble(wxCommandEvent &event);
	void OnSimulate(wxCommandEvent &event);
	void OnGenerate(wxCommandEvent &event);
public:
	void PrintMessage(const wxString&,bool aNewline=false);
	void PrintTaggedMessage(const wxString&,const wxString&,
		const wxColor& aTagColor=wxNullColour);
	void PrintInfoMessage(const wxString&);
	void PrintErrorMessage(const wxString&);
	void PrintValueDEC(int,int aWidth=0);
	void PrintValueHEX(int,int);
	void PrintMemoryContent(aword, int aSize=PRINT_BPL_COUNT);
	void PrintPeripheralInfo(void);
	void PrintHelp(void);
	void PrintUnknownCommand(const wxString&);
	void PrintUnknownParameter(const wxString&,const wxString&);
public:
	void OnCheckFont(wxKeyEvent &event);
	void OnCheckConsole(wxKeyEvent &event);
	void OnExecuteConsole(wxCommandEvent &event);
	void OnSimulationPick(wxCommandEvent &event);
	void OnSimulationInfo(wxCommandEvent &event);
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
	void UpdateMemoryPanel(void);
	void SimUpdateFLAG(void*);
	my1SimObject& FlagLink(int);
public: // 'wrapper' function
	bool SystemDefault(void);
	bool SystemDisconnect(void);
	bool ConnectROM(int aStart=I2764_INIT);
	bool ConnectRAM(int aStart=I6264_INIT);
	bool ConnectPPI(int aStart=I8255_INIT);
public:
	static void SimUpdateREG(void*);
	static void SimUpdateMEM(void*);
	static void SimDoUpdate(void*);
	static void SimDoDelay(void*,int);
};

#endif
