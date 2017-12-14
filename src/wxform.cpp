/**
*
* wxform.cpp
*
* - implementation for main wx-based form
*
**/

#define ABOUT_TITLE "8085 Microprocessor System Simulator"
#define ABOUT_COPYRIGHT "(C) 2011-2015 Azman M. Yusof"
#define ABOUT_WEBSITE "http://www.my1matrix.org"
#define ABOUT_AUTHOR "Azman M. Yusof <azman@my1matrix.org>"

#include "wxform.hpp"
#include "wxpanel.hpp"
#include "wxcode.hpp"
#include "wxled.hpp"
#include "wxswitch.hpp"
#include "wx/gbsizer.h"
#include "wx/aboutdlg.h"
#include "wx/textfile.h"
#include "wx/wfstream.h"
#include "wx/fileconf.h"
#include "wx/stdpaths.h"
#include "wx/filefn.h"

#include "../res/apps.xpm"
#include "../res/exit.xpm"
#include "../res/newd.xpm"
#include "../res/open.xpm"
#include "../res/save.xpm"
#include "../res/binary.xpm"
#include "../res/option.xpm"
#include "../res/build.xpm"
#include "../res/hexgen.xpm"
#include "../res/simx.xpm"
#include "../res/target.xpm"
#include "../res/devled.xpm"
#include "../res/devswi.xpm"
#include "../res/devbut.xpm"
#include "../res/dv7seg.xpm"
#include "../res/dvkpad.xpm"

// handy alias
#define WX_CEH wxCommandEventHandler
#define WX_KEH wxKeyEventHandler
#define WX_MEH wxMouseEventHandler
#define WX_TEH wxTimerEventHandler

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define REGS_PANEL_WIDTH 180
#define REGS_HEADER_HEIGHT 30
#define CONS_PANEL_HEIGHT 150
#define INFO_REG_SPACER 5
#define SEG7_NUM_SPACER 5
#define DEVC_POP_SPACER 5
#define STATUS_COUNT 3
#define STATUS_FIX_WIDTH (REGS_PANEL_WIDTH+3)
#define STATUS_F2X_WIDTH 70
#define STATUS_SYS_INDEX 1
#define STATUS_MSG_INDEX 2
#define STATUS_MSG_PERIOD 3000
#define SIM_START_ADDR 0x0000
#define SIM_EXEC_PERIOD 1
#define TOOL_FILE_POS 0
#define TOOL_PROC_POS 1
#define TOOL_DEVC_POS 2
#define TITLE_FONT_SIZE 24
#define EMAIL_FONT_SIZE 8
#define PANEL_FONT_SIZE 10
#define INFO_FONT_SIZE 8
#define LOGS_FONT_SIZE 8
#define SIMS_FONT_SIZE 8
#define GRID_FONT_SIZE 8
#define CONS_FONT_SIZE 10
#define KPAD_FONT_SIZE 10
#define FLOAT_INIT_X 40
#define FLOAT_INIT_Y 40
#define MEM_VIEW_WIDTH 4
#define MEM_VIEW_HEIGHT (MAX_MEMSIZE/MEM_VIEW_WIDTH)
#define MEM_MINIVIEW_WIDTH 8
#define MEM_MINIVIEW_HEIGHT 4
#define DOT_SIZE 11
#define AUI_EXTER_LAYER 3
#define AUI_OUTER_LAYER 2
#define AUI_INNER_LAYER 1
#ifdef DO_MINGW
#define DEV_INIT_POS -1
#else
#define DEV_INIT_POS 0
#endif
#define BOT_CONS_POS 0
#define BOT_TERM_POS 1

#define MSG_SYSTEM_IDLE wxS("Inactive")
#define MSG_SYSTEM_MSIM wxS("Idle")
#define MSG_SYSTEM_SSIM wxS("Stepping")
#define MSG_SYSTEM_RSIM wxS("Running")

my1Form::my1Form(const wxString &title, const my1App* p_app)
	: wxFrame( NULL, MY1ID_MAIN, title, wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_FRAME_STYLE), myApp((my1App*)p_app)
{
	mShowSystem = false;
	// simulation stuffs
	mSimulationMode = false;
	mSimulationRunning = false;
	mSimulationStepping = false;
	// default option?
	mOptions.mChanged = false;
	mOptions.mEdit_ViewWS = false;
	mOptions.mEdit_ViewEOL = false;
	mOptions.mConv_UnixEOL = false;
	mOptions.mSims_ShowRunInfo = false;
	mOptions.mSims_PauseOnINTR = false;
	mOptions.mSims_PauseOnHALT = false;
	mOptions.mSims_StartADDR = SIM_START_ADDR;
	mOptions.mComp_DoList = false;
	// reset mini-viewers (link-list)
	mFirstViewer = 0x0;
	// minimum window size... duh!
	this->SetMinSize(wxSize(WIN_WIDTH,WIN_HEIGHT));
	// status bar
	this->CreateStatusBar(STATUS_COUNT);
	this->SetStatusText(wxS("Simulation System: "));
	this->SetStatusText(MSG_SYSTEM_IDLE,STATUS_SYS_INDEX);
	const int cWidths[STATUS_COUNT] = { STATUS_FIX_WIDTH,STATUS_F2X_WIDTH,-1 };
	wxStatusBar* cStatusBar = this->GetStatusBar();
	cStatusBar->SetStatusWidths(STATUS_COUNT,cWidths);
	// create timers
	mDisplayTimer = new wxTimer(this, MY1ID_STAT_TIMER);
	mSimExecTimer = new wxTimer(this, MY1ID_SIMX_TIMER);
	// console command history
	mCmdHistory.Clear();
	mCmdHistory.Alloc(CMD_HISTORY_COUNT+1);
	mCmdHistIndex = 0;
	// some handy pointers
	mConsole = 0x0;
	mCommand = 0x0;
	mTermCon = new my1Term(this,MY1ID_MAIN_TERM);
	mFileTool = 0x0;
	mDevicePopupMenu = 0x0;
	mDevicePortMenu = 0x0;
	mMemoryGrid = 0x0;
	mPortPanel = 0x0;
	// keeps pointers to dev panels
	mDevPanels.Clear();
	// setup image
	//wxInitAllImageHandlers();
	wxIcon mIconApps = MACRO_WXICO(apps);
	this->SetIcon(mIconApps);
	// menu bar
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(MY1ID_NEW, wxS("&New\tCTRL+N"));
	fileMenu->Append(MY1ID_LOAD, wxS("&Open\tCTRL+O"));
	fileMenu->Append(MY1ID_SAVE, wxS("&Save\tCTRL+S"));
	fileMenu->Append(MY1ID_SAVEAS, wxS("Save &As..."));
	fileMenu->AppendSeparator();
	fileMenu->Append(MY1ID_EXIT, wxS("E&xit\tCTRL+Q"), wxS("Quit program"));
	wxMenu *editMenu = new wxMenu;
	editMenu->Append(MY1ID_SYSTEM, wxS("&Build System"),
		wxEmptyString, wxITEM_CHECK);
	editMenu->AppendSeparator();
	editMenu->Append(MY1ID_OPTIONS, wxS("&Preferences..."));
	wxMenu *systMenu = new wxMenu;
	systMenu->Append(MY1ID_BUILDLOD, wxS("&Load System..."));
	systMenu->Append(MY1ID_BUILDSAV, wxS("&Save System..."));
	systMenu->AppendSeparator();
	systMenu->Append(MY1ID_CREATE_MINIMV, wxS("Create miniMV Panel"));
	systMenu->AppendSeparator();
	systMenu->Append(MY1ID_CREATE_DV7SEG, wxS("Create dv7SEG Panel"));
	systMenu->Append(MY1ID_CREATE_DVKPAD, wxS("Create dvKPAD Panel"));
	systMenu->Append(MY1ID_CREATE_DEVLED, wxS("Create devLED Panel"));
	systMenu->Append(MY1ID_CREATE_DEVSWI, wxS("Create devSWI Panel"));
	systMenu->Append(MY1ID_CREATE_DEVBUT, wxS("Create devBUT Panel"));
	systMenu->Append(MY1ID_CREATE_DEVLVD, wxS("Create devLED Panel (V)"));
	systMenu->AppendSeparator();
	systMenu->Append(MY1ID_VIEW_SYSTPANE, wxS("View System Panel"));
	systMenu->Append(MY1ID_VIEW_REGSPANE, wxS("View Register Panel"));
	systMenu->Append(MY1ID_VIEW_INTRPANE, wxS("View Interrupt Panel"));
	systMenu->Append(MY1ID_VIEW_CONSPANE, wxS("View Console Panel"));
	systMenu->Append(MY1ID_VIEW_TERMPANE, wxS("View Terminal Panel"));
	wxMenu *procMenu = new wxMenu;
	procMenu->Append(MY1ID_ASSEMBLE, wxS("&Assemble"));
	procMenu->Append(MY1ID_GENERATE, wxS("&Generate"));
	procMenu->Append(MY1ID_SIMULATE, wxS("&Simulate"));
	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(MY1ID_README, wxS("&ReadMe"), wxS("Some Information"));
	helpMenu->Append(MY1ID_WHATSNEW, wxS("&ChangeLog"), wxS("What's New?"));
	helpMenu->AppendSeparator();
	helpMenu->Append(MY1ID_ABOUT, wxS("&About"), wxS("About This Program"));
	wxMenuBar *mainMenu = new wxMenuBar;
	mainMenu->Append(fileMenu, wxS("&File"));
	mainMenu->Append(editMenu, wxS("&Edit"));
	mainMenu->Append(systMenu, wxS("&System"));
	mainMenu->Append(procMenu, wxS("&Tool"));
	mainMenu->Append(helpMenu, wxS("&Help"));
	this->SetMenuBar(mainMenu);
	mainMenu->EnableTop(mainMenu->FindMenu(wxS("Tool")),false);
	mainMenu->EnableTop(mainMenu->FindMenu(wxS("System")),mShowSystem);
	wxMenuItem *pMenuItem = mainMenu->FindItem(MY1ID_SIMULATE,0x0);
	if(pMenuItem) pMenuItem->Enable(mShowSystem);
	// using AUI manager...
	mMainUI.SetManagedWindow(this);
	// create notebook for main/editor panel
	mNoteBook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE);
	mNoteBook->AddPage(CreateInitPanel(mNoteBook), wxS("Welcome"), true);
	// create initial pane for main view
	mMainUI.AddPane(mNoteBook, wxAuiPaneInfo().Name(wxS("codeBook")).
		CenterPane().MaximizeButton(true).PaneBorder(false));
	// tool bar - file
	mMainUI.AddPane(CreateFileToolBar(), wxAuiPaneInfo().
		Name(wxS("fileTool")).Caption(wxS("File")).
		ToolbarPane().Top().Position(TOOL_FILE_POS).
		Floatable(false).BottomDockable(false));
	// tool bar - proc
	mMainUI.AddPane(CreateProcToolBar(), wxAuiPaneInfo().
		Name(wxS("procTool")).Caption(wxS("Process")).
		ToolbarPane().Top().Position(TOOL_PROC_POS).Show(false).
		Floatable(false).BottomDockable(false));
	// tool bar - device
	mMainUI.AddPane(CreateDevcToolBar(), wxAuiPaneInfo().
		Name(wxS("devcTool")).Caption(wxS("Devices")).
		ToolbarPane().Top().Position(TOOL_DEVC_POS).Show(mShowSystem).
		Floatable(false).BottomDockable(false));
	// reg panel
	mMainUI.AddPane(CreateRegsPanel(), wxAuiPaneInfo().
		Name(wxS("regsPanel")).Caption(wxS("Registers")).
		DefaultPane().Left().Layer(AUI_EXTER_LAYER).Show(mShowSystem).
		Dockable(false).LeftDockable(true).
		MinSize(wxSize(REGS_PANEL_WIDTH,0)));
	// interrupt panel
	mMainUI.AddPane(CreateIntrPanel(), wxAuiPaneInfo().
		Name(wxS("intrPanel")).Caption(wxS("Interrupts")).
		DefaultPane().Top().Show(mShowSystem).Dockable(false).TopDockable(true));
	// system panel
	mMainUI.AddPane(CreateMainPanel(), wxAuiPaneInfo().
		Name(wxS("systPanel")).Caption(wxS("System")).
		DefaultPane().Left().Layer(AUI_OUTER_LAYER).Show(mShowSystem).
		Dockable(false).LeftDockable(true));
	// sim panel
	mMainUI.AddPane(CreateSimsPanel(), wxAuiPaneInfo().
		Name(wxS("simsPanel")).Caption(wxS("Simulation")).
		DefaultPane().Left().Layer(AUI_INNER_LAYER).Show(mShowSystem).
		Dockable(false).LeftDockable(true).CloseButton(false));
	// log panel
	mMainUI.AddPane(CreateConsPanel(), wxAuiPaneInfo().MaximizeButton(true).
		Name(wxS("consPanel")).Caption(wxS("Console Panel")).
		DefaultPane().Bottom().Position(BOT_CONS_POS).Layer(AUI_OUTER_LAYER).
		Dockable(false).BottomDockable(true).
		MinSize(wxSize(0,CONS_PANEL_HEIGHT)));
	// tty panel
	mMainUI.AddPane(mTermCon, wxAuiPaneInfo().MaximizeButton(true).
		Name(wxS("termPanel")).Caption(wxS("Terminal Panel")).
		DefaultPane().Bottom().Position(BOT_TERM_POS).Layer(AUI_OUTER_LAYER).
		Dockable(false).BottomDockable(true).
		MinSize(wxSize(0,CONS_PANEL_HEIGHT)));
	// disable simulate tool by default
	wxAuiToolBar *pTool = (wxAuiToolBar*) this->FindWindow(MY1ID_PROCTOOL);
	pTool->EnableTool(MY1ID_SIMULATE,mShowSystem);
	// commit changes!
	mMainUI.Update();
	// actions & events! - (int, wxEventType, wxObjectEventFunction)
	this->Connect(wxEVT_CLOSE_WINDOW,wxCloseEventHandler(my1Form::OnFormClose));
	wxEventType cEventType = wxEVT_COMMAND_TOOL_CLICKED;
	this->Connect(MY1ID_EXIT,cEventType,WX_CEH(my1Form::OnQuit));
	this->Connect(MY1ID_LOAD,cEventType,WX_CEH(my1Form::OnLoad));
	this->Connect(MY1ID_SAVE,cEventType,WX_CEH(my1Form::OnSave));
	this->Connect(MY1ID_SAVEAS,cEventType,WX_CEH(my1Form::OnSave));
	this->Connect(MY1ID_NEW,cEventType,WX_CEH(my1Form::OnNew));
	this->Connect(MY1ID_ABOUT,cEventType,WX_CEH(my1Form::OnAbout));
	this->Connect(MY1ID_WHATSNEW,cEventType,WX_CEH(my1Form::OnWhatsNew));
	this->Connect(MY1ID_README,cEventType,WX_CEH(my1Form::OnReadMe));
	this->Connect(MY1ID_SYSTEM,cEventType,WX_CEH(my1Form::OnShowSystem));
	this->Connect(MY1ID_VIEW_SYSTPANE,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_VIEW_REGSPANE,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_VIEW_INTRPANE,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_VIEW_CONSPANE,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_VIEW_TERMPANE,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_OPTIONS,cEventType,WX_CEH(my1Form::OnCheckOptions));
	this->Connect(MY1ID_ASSEMBLE,cEventType,WX_CEH(my1Form::OnAssemble));
	this->Connect(MY1ID_SIMULATE,cEventType,WX_CEH(my1Form::OnSimulate));
	this->Connect(MY1ID_GENERATE,cEventType,WX_CEH(my1Form::OnGenerate));
	this->Connect(MY1ID_CREATE_MINIMV,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_CREATE_DV7SEG,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_CREATE_DVKPAD,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_CREATE_DEVLED,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_CREATE_DEVSWI,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_CREATE_DEVBUT,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_CREATE_DEVLVD,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_BUILDLOD,cEventType,WX_CEH(my1Form::OnSysLoad));
	this->Connect(MY1ID_BUILDSAV,cEventType,WX_CEH(my1Form::OnSysSave));
	cEventType = wxEVT_COMMAND_BUTTON_CLICKED;
	this->Connect(MY1ID_CONSEXEC,cEventType,WX_CEH(my1Form::OnExecuteConsole));
	this->Connect(MY1ID_SIMSEXEC,cEventType,WX_CEH(my1Form::OnSimulationPick));
	this->Connect(MY1ID_SIMSSTEP,cEventType,WX_CEH(my1Form::OnSimulationPick));
	this->Connect(MY1ID_SIMSINFO,cEventType,WX_CEH(my1Form::OnSimulationInfo));
	this->Connect(MY1ID_SIMSPREV,cEventType,WX_CEH(my1Form::OnSimulationInfo));
	this->Connect(MY1ID_SIMRESET,cEventType,WX_CEH(my1Form::OnSimulationInfo));
	this->Connect(MY1ID_SIMSMIMV,cEventType,WX_CEH(my1Form::OnSimulationInfo));
	this->Connect(MY1ID_SIMSBRKP,cEventType,WX_CEH(my1Form::OnSimulationInfo));
	this->Connect(MY1ID_SIMSEXIT,cEventType,WX_CEH(my1Form::OnSimulationInfo));
	this->Connect(MY1ID_BUILDRST,cEventType,WX_CEH(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDDEF,cEventType,WX_CEH(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDNFO,cEventType,WX_CEH(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDROM,cEventType,WX_CEH(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDRAM,cEventType,WX_CEH(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDPPI,cEventType,WX_CEH(my1Form::OnBuildSelect));
	cEventType = wxEVT_TIMER;
	this->Connect(MY1ID_STAT_TIMER,cEventType,WX_TEH(my1Form::OnStatusTimer));
	this->Connect(MY1ID_SIMX_TIMER,cEventType,WX_TEH(my1Form::OnSimExeTimer));
	// disable status bar showing helpstring
	this->Connect(wxID_ANY,wxEVT_MENU_HIGHLIGHT,
		wxMenuEventHandler(my1Form::OnMenuHighlight));
	// AUI-related events
	this->Connect(wxID_ANY,wxEVT_AUI_PANE_CLOSE,
		wxAuiManagerEventHandler(my1Form::OnClosePane));
	this->Connect(wxID_ANY,wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING,
		wxAuiNotebookEventHandler(my1Form::OnPageChanging));
	this->Connect(wxID_ANY,wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED,
		wxAuiNotebookEventHandler(my1Form::OnPageChanged));
	this->Connect(wxID_ANY,wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE,
		wxAuiNotebookEventHandler(my1Form::OnPageClosing));
	// get program path/name
	wxStandardPaths& cPaths = wxStandardPaths::Get();
	wxFileName cFullName(cPaths.GetExecutablePath());
	mThisPath = cFullName.GetPathWithSep();
	for (int loop=0;loop<this->myApp->argc;loop++)
	{
		wxString ptest = wxString(this->myApp->argv[loop]);
		if (ptest==wxS("--thispath"))
		{
			mThisPath = wxGetCwd();
			break;
		}
	}
	wxSetWorkingDirectory(mThisPath);
	// setup hotkeys?
	wxAcceleratorEntry hotKeys[7];
	hotKeys[0].Set(wxACCEL_NORMAL, WXK_F8, MY1ID_SIMSEXEC);
	hotKeys[1].Set(wxACCEL_NORMAL, WXK_F7, MY1ID_SIMSSTEP);
	hotKeys[2].Set(wxACCEL_NORMAL, WXK_F6, MY1ID_SIMRESET);
	hotKeys[3].Set(wxACCEL_NORMAL, WXK_F5, MY1ID_SIMSEXIT);
	hotKeys[4].Set(wxACCEL_CTRL, WXK_F7, MY1ID_SIMULATE);
	hotKeys[5].Set(wxACCEL_CTRL, WXK_F6, MY1ID_GENERATE);
	hotKeys[6].Set(wxACCEL_CTRL, WXK_F5, MY1ID_ASSEMBLE);
	wxAcceleratorTable hkTable(7,hotKeys);
	this->SetAcceleratorTable(hkTable);
	// position this!
	this->Maximize(); //this->Centre();
	// cold reset to randomize values
	m8085.Reset(true);
	// assign function pointers :p
	m8085.SetLink((void*)this);
	//m8085.DoUpdate = &this->SimDoUpdate;
	//m8085.DoDelay = &this->SimDoDelay;
	// try to redirect standard console to gui console
	mRedirector = new wxStreamToTextRedirector(mConsole);
	// scroll console to last line
	while(mConsole->ScrollPages(1));
	// let command prompt has focus
	mCommand->SetFocus();
}

my1Form::~my1Form()
{
	// cleanup system
	this->SystemDisconnect();
	// cleanup aui
	mMainUI.UnInit();
	// cleanup mini-viewers (dual-link-list?)
	while(mFirstViewer)
	{
		my1MiniViewer *pViewer = mFirstViewer;
		mFirstViewer = pViewer->mNext;
		delete pViewer;
	}
	// cleanup redirector if neccessary
	if(mRedirector) { delete mRedirector; mRedirector = 0x0; }
	// just in case... it's just a link!
	if(mPortPanel) mPortPanel = 0x0;
}

void my1Form::SimulationMode(bool aGo)
{
	wxAuiToolBar *cFileTool = (wxAuiToolBar*) this->FindWindow(MY1ID_FILETOOL);
	wxAuiToolBar *cProcTool = (wxAuiToolBar*) this->FindWindow(MY1ID_PROCTOOL);
	this->GetMenuBar()->Enable(!aGo);
	cFileTool->Enable(!aGo);
	cProcTool->Enable(!aGo);
	wxAuiPaneInfo& cPaneSims = mMainUI.GetPane(wxS("simsPanel"));
	if(cPaneSims.IsOk()) cPaneSims.Show(aGo);
	wxAuiPaneInfo& cPaneSyst = mMainUI.GetPane(wxS("systPanel"));
	if(cPaneSyst.IsOk()) cPaneSyst.Show(!aGo);
	if(aGo) this->SetStatusText(MSG_SYSTEM_MSIM,STATUS_SYS_INDEX);
	else this->SetStatusText(MSG_SYSTEM_IDLE,STATUS_SYS_INDEX);
	mMainUI.Update();
	mSimulationMode = aGo;
}

bool my1Form::GetUniqueName(wxString& aName)
{
	wxString cName;
	int cIndex = 0;
	while(cIndex<0x100) // 256 max
	{
		cName = aName + wxString::Format(wxS("%02X"),cIndex++);
		wxAuiPaneInfo& rPane = mMainUI.GetPane(cName);
		if(!rPane.IsOk())
		{
			aName = cName;
			return true;
		}
	}
	return false;
}

bool my1Form::LinkPanelToPort(wxPanel* aPanel,int anIndex)
{
	// use existing method!
	wxCommandEvent cEvent(wxEVT_COMMAND_MENU_SELECTED,
		MY1ID_CPOT_OFFSET+anIndex);
	mPortPanel = aPanel;
	this->OnBITPortClick(cEvent);
	return mPortPanel ? true : false;
}

wxAuiToolBar* my1Form::CreateFileToolBar(void)
{
	wxBitmap mIconExit = MACRO_WXBMP(exit);
	wxBitmap mIconNewd = MACRO_WXBMP(newd);
	wxBitmap mIconLoad = MACRO_WXBMP(open);
	wxBitmap mIconSave = MACRO_WXBMP(save);
	wxBitmap mIconOpts = MACRO_WXBMP(option);
	wxBitmap mIconBild = MACRO_WXBMP(build);
	wxAuiToolBar* fileTool = new wxAuiToolBar(this, MY1ID_FILETOOL,
		wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	fileTool->SetToolBitmapSize(wxSize(16,16));
	fileTool->AddTool(MY1ID_EXIT, wxS("Exit"), mIconExit, wxS("Exit"));
	fileTool->AddSeparator();
	fileTool->AddTool(MY1ID_NEW, wxS("Clear"), mIconNewd, wxS("New"));
	fileTool->AddTool(MY1ID_LOAD, wxS("Open"), mIconLoad, wxS("Open"));
	fileTool->AddTool(MY1ID_SAVE, wxS("Save"), mIconSave, wxS("Save"));
	fileTool->AddSeparator();
	fileTool->AddTool(MY1ID_SYSTEM, wxS("System"), mIconBild,
		wxS("System"), wxITEM_CHECK);
	fileTool->AddSeparator();
	fileTool->AddTool(MY1ID_OPTIONS, wxS("Options"), mIconOpts,
		wxS("Options"));
	fileTool->Realize();
	if(!mFileTool) mFileTool = fileTool;
	return fileTool;
}

wxAuiToolBar* my1Form::CreateProcToolBar(void)
{
	wxBitmap mIconAssemble = MACRO_WXBMP(binary);
	wxBitmap mIconSimulate = MACRO_WXBMP(simx);
	wxBitmap mIconGenerate = MACRO_WXBMP(hexgen);
	wxAuiToolBar* procTool = new wxAuiToolBar(this, MY1ID_PROCTOOL,
		wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	procTool->SetToolBitmapSize(wxSize(16,16));
	procTool->AddTool(MY1ID_ASSEMBLE, wxS("Assemble"),
		mIconAssemble, wxS("Assemble"));
	procTool->AddTool(MY1ID_GENERATE, wxS("Generate"),
		mIconGenerate, wxS("Generate"));
	procTool->AddTool(MY1ID_SIMULATE, wxS("Simulate"),
		mIconSimulate, wxS("Simulate"));
	procTool->Realize();
	return procTool;
}

wxAuiToolBar* my1Form::CreateDevcToolBar(void)
{
	wxBitmap mIconDEVLED = MACRO_WXBMP(devled);
	wxBitmap mIconDEVSWI = MACRO_WXBMP(devswi);
	wxBitmap mIconDEVBUT = MACRO_WXBMP(devbut);
	wxBitmap mIconDV7SEG = MACRO_WXBMP(dv7seg);
	wxBitmap mIconDVKPAD = MACRO_WXBMP(dvkpad);
	wxBitmap mIconMiniMV = MACRO_WXBMP(target);
	wxAuiToolBar* devcTool = new wxAuiToolBar(this, MY1ID_DEVCTOOL,
		wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	devcTool->SetToolBitmapSize(wxSize(16,16));
	devcTool->AddTool(MY1ID_CREATE_DEVLED, wxS("LED"),
		mIconDEVLED, wxS("LED"));
	devcTool->AddTool(MY1ID_CREATE_DEVSWI, wxS("Switch"),
		mIconDEVSWI, wxS("Switch"));
	devcTool->AddTool(MY1ID_CREATE_DEVBUT, wxS("Button"),
		mIconDEVBUT, wxS("Button"));
	devcTool->AddTool(MY1ID_CREATE_DV7SEG, wxS("7-segment"),
		mIconDV7SEG, wxS("7-segment"));
	devcTool->AddTool(MY1ID_CREATE_DVKPAD, wxS("Keypad"),
		mIconDVKPAD, wxS("Keypad"));
	devcTool->AddSeparator();
	devcTool->AddTool(MY1ID_CREATE_MINIMV, wxS("MiniMV"),
		mIconMiniMV, wxS("Create Mini MemViewer"));
	devcTool->Realize();
	return devcTool;
}

wxPanel* my1Form::CreateInitPanel(wxWindow *parent)
{
	wxPanel *cPanelX = new wxPanel(mNoteBook);
	wxFont cFont(PANEL_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanelX->SetFont(cFont);
	wxStaticText *tLabel = new wxStaticText(cPanelX,wxID_ANY,wxS(MY1APP_TITLE));
	wxFont tFont(TITLE_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	tLabel->SetFont(tFont);
	wxStaticText *pLabel = new wxStaticText(cPanelX,wxID_ANY,
		wxS("8085 Microprocessor System Development"));
	wxFont pFont(SIMS_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	pLabel->SetFont(pFont);
	wxStaticText *eLabel = new wxStaticText(cPanelX,wxID_ANY,
		wxS(MY1APP_AUTHOR));
	wxFont eFont(EMAIL_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	eLabel->SetFont(eFont);
	wxBoxSizer *aBoxSizer = new wxBoxSizer(wxVERTICAL);
	aBoxSizer->AddStretchSpacer();
	aBoxSizer->Add(tLabel,1,wxALIGN_CENTER);
	aBoxSizer->Add(pLabel,1,wxALIGN_CENTER);
	aBoxSizer->Add(eLabel,0,wxALIGN_BOTTOM|wxALIGN_RIGHT);
	cPanelX->SetSizerAndFit(aBoxSizer);
	return cPanelX;
}

wxPanel* my1Form::CreateMainPanel(void)
{
	wxPanel *cPanel = new wxPanel(this);
	wxFont cFont(PANEL_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	// start sidebox content - build panel!
	my1Panel *cBuildHead = new my1Panel(cPanel,wxID_ANY,-1,
		wxS("Build Menu"),-1,-1,wxTAB_TRAVERSAL|wxBORDER_RAISED);
	cBuildHead->SetTextColor(*wxBLUE);
	cBuildHead->SetBackgroundColour(wxColor(0xAA,0xAA,0xAA));
	wxButton *cButtonRST = new wxButton(cPanel, MY1ID_BUILDRST, wxS("Reset"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonDEF = new wxButton(cPanel, MY1ID_BUILDDEF, wxS("Default"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonNFO = new wxButton(cPanel, MY1ID_BUILDNFO, wxS("Current"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonROM = new wxButton(cPanel, MY1ID_BUILDROM, wxS("Add ROM"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonRAM = new wxButton(cPanel, MY1ID_BUILDRAM, wxS("Add RAM"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonPPI = new wxButton(cPanel, MY1ID_BUILDPPI, wxS("Add PPI"),
		wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *sBoxSizer = new wxBoxSizer(wxVERTICAL);
	sBoxSizer->Add(cBuildHead, 1, wxEXPAND);
	sBoxSizer->Add(cButtonRST, 1, wxEXPAND);
	sBoxSizer->Add(cButtonDEF, 1, wxEXPAND);
	sBoxSizer->Add(cButtonNFO, 1, wxEXPAND);
	sBoxSizer->Add(cButtonROM, 1, wxEXPAND);
	sBoxSizer->Add(cButtonRAM, 1, wxEXPAND);
	sBoxSizer->Add(cButtonPPI, 1, wxEXPAND);
	cPanel->SetSizerAndFit(sBoxSizer);
	return cPanel;
}

wxPanel* my1Form::CreateRegsPanel(void)
{
	wxPanel *cPanel = new wxPanel(this);
	wxFont cFont(PANEL_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	// vertical layout
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	// header panel - general purpose registers
	my1Panel *cHeader = new my1Panel(cPanel,wxID_ANY,-1,
		wxS("8-bit Registers"),REGS_PANEL_WIDTH,REGS_HEADER_HEIGHT,
		wxTAB_TRAVERSAL|wxBORDER_SUNKEN);
	pBoxSizer->Add(cHeader,0,wxEXPAND);
	// fill - general purpose registers
	for(int cLoop=0;cLoop<8;cLoop++) // 8-bit regs
	{
		wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
		my1Reg85 *pReg85 = m8085.Register(cLoop);
		int cRegID = cLoop;
		if(cLoop==7) cRegID = -1;
		else if(cLoop==6) cRegID = 4;
		else if(cLoop==5) cRegID = 10;
		else if(cLoop==4) cRegID = 6;
		wxString cRegName = wxString::Format(wxS("%c"),(char)cRegID+'B');
		my1Panel *cLabel = new my1Panel(cPanel,wxID_ANY,-1,cRegName,
			-1,-1,wxTAB_TRAVERSAL|wxBORDER_SUNKEN);
		wxString cRegValue = wxString::Format("%02X",pReg85->GetData());
		my1Panel *cValue = new my1Panel(cPanel,wxID_ANY,cLoop,cRegValue,
			-1,-1,wxTAB_TRAVERSAL|wxBORDER_SUNKEN);
		cValue->SetBackgroundColour(*wxWHITE);
		pReg85->SetLink((void*)cValue);
		pReg85->DoUpdate = &this->SimUpdateREG;
		// add to row sizer
		cBoxSizer->AddSpacer(INFO_REG_SPACER);
		cBoxSizer->Add(cLabel,1,wxEXPAND);
		cBoxSizer->AddSpacer(INFO_REG_SPACER);
		cBoxSizer->Add(cValue,1,wxEXPAND);
		cBoxSizer->AddSpacer(INFO_REG_SPACER);
		// add to main sizer
		pBoxSizer->AddSpacer(INFO_REG_SPACER);
		pBoxSizer->Add(cBoxSizer,0,wxEXPAND);
	}
	pBoxSizer->AddSpacer(INFO_REG_SPACER);
	// header panel - system registers
	cHeader = new my1Panel(cPanel,wxID_ANY,-1,
		wxS("System Registers"),REGS_PANEL_WIDTH,REGS_HEADER_HEIGHT,
		wxTAB_TRAVERSAL|wxBORDER_SUNKEN);
	pBoxSizer->Add(cHeader,0,wxEXPAND);
	// program counter
	{
		wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
		my1Reg85 *pReg85 = m8085.Register(I8085_RP_PC+I8085_REG_COUNT);
		wxString cRegName = wxS("PC");
		my1Panel *cLabel = new my1Panel(cPanel,wxID_ANY,-1,cRegName,
			-1,-1,wxTAB_TRAVERSAL|wxBORDER_SUNKEN);
		wxString cRegValue = wxString::Format("%04X",pReg85->GetData());
		my1Panel *cValue = new my1Panel(cPanel,wxID_ANY,-1,cRegValue,
			-1,-1,wxTAB_TRAVERSAL|wxBORDER_SUNKEN);
		cValue->SetBackgroundColour(*wxWHITE);
		pReg85->SetLink((void*)cValue);
		pReg85->DoUpdate = &this->SimUpdateREG;
		// add to row sizer
		cBoxSizer->AddSpacer(INFO_REG_SPACER);
		cBoxSizer->Add(cLabel,1,wxEXPAND);
		cBoxSizer->AddSpacer(INFO_REG_SPACER);
		cBoxSizer->Add(cValue,1,wxEXPAND);
		cBoxSizer->AddSpacer(INFO_REG_SPACER);
		// add to main sizer
		pBoxSizer->AddSpacer(INFO_REG_SPACER);
		pBoxSizer->Add(cBoxSizer,0,wxEXPAND);
	}
	// stack pointer
	{
		wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
		my1Reg85 *pReg85 = m8085.Register(I8085_RP_SP+I8085_REG_COUNT);
		wxString cRegName = wxS("SP");
		my1Panel *cLabel = new my1Panel(cPanel,wxID_ANY,-1,cRegName,
			-1,-1,wxTAB_TRAVERSAL|wxBORDER_SUNKEN);
		wxString cRegValue = wxString::Format("%04X",pReg85->GetData());
		my1Panel *cValue = new my1Panel(cPanel,wxID_ANY,-1,cRegValue,
			-1,-1,wxTAB_TRAVERSAL|wxBORDER_SUNKEN);
		cValue->SetBackgroundColour(*wxWHITE);
		pReg85->SetLink((void*)cValue);
		pReg85->DoUpdate = &this->SimUpdateREG;
		// add to row sizer
		cBoxSizer->AddSpacer(INFO_REG_SPACER);
		cBoxSizer->Add(cLabel,1,wxEXPAND);
		cBoxSizer->AddSpacer(INFO_REG_SPACER);
		cBoxSizer->Add(cValue,1,wxEXPAND);
		cBoxSizer->AddSpacer(INFO_REG_SPACER);
		// add to main sizer
		pBoxSizer->AddSpacer(INFO_REG_SPACER);
		pBoxSizer->Add(cBoxSizer,0,wxEXPAND);
	}
	pBoxSizer->AddSpacer(INFO_REG_SPACER);
	// header panel - flag bits
	cHeader = new my1Panel(cPanel,wxID_ANY,-1,
		wxS("Flag Bits"),REGS_PANEL_WIDTH,REGS_HEADER_HEIGHT,
		wxTAB_TRAVERSAL|wxBORDER_SUNKEN);
	pBoxSizer->Add(cHeader,0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_REG_SPACER);
	// flag labels
	{
		wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
		char cFlagHD[] = "SZXAXPXC";
		for(int cLoop=0;cLoop<8;cLoop++) // flag header
		{
			wxString cFlagName = cFlagHD[cLoop];
			my1Panel *cLabel = new my1Panel(cPanel,wxID_ANY,-1,cFlagName,
				-1,-1,wxTAB_TRAVERSAL|wxBORDER_SUNKEN);
			// add to row sizer
			cBoxSizer->Add(cLabel,1,wxEXPAND);
		}
		// add to main sizer
		pBoxSizer->Add(cBoxSizer,0,wxEXPAND);
	}
	// flag bits
	{
		wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
		int cFlagID = 0x80;
		for(int cLoop=0;cLoop<8;cLoop++,cFlagID>>=1) // flag value
		{
			bool cGoWhite = false;
			wxString cFlagValue = wxS("X");
			if(cFlagID&I8085_FLAG_BITS)
			{
				my1Reg85 *pReg85 = m8085.Register(I8085_REG_F);
				cFlagValue = wxString::Format(wxS("%01X"),
						pReg85->GetData()&cFlagID?1:0);
				cGoWhite = true;
			}
			my1Panel *cValue = new my1Panel(cPanel,wxID_ANY,-1,cFlagValue,
				-1,-1,wxTAB_TRAVERSAL|wxBORDER_SUNKEN);
			if(cGoWhite)
			{
				cValue->SetBackgroundColour(*wxWHITE);
				this->FlagLink(cFlagID).SetLink((void*)cValue);
			}
			// add to row sizer
			cBoxSizer->Add(cValue,1,wxEXPAND);
		}
		// add to main sizer
		pBoxSizer->Add(cBoxSizer,0,wxEXPAND);
	}
	pBoxSizer->AddSpacer(INFO_REG_SPACER);
	// header panel - flag bits
	cHeader = new my1Panel(cPanel,wxID_ANY,-1,
		wxS("Memory Space"),REGS_PANEL_WIDTH,REGS_HEADER_HEIGHT,
		wxTAB_TRAVERSAL|wxBORDER_SUNKEN);
	cHeader->SetBackgroundColour(*wxWHITE);
	pBoxSizer->Add(cHeader,0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_REG_SPACER);
	// put memory panel here?
	{
		wxPanel *pPanelM = CreateMemoryGridPanel(cPanel,0x0000,
			MEM_VIEW_WIDTH,MEM_VIEW_HEIGHT,&mMemoryGrid);
		// add to main sizer
		pBoxSizer->Add(pPanelM,1,wxEXPAND);
	}
	// assign to main panel
	cPanel->SetSizerAndFit(pBoxSizer);
	return cPanel;
}

wxPanel* my1Form::CreateIntrPanel(void)
{
	wxPanel *cPanel = new wxPanel(this);
	wxFont cFont(SIMS_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	cPanel->SetLabel(wxS("INTPANEL"));
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	int cInterruptID = I8085_PIN_TRAP;
	for(int cLoop=0;cLoop<I8085_PIN_COUNT;cLoop++)
	{
		wxString cLabel, cType;
		switch(cInterruptID)
		{
			case I8085_PIN_TRAP:
				cType = wxS("Trap");
				cLabel = wxString::Format(wxS("TRAP [0x%04X]"),I8085_ISR_TRP);
				break;
			case I8085_PIN_I7P5:
				cType = wxS("I7.5");
				cLabel = wxString::Format(wxS("I7.5 [0x%04X]"),I8085_ISR_7P5);
				break;
			case I8085_PIN_I6P5:
				cType = wxS("I6.5");
				cLabel = wxString::Format(wxS("I6.5 [0x%04X]"),I8085_ISR_6P5);
				break;
			case I8085_PIN_I5P5:
				cType = wxS("I5.5");
				cLabel = wxString::Format(wxS("I5.5 [0x%04X]"),I8085_ISR_5P5);
				break;
		}
		my1INTCtrl* pCtrl = new my1INTCtrl(cPanel,wxID_ANY,
			REGS_PANEL_WIDTH/I8085_PIN_COUNT,REGS_HEADER_HEIGHT*4/5,cType);
		pCtrl->SetLabel(cLabel);
		// get interrupt index & link, anID should be >=0 && <I8085_PIN_COUNT
		my1BitIO& rBitIO = m8085.Pin(cInterruptID);
		my1BitSelect cLink(cInterruptID,(void*) &rBitIO);
		pCtrl->LinkCheck(cLink);
		pBoxSizer->Add((wxWindow*)pCtrl,0,wxALIGN_CENTER);
		cInterruptID++;
	}
	cPanel->SetSizerAndFit(pBoxSizer);
	return cPanel;
}

wxPanel* my1Form::CreateSimsPanel(void)
{
	wxPanel *cPanel = new wxPanel(this);
	wxFont cFont(SIMS_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxButton *cButtonStep = new wxButton(cPanel, MY1ID_SIMSSTEP, wxS("Step"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonExec = new wxButton(cPanel, MY1ID_SIMSEXEC, wxS("Run"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonRset = new wxButton(cPanel, MY1ID_SIMRESET, wxS("Reset"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonBRKP = new wxButton(cPanel, MY1ID_SIMSBRKP, wxS("Break"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonInfo = new wxButton(cPanel, MY1ID_SIMSINFO, wxS("Info"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonPrev = new wxButton(cPanel, MY1ID_SIMSPREV, wxS("Prev"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonMini = new wxButton(cPanel, MY1ID_SIMSMIMV, wxS("miniMV"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonExit = new wxButton(cPanel, MY1ID_SIMSEXIT, wxS("Exit"),
		wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	pBoxSizer->Add(cButtonStep, 1, wxEXPAND);
	pBoxSizer->Add(cButtonExec, 1, wxEXPAND);
	pBoxSizer->Add(cButtonRset, 1, wxEXPAND);
	pBoxSizer->Add(cButtonBRKP, 1, wxEXPAND);
	pBoxSizer->Add(cButtonInfo, 1, wxEXPAND);
	pBoxSizer->Add(cButtonPrev, 1, wxEXPAND);
	pBoxSizer->Add(cButtonMini, 1, wxEXPAND);
	pBoxSizer->Add(cButtonExit, 1, wxEXPAND);
	cPanel->SetSizerAndFit(pBoxSizer);
	return cPanel;
}

wxPanel* my1Form::CreateConsPanel(void)
{
	wxPanel *cPanel = new wxPanel(this);
	wxTextCtrl *cConsole = new wxTextCtrl(cPanel, wxID_ANY,
		wxS(""), wxDefaultPosition, wxDefaultSize,
		wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH, wxDefaultValidator);
	wxPanel *cComsPanel = new wxPanel(cPanel);
	wxTextCtrl *cCommandText = new wxTextCtrl(cComsPanel, MY1ID_CONSCOMM,
		wxS(""), wxDefaultPosition, wxDefaultSize,wxTE_PROCESS_ENTER);
	cCommandText->Connect(MY1ID_CONSCOMM,wxEVT_KEY_DOWN,
		WX_KEH(my1Form::OnCheckConsole),NULL,this);
	wxButton *cButton = new wxButton(cComsPanel, MY1ID_CONSEXEC,
		wxS("Execute"));
	wxBoxSizer *dBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	dBoxSizer->Add(cCommandText, 1, wxEXPAND);
	dBoxSizer->Add(cButton, 0, wxALIGN_RIGHT);
	cComsPanel->SetSizer(dBoxSizer);
	dBoxSizer->Fit(cComsPanel);
	dBoxSizer->SetSizeHints(cComsPanel);
	wxBoxSizer *eBoxSizer = new wxBoxSizer(wxVERTICAL);
	eBoxSizer->Add(cConsole, 1, wxEXPAND);
	eBoxSizer->Add(cComsPanel, 0, wxALIGN_BOTTOM|wxEXPAND);
	cPanel->SetSizerAndFit(eBoxSizer);
	wxFont cFont(CONS_FONT_SIZE,wxFONTFAMILY_TELETYPE,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,
		false,wxEmptyString,wxFONTENCODING_ISO8859_1);
	cConsole->SetFont(cFont);
	cConsole->AppendText(wxString::Format(wxS("Welcome to %s\n\n"),
		MY1APP_TITLE));
	// 'remember' main console
	if(!mConsole) mConsole = cConsole;
	if(!mCommand) mCommand = cCommandText;
	return cPanel;
}

wxPanel* my1Form::CreateMemoryGridPanel(wxWindow* aParent, int aStart,
	int aWidth, int aHeight, wxGrid** ppGrid)
{
	wxPanel *cPanel = new wxPanel(aParent);
	wxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	wxGrid *pGrid = new wxGrid(cPanel, wxID_ANY);
	pGrid->CreateGrid(aHeight,aWidth);
	wxFont cFont(GRID_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	pGrid->SetFont(cFont);
	pGrid->SetLabelFont(cFont);
	//pGrid->UseNativeColHeader();
	pGrid->SetRowLabelAlignment(wxALIGN_CENTRE,wxALIGN_CENTRE);
	pGrid->SetColLabelAlignment(wxALIGN_CENTRE,wxALIGN_CENTRE);
	pGrid->SetDefaultCellAlignment(wxALIGN_CENTRE,wxALIGN_CENTRE);
	for(int cRow=0;cRow<aHeight;cRow++)
		pGrid->SetRowLabelValue(cRow,
			wxString::Format(wxS("%04X"),aStart+cRow*aWidth));
	for(int cCol=0;cCol<aWidth;cCol++)
		pGrid->SetColLabelValue(cCol,wxString::Format(wxS("%02X"),cCol));
	for(int cRow=0;cRow<aHeight;cRow++)
		for(int cCol=0;cCol<aWidth;cCol++)
			pGrid->SetCellValue(cRow,cCol,wxString::Format(wxS("%02X"),0x0));
	pGrid->DisableCellEditControl();
	pGrid->EnableEditing(false);
	pGrid->SetRowLabelSize(wxGRID_AUTOSIZE);
	pGrid->AutoSize();
	for(int cRow=0;cRow<aHeight;cRow++)
		pGrid->DisableRowResize(cRow);
	for(int cCol=0;cCol<aWidth;cCol++)
		pGrid->DisableColResize(cCol);
	pBoxSizer->Add(pGrid,1,wxEXPAND);
	cPanel->SetSizerAndFit(pBoxSizer);
	*ppGrid = pGrid;
	return cPanel;
}

wxPanel* my1Form::CreateMemoryMiniPanel(int cAddress)
{
	if(cAddress<0)
	{
		cAddress = this->GetBuildAddress(wxS("Start Address for miniMV"));
		if(cAddress<0) return 0x0;
	}
	if(cAddress%8!=0)
	{
		cAddress = cAddress-cAddress%8;
		wxString cStatus = wxS("[miniMV] Address must be in multiples of 8!") +
			wxString::Format(wxS(" Using [0x%04X]"),cAddress);
		this->PrintMessage(cStatus.ToAscii());
	}
	my1Memory* pMemory = (my1Memory*) m8085.MemoryMap().Object((aword)cAddress);
	if(!pMemory)
	{
		wxString cStatus = wxS("[miniMV] Creation Error!");
		cStatus += wxS(" No memory object at address ") +
			wxString::Format(wxS("0x%04X!"),cAddress);
		this->PrintMessage(cStatus.ToAscii());
		return 0x0;
	}
	wxString cPanelName = wxS("miniMV") +
		wxString::Format(wxS("%04X"),cAddress);
	wxAuiPaneInfo& cPane = mMainUI.GetPane(cPanelName);
	if(cPane.IsOk())
	{
		cPane.Show();
		mMainUI.Update();
		return 0x0;
	}
	my1MiniViewer *pViewer = new my1MiniViewer;
	wxGrid* pGrid = 0x0;
	wxPanel* cPanel = CreateMemoryGridPanel(this,
		cAddress,MEM_MINIVIEW_WIDTH,MEM_MINIVIEW_HEIGHT,&pGrid);
	// update grid?
	aword cStart = cAddress;
	abyte cData;
	for(int cRow=0;cRow<MEM_MINIVIEW_HEIGHT;cRow++)
	{
		for(int cCol=0;cCol<MEM_MINIVIEW_WIDTH;cCol++)
		{
			if(pMemory->GetData(cStart,cData))
				pGrid->SetCellValue(cRow,cCol,
					wxString::Format(wxS("%02X"),(int)cData));
			cStart++;
		}
	}
	wxPoint cPoint = this->GetScreenPosition();
	mMainUI.AddPane(cPanel, wxAuiPaneInfo().Name(cPanelName).
		Caption(cPanelName).DefaultPane().Float().DestroyOnClose().
		TopDockable(false).BottomDockable(false).
		LeftDockable(true).RightDockable(false).
		FloatingPosition(cPoint.x+FLOAT_INIT_X,cPoint.y+FLOAT_INIT_Y));
	mMainUI.Update();
	pViewer->mStart = cAddress;
	pViewer->mSize = MEM_MINIVIEW_HEIGHT*MEM_VIEW_WIDTH;
	pViewer->pMemory = pMemory;
	pViewer->pGrid = pGrid;
	// get insert location based on start address
	my1MiniViewer *pTemp = mFirstViewer, *pPrev = 0x0;
	while(pTemp)
	{
		if(cAddress<pTemp->mStart)
			break;
		pPrev = pTemp;
		pTemp = pTemp->mNext;
	}
	// now, insert!
	if(!pPrev)
		mFirstViewer = pViewer;
	else
		pPrev->mNext = pViewer;
	pViewer->mNext = pTemp;
	return cPanel;
}

my1DEVPanel* my1Form::CreateDevice7SegPanel(const wxString& aName)
{
	// create unique panel name
	wxString cPanelName=wxS("dev7SEG");
	wxString cPanelCaption=wxS("7segment");
	if(aName!=wxEmptyString) cPanelName = aName;
	else if(!this->GetUniqueName(cPanelName)) return 0x0;
	// create 7-segment panel
	my1DEVPanel *cPanel = new my1DEVPanel(this);
	wxFont cFont(SIMS_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	my1LED7Seg *cTemp;
	wxGBPosition cPosGB;
	wxString cLabel;
	wxGridBagSizer *pGridBagSizer = new wxGridBagSizer(); // vgap,hgap
	// this is 'msb' - for panel port linking (inserted below!)
	my1LEDCtrl* cTest = new my1LEDCtrl(cPanel, wxID_ANY,
		true, DOT_SIZE, DOT_SIZE); // dot
	cLabel = wxS("dot"); cTest->SetLabel(cLabel);
	cPosGB.SetRow(4); cPosGB.SetCol(3);
	pGridBagSizer->Add((wxWindow*)cTest,cPosGB);
	// bit-6 => 'g'
	cTemp = new my1LED7Seg(cPanel, wxID_ANY, false); // mid horiz
	cLabel = wxS("g"); cTemp->SetLabel(cLabel);
	cPosGB.SetRow(2); cPosGB.SetCol(1);
	pGridBagSizer->Add((wxWindow*)cTemp,cPosGB);
	// bit-5 => 'f'
	cTemp = new my1LED7Seg(cPanel, wxID_ANY, true); // top-left vert
	cLabel = wxS("f"); cTemp->SetLabel(cLabel);
	cPosGB.SetRow(1); cPosGB.SetCol(0);
	pGridBagSizer->Add((wxWindow*)cTemp,cPosGB);
	// bit-4 => 'e'
	cTemp = new my1LED7Seg(cPanel, wxID_ANY, true); // bot-left vert
	cLabel = wxS("e"); cTemp->SetLabel(cLabel);
	cPosGB.SetRow(3); cPosGB.SetCol(0);
	pGridBagSizer->Add((wxWindow*)cTemp,cPosGB);
	// bit-3 => 'd'
	cTemp = new my1LED7Seg(cPanel, wxID_ANY, false); // bot horiz
	cLabel = wxS("d"); cTemp->SetLabel(cLabel);
	cPosGB.SetRow(4); cPosGB.SetCol(1);
	pGridBagSizer->Add((wxWindow*)cTemp,cPosGB);
	// bit-2 => 'c'
	cTemp = new my1LED7Seg(cPanel, wxID_ANY, true); // bot-right vert
	cLabel = wxS("c"); cTemp->SetLabel(cLabel);
	cPosGB.SetRow(3); cPosGB.SetCol(2);
	pGridBagSizer->Add((wxWindow*)cTemp,cPosGB);
	// bit-1 => 'b'
	cTemp = new my1LED7Seg(cPanel, wxID_ANY, true); // top-right vert
	cLabel = wxS("b"); cTemp->SetLabel(cLabel);
	cPosGB.SetRow(1); cPosGB.SetCol(2);
	pGridBagSizer->Add((wxWindow*)cTemp,cPosGB);
	// bit-0 => 'a'
	cTemp = new my1LED7Seg(cPanel, wxID_ANY, false); // top horiz
	cLabel = wxS("a"); cTemp->SetLabel(cLabel);
	cPosGB.SetRow(0); cPosGB.SetCol(1);
	pGridBagSizer->Add((wxWindow*)cTemp,cPosGB);
	// add this!
	pBoxSizer->AddSpacer(SEG7_NUM_SPACER);
	pBoxSizer->Add(pGridBagSizer, 0, wxALIGN_CENTER);
	cPanel->SetSizerAndFit(pBoxSizer);
	// pass to aui manager
	mMainUI.AddPane(cPanel,wxAuiPaneInfo().Name(cPanelName).
		Caption(cPanelCaption).DefaultPane().Fixed().Position(DEV_INIT_POS).
		Bottom().Dockable(true).DestroyOnClose());
	mMainUI.Update();
	// save in main list
	mDevPanels.Append(cPanel);
	this->PrintInfoMessage(wxString::Format("Panel '%s' created!",
		cPanelName.Mid(0,cPanelName.Length()-2)));
	// port selector menu
	cPanel->Connect(cPanel->GetId(),wxEVT_RIGHT_DOWN,
		WX_MEH(my1Form::OnBITPanelClick),NULL,this);
	cPanel->Connect(cPanel->GetId(),wxEVT_LEFT_DCLICK,
		WX_MEH(my1Form::OnBITPanelClick),NULL,this);
	// return pointer to panel
	return cPanel;
}

my1DEVPanel* my1Form::CreateDeviceKPadPanel(const wxString& aName)
{
	// create unique panel name
	wxString cPanelName=wxS("devKPAD");
	wxString cPanelCaption=wxS("Keypad");
	if(aName!=wxEmptyString) cPanelName = aName;
	else if(!this->GetUniqueName(cPanelName)) return 0x0;
	// create keypad panel
	my1DEVPanel *cPanel = new my1DEVPanel(this);
	wxFont cFont(KPAD_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	wxString cLabel;
	wxGBPosition cPosGB;
	// need to create the bitctrls first! wxCLASSINFO macro is not so smart!
	wxGridBagSizer *pGridBagSizer = new wxGridBagSizer(); // vgap,hgap
	// 3 dummy controls (for port assignment)
	my1ENCkPad *pData = new my1ENCkPad(cPanel, wxID_ANY,true);
	pData = new my1ENCkPad(cPanel, wxID_ANY,true);
	pData = new my1ENCkPad(cPanel, wxID_ANY,true);
	// data accessible signal
	pData = new my1ENCkPad(cPanel, wxID_ANY);
	cLabel = wxS("DA"); pData->SetLabel(cLabel);
	cPosGB.SetRow(0); cPosGB.SetCol(0);
	pGridBagSizer->Add(pData,cPosGB);
	// encoder output d3
	pData = new my1ENCkPad(cPanel, wxID_ANY);
	cLabel = wxS("D3"); pData->SetLabel(cLabel);
	cPosGB.SetRow(1); cPosGB.SetCol(0);
	pGridBagSizer->Add(pData,cPosGB);
	// encoder output d2
	pData = new my1ENCkPad(cPanel, wxID_ANY);
	cLabel = wxS("D2"); pData->SetLabel(cLabel);
	cPosGB.SetRow(2); cPosGB.SetCol(0);
	pGridBagSizer->Add(pData,cPosGB);
	// encoder output d1
	pData = new my1ENCkPad(cPanel, wxID_ANY);
	cLabel = wxS("D1"); pData->SetLabel(cLabel);
	cPosGB.SetRow(3); cPosGB.SetCol(0);
	pGridBagSizer->Add(pData,cPosGB);
	// encoder output d0
	pData = new my1ENCkPad(cPanel, wxID_ANY);
	cLabel = wxS("D0"); pData->SetLabel(cLabel);
	cPosGB.SetRow(4); cPosGB.SetCol(0);
	pGridBagSizer->Add(pData,cPosGB);
	// create new grid
	wxGridBagSizer *qGridBagSizer = new wxGridBagSizer(); // vgap,hgap
	int cSize = (KEY_SIZE_PANEL*5)/4;
	for(int cRow=0,cIndex=0;cRow<4;cRow++)
	{
		for(int cCol=0;cCol<4;cCol++)
		{
			if(cRow==3&&cCol==0)
			{	cLabel = wxS("*"); cIndex = -1; }
			else if(cRow==3&&cCol==2)
			{	cLabel = wxS("#"); cIndex = 15; }
			else if(cCol==3)
				cLabel = wxString::Format(wxS("%c"),(char)(cIndex/4)+'A');
			else
				cLabel = wxString::Format(wxS("%d"),++cIndex);
			my1KEYCtrl *pCtrl = new my1KEYCtrl(cPanel,wxID_ANY,cSize,cSize,
				(cRow*4+cCol),cLabel);
			cPosGB.SetRow(cRow); cPosGB.SetCol(cCol);
			qGridBagSizer->Add(pCtrl,cPosGB);
		}
	}
	// add to main sizer
	pBoxSizer->Add(qGridBagSizer,0,wxALIGN_CENTER);
	pBoxSizer->AddSpacer(5);
	pBoxSizer->Add(pGridBagSizer,0,wxALIGN_CENTER);
	pBoxSizer->AddSpacer(5);
	// assign sizer to main panel
	cPanel->SetSizerAndFit(pBoxSizer);
	// pass to aui manager
	mMainUI.AddPane(cPanel,wxAuiPaneInfo().Name(cPanelName).
		Caption(cPanelCaption).DefaultPane().Fixed().Position(DEV_INIT_POS).
		Bottom().Dockable(true).DestroyOnClose());
	mMainUI.Update();
	// save in main list
	mDevPanels.Append(cPanel);
	this->PrintInfoMessage(wxString::Format("Panel '%s' created!",
		cPanelName.Mid(0,cPanelName.Length()-2)));
	// panel doesn't look nice at first, refreshing view
	cPanel->SendSizeEvent();
	// port selector menu
	cPanel->Connect(cPanel->GetId(),wxEVT_RIGHT_DOWN,
		WX_MEH(my1Form::OnBITPanelClick),NULL,this);
	cPanel->Connect(cPanel->GetId(),wxEVT_LEFT_DCLICK,
		WX_MEH(my1Form::OnBITPanelClick),NULL,this);
	// return pointer to panel
	return cPanel;
}

my1DEVPanel* my1Form::CreateDeviceLEDPanel(const wxString& aName,
	bool aVertical)
{
	// create unique panel name
	wxString cPanelName=wxS("devLED");
	wxString cPanelCaption=wxS("LED");
	if(aName!=wxEmptyString) cPanelName = aName;
	else if(!this->GetUniqueName(cPanelName)) return 0x0;
	// create the panel
	my1DEVPanel *cPanel = new my1DEVPanel(this);
	wxFont cFont(SIMS_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	cPanel->Flag(aVertical);
	int cOrient = aVertical ? wxVERTICAL : wxHORIZONTAL;
	wxBoxSizer *pBoxSizer = new wxBoxSizer(cOrient);
	pBoxSizer->AddSpacer(DEVC_POP_SPACER);
	for(int cLoop=0;cLoop<DATASIZE;cLoop++)
	{
		my1LEDCtrl* pCtrl = new my1LEDCtrl(cPanel,wxID_ANY);
		pBoxSizer->Add((wxWindow*)pCtrl,0,wxALIGN_TOP);
	}
	pBoxSizer->AddSpacer(DEVC_POP_SPACER);
	cPanel->SetSizerAndFit(pBoxSizer);
	// pass to aui manager
	mMainUI.AddPane(cPanel,wxAuiPaneInfo().Name(cPanelName).
		Caption(cPanelCaption).DefaultPane().Fixed().Position(DEV_INIT_POS).
		Top().Dockable(true).DestroyOnClose());
	if(aVertical)
	{
		wxAuiPaneInfo &cPane = mMainUI.GetPane(cPanelName);
		if(cPane.IsOk()) cPane.Left();
	}
	mMainUI.Update();
	// save in main list
	mDevPanels.Append(cPanel);
	this->PrintInfoMessage(wxString::Format("Panel '%s' created!",
		cPanelName.Mid(0,cPanelName.Length()-2)));
	// port selector menu
	cPanel->Connect(cPanel->GetId(),wxEVT_RIGHT_DOWN,
		WX_MEH(my1Form::OnBITPanelClick),NULL,this);
	cPanel->Connect(cPanel->GetId(),wxEVT_LEFT_DCLICK,
		WX_MEH(my1Form::OnBITPanelClick),NULL,this);
	// return pointer to panel
	return cPanel;
}

my1DEVPanel* my1Form::CreateDeviceSWIPanel(const wxString& aName)
{
	// create unique panel name
	wxString cPanelName=wxS("devSWI");
	wxString cPanelCaption=wxS("Switch");
	if(aName!=wxEmptyString) cPanelName = aName;
	else if(!this->GetUniqueName(cPanelName)) return 0x0;
	// create the panel
	my1DEVPanel *cPanel = new my1DEVPanel(this);
	wxFont cFont(SIMS_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	pBoxSizer->AddSpacer(DEVC_POP_SPACER);
	for(int cLoop=0;cLoop<DATASIZE;cLoop++)
	{
		my1SWICtrl* pCtrl = new my1SWICtrl(cPanel,wxID_ANY);
		pBoxSizer->Add((wxWindow*)pCtrl,0,wxALIGN_TOP);
	}
	pBoxSizer->AddSpacer(DEVC_POP_SPACER);
	cPanel->SetSizerAndFit(pBoxSizer);
	// pass to aui manager
	mMainUI.AddPane(cPanel,wxAuiPaneInfo().Name(cPanelName).
		Caption(cPanelCaption).DefaultPane().Fixed().Position(DEV_INIT_POS).
		Top().Dockable(true).DestroyOnClose());
	mMainUI.Update();
	// save in main list
	mDevPanels.Append(cPanel);
	this->PrintInfoMessage(wxString::Format("Panel '%s' created!",
		cPanelName.Mid(0,cPanelName.Length()-2)));
	// port selector menu
	cPanel->Connect(cPanel->GetId(),wxEVT_RIGHT_DOWN,
		WX_MEH(my1Form::OnBITPanelClick),NULL,this);
	cPanel->Connect(cPanel->GetId(),wxEVT_LEFT_DCLICK,
		WX_MEH(my1Form::OnBITPanelClick),NULL,this);
	// return pointer to panel
	return cPanel;
}

my1DEVPanel* my1Form::CreateDeviceBUTPanel(const wxString& aName)
{
	// create unique panel name
	wxString cPanelName=wxS("devBUT");
	wxString cPanelCaption=wxS("Button");
	if(aName!=wxEmptyString) cPanelName = aName;
	else if(!this->GetUniqueName(cPanelName)) return 0x0;
	// create the panel
	my1DEVPanel *cPanel = new my1DEVPanel(this);
	wxFont cFont(SIMS_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	pBoxSizer->AddSpacer(DEVC_POP_SPACER);
	for(int cLoop=0;cLoop<DATASIZE;cLoop++)
	{
		my1BUTCtrl* pCtrl = new my1BUTCtrl(cPanel,wxID_ANY);
		pBoxSizer->Add((wxWindow*)pCtrl,0,wxALIGN_TOP);
	}
	pBoxSizer->AddSpacer(DEVC_POP_SPACER);
	cPanel->SetSizerAndFit(pBoxSizer);
	// pass to aui manager
	mMainUI.AddPane(cPanel,wxAuiPaneInfo().Name(cPanelName).
		Caption(cPanelCaption).DefaultPane().Fixed().Position(DEV_INIT_POS).
		Top().Dockable(true).DestroyOnClose());
	mMainUI.Update();
	// save in main list
	mDevPanels.Append(cPanel);
	this->PrintInfoMessage(wxString::Format("Panel '%s' created!",
		cPanelName.Mid(0,cPanelName.Length()-2)));
	// port selector menu
	cPanel->Connect(cPanel->GetId(),wxEVT_RIGHT_DOWN,
		WX_MEH(my1Form::OnBITPanelClick),NULL,this);
	cPanel->Connect(cPanel->GetId(),wxEVT_LEFT_DCLICK,
		WX_MEH(my1Form::OnBITPanelClick),NULL,this);
	// return pointer to panel
	return cPanel;
}

void my1Form::OpenEdit(wxString& cFileName)
{
	my1CodeEdit *cCodeEdit = new my1CodeEdit(mNoteBook,
		wxID_ANY, cFileName, this->mOptions);
	wxString cTempFile = cCodeEdit->GetFileName();
	if(!cTempFile.Length())
		cTempFile = wxS("unnamed");
	cCodeEdit->Connect(cCodeEdit->GetId(),wxEVT_KEY_DOWN,
		WX_KEH(my1Form::OnCheckFont),NULL,this);
	mNoteBook->AddPage(cCodeEdit, cTempFile,true);
	if(mOptions.mConv_UnixEOL)
		cCodeEdit->ConvertEOLs(2);
	wxString cStatus = wxS("File ") +
		cCodeEdit->GetFileName() + wxS(" loaded!");
	this->ShowStatus(cStatus);
}

void my1Form::SaveEdit(wxWindow* cEditPane, bool aSaveAs)
{
	wxString cFileName;
	my1CodeEdit *cEditor = (my1CodeEdit*) cEditPane;
	if(aSaveAs||!cEditor->GetFileName().Length())
	{
		wxFileName cThisPath(mThisPath,"");
		cThisPath.AppendDir(wxS("asm"));
		wxFileDialog *cSelect = new wxFileDialog(this,wxS("Assign File Name"),
			wxS(""),wxS(""),wxS("Any file (*.*)|*.*"),
			wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
		cSelect->SetWildcard("ASM files (*.asm)|*.asm|Any file (*.*)|*.*");
		cSelect->SetDirectory(cThisPath.GetPath());
		if(cSelect->ShowModal()!=wxID_OK) return;
		cFileName = cSelect->GetPath();
		if(cSelect->GetFilterIndex()==0)
		{
			if(cFileName.Right(4)!=wxS(".asm"))
				cFileName += wxS(".asm");
		}
	}
	cEditor->SaveEdit(cFileName);
	wxString cStatus = wxS("File ") + cEditor->GetFileName() + wxS(" saved!");
	this->ShowStatus(cStatus);
}

void my1Form::ShowStatus(wxString& aString)
{
	this->SetStatusText(aString,STATUS_MSG_INDEX);
	mDisplayTimer->Start(STATUS_MSG_PERIOD,wxTIMER_ONE_SHOT);
}

void my1Form::OnFormClose(wxCloseEvent& event)
{
	// browse open notebook page
	int cCount = mNoteBook->GetPageCount();
	for(int cLoop=0;cLoop<cCount;cLoop++)
	{
		wxWindow *cTarget = mNoteBook->GetPage(cLoop);
		if(cTarget->IsKindOf(wxCLASSINFO(my1CodeEdit)))
		{
			my1CodeEdit *cEditor = (my1CodeEdit*) cTarget;
			if(cEditor->GetModify())
			{
				wxString cTitle = wxS("Changes in '") + cEditor->GetFileName();
				cTitle += wxS("' NOT Saved!");
				wxString cMessage = wxS("Save Before Closing?");
				cMessage += wxS(" [Cancel] will ignore all remaining!");
				int cGoSave = wxMessageBox(cMessage,cTitle,
					wxYES_NO|wxCANCEL|wxCANCEL_DEFAULT|wxICON_QUESTION,this);
				if(cGoSave==wxYES) this->SaveEdit(cTarget);
				else if(cGoSave==wxCANCEL) break;
			}
		}
	}
	if(event.CanVeto()) event.Skip();
	else this->Destroy();
}

void my1Form::OnQuit(wxCommandEvent& event)
{
	Close(true);
}

void my1Form::OnNew(wxCommandEvent& event)
{
	wxString cFileName = wxS("");
	this->OpenEdit(cFileName);
}

void my1Form::OnLoad(wxCommandEvent& event)
{
	wxFileName cThisPath(mThisPath,"");
	cThisPath.AppendDir(wxS("asm"));
	wxFileDialog *cSelect = new wxFileDialog(this,wxS("Select code file"),
		wxS(""),wxS(""),wxS("Any file (*.*)|*.*"),
		wxFD_OPEN|wxFD_FILE_MUST_EXIST);
	cSelect->SetWildcard("ASM files (*.asm)|*.asm|Any file (*.*)|*.*");
	cSelect->SetDirectory(cThisPath.GetPath());
	if(cSelect->ShowModal()!=wxID_OK) return;
	wxString cFileName = cSelect->GetPath();
	this->OpenEdit(cFileName);
}

void my1Form::OnSave(wxCommandEvent &event)
{
	int cSelect = mNoteBook->GetSelection();
	if(cSelect<0) return;
	wxWindow *cTarget = mNoteBook->GetPage(cSelect);
	if(!cTarget->IsKindOf(wxCLASSINFO(my1CodeEdit))) return;
	bool cSaveAs = false;
	if(event.GetId()==MY1ID_SAVEAS) cSaveAs = true;
	this->SaveEdit(cTarget,cSaveAs);
}

void my1Form::OnAbout(wxCommandEvent& event)
{
	wxAboutDialogInfo cAboutInfo;
	cAboutInfo.SetName(MY1APP_PROGNAME);
	cAboutInfo.SetVersion(MY1APP_PROGVERS);
	cAboutInfo.SetDescription(wxS(ABOUT_TITLE));
	cAboutInfo.SetCopyright(ABOUT_COPYRIGHT);
	cAboutInfo.SetWebSite(ABOUT_WEBSITE);
	cAboutInfo.AddDeveloper(ABOUT_AUTHOR);
	wxAboutBox(cAboutInfo,this);
}

void my1Form::OnWhatsNew(wxCommandEvent& event)
{
	wxFileName cFileName(mThisPath,wxS("CHANGELOG"));
	if(!cFileName.IsOk()||!cFileName.FileExists())
	{
		wxMessageBox(wxS("Cannot find file 'CHANGELOG'!"),wxS("[INFO]"),
			wxOK|wxICON_INFORMATION);
		return;
	}
	wxTextCtrl *cChangeLog = new wxTextCtrl(mNoteBook, wxID_ANY,
		wxS(MY1APP_TITLE" CHANGELOG\n\n"), wxDefaultPosition, wxDefaultSize,
		wxTE_AUTO_SCROLL|wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator);
	wxFont cFont(CONS_FONT_SIZE,wxFONTFAMILY_TELETYPE,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,
		false,wxEmptyString,wxFONTENCODING_ISO8859_1);
	cChangeLog->SetFont(cFont);
	cChangeLog->LoadFile(cFileName.GetFullPath()); // already checked?
	mNoteBook->AddPage(cChangeLog, wxS("CHANGELOG"),true);
}

void my1Form::OnReadMe(wxCommandEvent& event)
{
	wxFileName cFileName(mThisPath,wxS("README"));
	if(!cFileName.IsOk()||!cFileName.FileExists())
	{
		wxMessageBox(wxS("Cannot find file 'README'!"),wxS("[INFO]"),
			wxOK|wxICON_INFORMATION);
		return;
	}
	wxTextCtrl *cReadMe = new wxTextCtrl(mNoteBook, wxID_ANY,
		wxS(MY1APP_TITLE" README\n\n"), wxDefaultPosition, wxDefaultSize,
		wxTE_AUTO_SCROLL|wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator);
	wxFont cFont(CONS_FONT_SIZE,wxFONTFAMILY_TELETYPE,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,
		false,wxEmptyString,wxFONTENCODING_ISO8859_1);
	cReadMe->SetFont(cFont);
	cReadMe->LoadFile(cFileName.GetFullPath()); // already checked?
	mNoteBook->AddPage(cReadMe, wxS("README"),true);
}

void my1Form::OnMenuHighlight(wxMenuEvent& event)
{
	event.Skip(false);
}

void my1Form::OnAssemble(wxCommandEvent &event)
{
	my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
	wxString cFileLST;
	char *cDoList = 0x0;
	if(!cEditor)
	{
		int cSelect = mNoteBook->GetSelection();
		wxWindow *cTarget = mNoteBook->GetPage(cSelect);
		if(!cTarget->IsKindOf(wxCLASSINFO(my1CodeEdit))) return;
		cEditor = (my1CodeEdit*) cTarget;
	}
	if(cEditor->GetModify())
	{
		int cGoSave = wxMessageBox(wxS("Save & Continue?"),
			wxS("File modified!"),wxOK|wxCANCEL,this);
		if(cGoSave==wxCANCEL) return;
		this->SaveEdit((wxWindow*)cEditor);
	}
	wxString cStatus = wxS("Processing ") + cEditor->GetFileName() + wxS("...");
	this->ShowStatus(cStatus);
	if(this->mOptions.mComp_DoList) {
		cFileLST = cEditor->GetPathName() +
			cEditor->GetFileNoXT() + wxS(".lst");
		wxCStrData tbuf = cFileLST.c_str();
		cDoList = (char*) tbuf.AsChar();
	}
	if(m8085.Assemble(cEditor->GetFullName().ToAscii(),cDoList))
	{
		if(cDoList)
		{
			cStatus = wxS("[SUCCESS] LST file ") + cFileLST + wxS(" written!");
		}
		else
		{
			cStatus = wxS("[SUCCESS] Code in ") +
				cEditor->GetFileName() + wxS(" processed!");
		}
		this->ShowStatus(cStatus);
		m8085.SetCodeLink((void*)cEditor);
		cEditor->Assembled();
	}
	else
	{
		cStatus = wxS("[ERROR] Check start address?");
		this->ShowStatus(cStatus);
	}
}

void my1Form::OnSimulate(wxCommandEvent &event)
{
	my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
	if(!cEditor||cEditor->GetModify()||!cEditor->IsAssembled())
		this->OnAssemble(event);
	cEditor = (my1CodeEdit*) m8085.GetCodeLink();
	if(!cEditor) return;
	wxString cStatus = wxS("Preparing ") + cEditor->GetFileName() + wxS("...");
	this->ShowStatus(cStatus);
	m8085.SetStartAddress(mOptions.mSims_StartADDR);
	if(m8085.Simulate(0)) // force a reset!
	{
		if(m8085.NoCodex())
		{
			cStatus = wxS("[INFO] No code @ address 0x") +
				wxString::Format(wxS("%04X"),mOptions.mSims_StartADDR);
			this->PrintMessage(cStatus.ToAscii());
			return;
		}
		cStatus = wxS("[SUCCESS] Ready for Simulation!");
		this->ShowStatus(cStatus);
		this->SimulationMode();
		cEditor->SetReadOnly(mSimulationMode);
		cEditor->ExecLine(m8085.GetCodexLine()-1);
	}
	else
	{
		cStatus = wxS("[ERROR] No memory @ start address?");
		this->ShowStatus(cStatus);
	}
}

void my1Form::OnGenerate(wxCommandEvent &event)
{
	my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
	if(!cEditor||cEditor->GetModify()||!cEditor->IsAssembled())
		this->OnAssemble(event);
	cEditor = (my1CodeEdit*) m8085.GetCodeLink();
	if(!cEditor) return;
	wxString cFileHEX = cEditor->GetPathName() +
		cEditor->GetFileNoXT() + wxS(".hex");
	wxString cStatus = wxS("Processing ") +
		cEditor->GetFileName() + wxS("...");
	this->ShowStatus(cStatus);
	if(m8085.Generate(cFileHEX.ToAscii()))
	{
		cStatus = wxS("[SUCCESS] HEX file ") + cFileHEX + wxS(" written!");
		this->ShowStatus(cStatus);
	}
	else
	{
		cStatus = wxS("[ERROR] Cannot generate HEX file!");
		this->ShowStatus(cStatus);
	}
}

void my1Form::OnSysLoad(wxCommandEvent &event)
{
	wxFileName cThisPath(mThisPath,"");
	cThisPath.AppendDir(wxS("sys"));
	wxFileDialog *cSelect = new wxFileDialog(this,wxS("Select config file"),
		wxS(""),wxS(""),wxS("Any file (*.*)|*.*"),
		wxFD_OPEN|wxFD_FILE_MUST_EXIST);
	cSelect->SetWildcard("8085-System files (*.8085)|*.8085|"
		"Any file (*.*)|*.*");
	cSelect->SetDirectory(cThisPath.GetPath());
	if(cSelect->ShowModal()!=wxID_OK) return;
	wxString cFilename = cSelect->GetPath();
	if(!this->LoadSystem(cFilename))
	{
		wxString cMessage = wxString::Format(
			wxS("Cannot load system from '%s'!"),cFilename.ToAscii());
		wxMessageBox(cMessage,wxS("[System Load Error]"),
			wxOK|wxICON_ERROR);
	}
}

void my1Form::OnSysSave(wxCommandEvent &event)
{
	wxFileName cThisPath(mThisPath,"");
	cThisPath.AppendDir(wxS("sys"));
	wxFileDialog *cSelect = new wxFileDialog(this,wxS("Assign File Name"),
		wxS(""),wxS(""),wxS("Any file (*.*)|*.*"),
		wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	cSelect->SetWildcard("8085-System files (*.8085)|*.8085|"
		"Any file (*.*)|*.*");
	cSelect->SetDirectory(cThisPath.GetPath());
	if(cSelect->ShowModal()!=wxID_OK) return;
	wxString cFilename = cSelect->GetPath();
	if(cSelect->GetFilterIndex()==0)
	{
		if(cFilename.Right(5)!=wxS(".8085"))
			cFilename += wxS(".8085");
	}
	if(!this->SaveSystem(cFilename))
	{
		wxString cMessage = wxString::Format(
			wxS("Cannot save system to '%s'!"),cFilename.ToAscii());
		wxMessageBox(cMessage,wxS("[System Save Error]"),
			wxOK|wxICON_ERROR);
	}
}

void my1Form::PrintMessage(const wxString& aMessage, bool aNewline)
{
	mConsole->AppendText(aMessage);
	if(aNewline) mConsole->AppendText("\n");
}

void my1Form::PrintTaggedMessage(const wxString& aTag, const wxString& aMessage,
	const wxColor& aTagColor)
{
	long cPosB = mConsole->GetInsertionPoint();
	wxTextAttr cTextAttr;
	mConsole->GetStyle(cPosB,cTextAttr);
	wxString cTag = wxS("[") + aTag + wxS("] ");
	this->PrintMessage(cTag);
	if(aTagColor!=wxNullColour)
	{
		long cPosE = mConsole->GetInsertionPoint();
		wxColor cSaveColor = cTextAttr.GetTextColour();
		cTextAttr.SetTextColour(aTagColor);
		mConsole->SetStyle(cPosB,cPosE,cTextAttr);
		cTextAttr.SetTextColour(cSaveColor);
		mConsole->SetStyle(cPosE,cPosE,cTextAttr);
	}
	this->PrintMessage(aMessage,true);
}

void my1Form::PrintInfoMessage(const wxString& aMessage)
{
	this->PrintTaggedMessage(wxS("INFO"),aMessage,*wxBLUE);
}

void my1Form::PrintErrorMessage(const wxString& aMessage)
{
	this->PrintTaggedMessage(wxS("ERROR"),aMessage,*wxRED);
}

void my1Form::PrintValueDEC(int aValue, int aWidth)
{
	wxString cFormat = wxS("%d");
	if(aWidth>0) cFormat = wxString::Format(wxS("%%%dd"),aWidth);
	this->PrintMessage(wxString::Format(cFormat,aValue));
}

void my1Form::PrintValueHEX(int aValue, int aWidth)
{
	wxString cFormat = wxString::Format(wxS("%%0%dX"),aWidth);
	this->PrintMessage(wxString::Format(cFormat,aValue));
}

void my1Form::PrintMemoryContent(aword anAddress, int aSize)
{
	aword cAddress = anAddress;
	if(cAddress%PRINT_BPL_COUNT!=0)
		cAddress -= (cAddress%PRINT_BPL_COUNT);
	if(aSize%PRINT_BPL_COUNT!=0)
		aSize += (aSize%PRINT_BPL_COUNT);
	abyte cData;
	int cCount = 0;
	// print header!
	this->PrintMessage(wxS("\n--------"));
	for(int cLoop=0;cLoop<PRINT_BPL_COUNT;cLoop++)
		this->PrintMessage(wxS("-----"));
	this->PrintMessage(wxS("\n|      |"));
	for(int cLoop=0;cLoop<PRINT_BPL_COUNT;cLoop++)
	{
		this->PrintMessage(wxS(" "));
		this->PrintValueHEX(cLoop,2);
		this->PrintMessage(wxS(" |"));
	}
	// print table!
	while(cCount<aSize&&cAddress<MAX_MEMSIZE)
	{
		if(!m8085.MemoryMap().Read(cAddress,cData))
		{
			this->PrintMessage(wxS("\n"));
			this->PrintErrorMessage(wxS("Cannot read from address 0x")+
				wxString::Format(wxS("%04X!"),cAddress));
			break;
		}
		if(cCount%PRINT_BPL_COUNT==0)
		{
			this->PrintMessage(wxS("\n--------"));
			for(int cLoop=0;cLoop<PRINT_BPL_COUNT;cLoop++)
				this->PrintMessage(wxS("-----"));
			this->PrintMessage(wxS("\n| "));
			this->PrintValueHEX(cAddress,4);
			this->PrintMessage(wxS(" |"));
		}
		this->PrintMessage(wxS(" "));
		this->PrintValueHEX(cData,2);
		this->PrintMessage(wxS(" |"));
		cCount++; cAddress++;
	}
	this->PrintMessage(wxS("\n--------"));
	for(int cLoop=0;cLoop<PRINT_BPL_COUNT;cLoop++)
		this->PrintMessage(wxS("-----"));
	this->PrintMessage(wxS("\n"));
}

void my1Form::PrintPeripheralInfo(void)
{
	this->PrintMessage(wxS("\nMemory Count: "));
	this->PrintValueDEC(m8085.MemoryMap().GetCount());
	this->PrintMessage(wxS("\n"));
	my1Memory* cMemory = m8085.Memory(0);
	while(cMemory)
	{
		this->PrintMessage(wxS("(Memory) Name: "));
		this->PrintMessage(cMemory->GetName());
		this->PrintMessage(wxS(", "));
		this->PrintMessage(wxS("Read-Only: "));
		this->PrintMessage(cMemory->IsReadOnly()?wxS("YES"):wxS("NO "));
		this->PrintMessage(wxS(", "));
		this->PrintMessage(wxS("Start: 0x"));
		this->PrintValueHEX(cMemory->GetStart(),4);
		this->PrintMessage(wxS(", "));
		this->PrintMessage(wxS("Size: 0x"));
		this->PrintValueHEX(cMemory->GetSize(),4);
		this->PrintMessage(wxS("\n"));
		cMemory = (my1Memory*) cMemory->Next();
	}
	this->PrintMessage(wxS("Device Count: "));
	this->PrintValueDEC(m8085.DeviceMap().GetCount());
	this->PrintMessage(wxS("\n"));
	my1Device* cDevice = m8085.Device(0);
	while(cDevice)
	{
		this->PrintMessage(wxS("(Device) Name: "));
		this->PrintMessage(cDevice->GetName());
		this->PrintMessage(wxS(", "));
		this->PrintMessage(wxS("Start: 0x"));
		this->PrintValueHEX(cDevice->GetStart(),2);
		this->PrintMessage(wxS(", "));
		this->PrintMessage(wxS("Size: 0x"));
		this->PrintValueHEX(cDevice->GetSize(),2);
		this->PrintMessage(wxS("\n"));
		cDevice = (my1Device*) cDevice->Next();
	}
}

void my1Form::PrintHelp(void)
{
	mConsole->AppendText(wxS("\nAvailable command(s):\n"));
	mConsole->AppendText(wxS("- show [system|mem=?|minimv=?]\n"));
	mConsole->AppendText(wxS("  > system (print system info)\n"));
	mConsole->AppendText(wxS("  > info (print codex info)\n"));
	mConsole->AppendText(wxS("  > prev (print previous codex info)\n"));
	mConsole->AppendText(wxS("  > mem=? (show memory @ given addr)\n"));
	mConsole->AppendText(wxS("  > minimv=? (show minimv @ given addr)\n"));
	mConsole->AppendText(wxS("- build [default|reset|rom=?|ram=?|ppi=?]\n"));
	mConsole->AppendText(wxS("  > default (build default system)\n"));
	mConsole->AppendText(wxS("  > reset (reset system build)\n"));
	mConsole->AppendText(wxS("  > rom=? (add 2764 ROM @given addr)\n"));
	mConsole->AppendText(wxS("  > ram=? (add 6264 RAM @given addr)\n"));
	mConsole->AppendText(wxS("  > ppi=? (add 8255 PPI @given addr)\n"));
	mConsole->AppendText(wxS("- clear\n"));
	mConsole->AppendText(wxS("  > clear this console\n"));
	mConsole->AppendText(wxS("- help\n"));
	mConsole->AppendText(wxS("  > show this text\n"));
}

void my1Form::PrintUnknownCommand(const wxString& aCommand)
{
	mConsole->AppendText(wxS("\nUnknown command '"));
	mConsole->AppendText(aCommand);
	mConsole->AppendText(wxS("'\n"));
}

void my1Form::PrintUnknownParameter(const wxString& aParam,
	const wxString& aCommand)
{
	mConsole->AppendText(wxS("\nUnknown parameter '"));
	mConsole->AppendText(aParam);
	mConsole->AppendText(wxS("' for ["));
	mConsole->AppendText(aCommand);
	mConsole->AppendText(wxS("]\n"));
}

void my1Form::OnCheckFont(wxKeyEvent &event)
{
	if(!event.ControlDown()) event.Skip();
	int cSelect = mNoteBook->GetSelection();
	wxWindow *cTarget = mNoteBook->GetPage(cSelect);
	if(!cTarget->IsKindOf(wxCLASSINFO(my1CodeEdit))) return;
	my1CodeEdit *cEditor = (my1CodeEdit*) cTarget;
	int cKeyCode = event.GetKeyCode();
	switch(cKeyCode)
	{
		case WXK_NUMPAD_ADD:
			cEditor->LargerFont();
			break;
		case WXK_NUMPAD_SUBTRACT:
			cEditor->SmallerFont();
			break;
		default:
			event.Skip();
	}
}

void my1Form::OnCheckConsole(wxKeyEvent &event)
{
	//if(event.GetUnicodeKey() != WXK_NONE) // check if printable?
	int cKeyCode = event.GetKeyCode();
	switch(cKeyCode)
	{
		case WXK_UP:
			if(mCmdHistIndex>0)
			{
				mCmdHistIndex--;
				mCommand->Clear();
				mCommand->AppendText(mCmdHistory[mCmdHistIndex]);
			}
			break;
		case WXK_DOWN:
			if(mCmdHistIndex<(int)mCmdHistory.GetCount())
			{
				mCmdHistIndex++;
				mCommand->Clear();
				if(mCmdHistIndex<(int)mCmdHistory.GetCount())
					mCommand->AppendText(mCmdHistory[mCmdHistIndex]);
			}
			break;
		case WXK_RETURN:
			this->OnExecuteConsole((wxCommandEvent&)event);
			break;
		default:
			event.Skip();
	}
}

void my1Form::OnExecuteConsole(wxCommandEvent &event)
{
	bool cValidCommand = false;
	wxString cCommandLine = mCommand->GetLineText(0);
	mCommand->SelectAll(); mCommand->Cut();
	if(!cCommandLine.Length())
	{
		mConsole->AppendText("\n");
		return;
	}
	wxString cCommandWord = cCommandLine.BeforeFirst(' ');
	wxString cParameters = cCommandLine.AfterFirst(' ');
	if(!cCommandWord.Cmp(wxS("show")))
	{
		wxString cParam = cParameters.BeforeFirst(' ');
		int cEqual = cParam.Find('=');
		if(cEqual==wxNOT_FOUND)
		{
			if(!cParam.Cmp(wxS("system")))
			{
				this->PrintPeripheralInfo();
				cValidCommand = true;
			}
			else if(!cParam.Cmp(wxS("info")))
			{
				if(!mSimulationMode)
				{
					this->PrintMessage("Only available during simulation!");
					return;
				}
				m8085.PrintCodexInfo();
			}
			else if(!cParam.Cmp(wxS("prev")))
			{
				if(!mSimulationMode)
				{
					this->PrintMessage("Only available during simulation!");
					return;
				}
				m8085.PrintCodexPrev();
			}
			else
			{
				this->PrintUnknownParameter(cParameters,cCommandWord);
			}
		}
		else
		{
			wxString cKey = cParam.BeforeFirst('=');
			wxString cValue = cParam.AfterFirst('=');
			if(!cKey.Cmp(wxS("mem")))
			{
				unsigned long cStart;
				if(cValue.ToULong(&cStart,16)&&cStart<=0xFFFF)
				{
					this->PrintMemoryContent(cStart);
					cValidCommand = true;
				}
				else
				{
					this->PrintUnknownParameter(cValue,cKey);
				}
			}
			else if(!cKey.Cmp(wxS("minimv")))
			{
				unsigned long cStart;
				if(cValue.ToULong(&cStart,16)&&cStart<=0xFFFF)
				{
					this->CreateMemoryMiniPanel(cStart);
					cValidCommand = true;
				}
				else
				{
					this->PrintUnknownParameter(cValue,cKey);
				}
			}
			else
			{
				this->PrintUnknownParameter(cParameters,cCommandWord);
			}
		}
	}
	else if(!cCommandWord.Cmp(wxS("build")))
	{
		if(mSimulationMode)
		{
			this->PrintMessage("Build mode disabled during simulation!");
			return;
		}
		if(!mShowSystem)
		{
			this->PrintMessage("Only available when system is enabled!");
			return;
		}
		wxString cParam = cParameters.BeforeFirst(' ');
		int cEqual = cParam.Find('=');
		if(cEqual==wxNOT_FOUND)
		{
			if(!cParam.Cmp(wxS("default")))
			{
				this->SystemDefault();
				cValidCommand = true;
			}
			else if(!cParam.Cmp(wxS("reset")))
			{
				this->SystemDisconnect();
				cValidCommand = true;
			}
			else
			{
				this->PrintUnknownParameter(cParameters,cCommandWord);
			}
		}
		else
		{
			wxString cKey = cParam.BeforeFirst('=');
			wxString cValue = cParam.AfterFirst('=');
			if(!cKey.Cmp(wxS("rom")))
			{
				unsigned long cStart;
				if(cValue.ToULong(&cStart,16)&&cStart<=0xFFFF)
					this->ConnectROM(cStart);
				else
					this->PrintUnknownParameter(cValue,cKey);
				cValidCommand = true;
			}
			else if(!cKey.Cmp(wxS("ram")))
			{
				unsigned long cStart;
				if(cValue.ToULong(&cStart,16)&&cStart<=0xFFFF)
					this->ConnectRAM(cStart);
				else
					this->PrintUnknownParameter(cValue,cKey);
				cValidCommand = true;
			}
			else if(!cKey.Cmp(wxS("ppi")))
			{
				unsigned long cStart;
				if(cValue.ToULong(&cStart,16)&&cStart<=0xFF)
					this->ConnectPPI(cStart);
				else
					this->PrintUnknownParameter(cValue,cKey);
				cValidCommand = true;
			}
			else
			{
				this->PrintUnknownParameter(cParameters,cCommandWord);
			}
		}
	}
	else if(!cCommandWord.Cmp(wxS("clear")))
	{
		mConsole->Clear();
		mConsole->AppendText(wxString::Format(wxS("Welcome to %s\n\n"),
			MY1APP_TITLE));
		cValidCommand = true;
	}
	else if(!cCommandWord.Cmp(wxS("help")))
	{
		this->PrintHelp();
		cValidCommand = true;
	}
	else if(!cCommandWord.Cmp(wxS("test")))
	{
		this->PrintMessage("\nNothing to test!",true);
		cValidCommand = true;
	}
	else
	{
		this->PrintUnknownCommand(cCommandWord);
	}
	if(cValidCommand)
	{
		if(!mCmdHistory.GetCount()||mCmdHistory.Last()!=cCommandLine)
		{
			mCmdHistory.Add(cCommandLine);
			if(mCmdHistory.GetCount()>CMD_HISTORY_COUNT)
				mCmdHistory.RemoveAt(0);
		}
	}
	// reset command history index
	mCmdHistIndex = mCmdHistory.GetCount();
}

void my1Form::OnSimulationPick(wxCommandEvent &event)
{
	if(!mSimulationMode)
		return;
	switch(event.GetId())
	{
		case MY1ID_SIMSEXEC:
			if(mSimulationStepping)
				mSimulationRunning = true;
			else
				mSimulationRunning = !mSimulationRunning;
			mSimulationStepping = false;
			break;
		case MY1ID_SIMSSTEP:
			mSimulationRunning = true;
			mSimulationStepping = true;
			break;
		default:
			mSimulationRunning = false;
			mSimulationStepping = false;
	}
	if(mSimulationRunning)
	{
		my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
		if(cEditor) cEditor->ShowLine(mSimulationStepping);
		if(mSimulationStepping)
			this->SetStatusText(MSG_SYSTEM_SSIM,STATUS_SYS_INDEX);
		else
			this->SetStatusText(MSG_SYSTEM_RSIM,STATUS_SYS_INDEX);
		if(!mSimExecTimer->IsRunning())
			mSimExecTimer->Start(SIM_EXEC_PERIOD,wxTIMER_ONE_SHOT);
	}
	else
	{
		this->SetStatusText(MSG_SYSTEM_MSIM,STATUS_SYS_INDEX);
	}
}

void my1Form::OnSimulationInfo(wxCommandEvent &event)
{
	if(!mSimulationMode)
		return;
	if(event.GetId()==MY1ID_SIMSINFO)
	{
		m8085.PrintCodexInfo();
	}
	else if(event.GetId()==MY1ID_SIMSPREV)
	{
		m8085.PrintCodexPrev();
	}
	else if(event.GetId()==MY1ID_SIMSMIMV)
	{
		this->CreateMemoryMiniPanel();
	}
	else if(event.GetId()==MY1ID_SIMRESET)
	{
		if(mSimExecTimer->IsRunning())
			mSimExecTimer->Stop();
		this->SetStatusText(MSG_SYSTEM_MSIM,STATUS_SYS_INDEX);
		m8085.Simulate(0);
		my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
		if(cEditor) cEditor->ExecLine(m8085.GetCodexLine()-1);
	}
	else if(event.GetId()==MY1ID_SIMSBRKP)
	{
		my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
		if(cEditor) cEditor->ToggleBreak(cEditor->GetCurrentLine());
	}
	else if(event.GetId()==MY1ID_SIMSEXIT)
	{
		if(mSimExecTimer->IsRunning())
			mSimExecTimer->Stop();
		my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
		cEditor->ExecDone();
		m8085.SetCodeLink((void*)0x0);
		mSimulationRunning = false;
		mSimulationStepping = false;
		this->SimulationMode(false);
		cEditor->SetReadOnly(mSimulationMode);
	}
}

int my1Form::GetBuildAddress(const wxString& aString)
{
	wxTextEntryDialog* cDialog = new wxTextEntryDialog(this,
		wxS("Enter Address in HEX"), aString);
	if(cDialog->ShowModal()!=wxID_OK)
		return -1;
	unsigned long cStart = 0x0;
	cDialog->GetValue().ToULong(&cStart,16);
	return cStart;
}

void my1Form::OnBuildSelect(wxCommandEvent &event)
{
	if(mSimulationMode)
	{
		this->PrintMessage("Build mode disabled during simulation!\n");
		return;
	}
	int cAddress;
	switch(event.GetId())
	{
		case MY1ID_BUILDRST:
			this->SystemDisconnect();
			break;
		case MY1ID_BUILDDEF:
			this->SystemDefault();
			break;
		case MY1ID_BUILDNFO:
			this->PrintPeripheralInfo();
			break;
		case MY1ID_BUILDROM:
			cAddress = this->GetBuildAddress(wxS("[BUILD] Adding 2764 ROM"));
			if(cAddress<0) return;
			this->ConnectROM(cAddress);
			break;
		case MY1ID_BUILDRAM:
			cAddress = this->GetBuildAddress(wxS("[BUILD] Adding 6264 RAM"));
			if(cAddress<0) return;
			this->ConnectRAM(cAddress);
			break;
		case MY1ID_BUILDPPI:
			cAddress = this->GetBuildAddress(wxS("[BUILD] Adding 8255 PPI"));
			if(cAddress<0) return;
			this->ConnectPPI(cAddress);
			break;
	}
}

void my1Form::OnClosePane(wxAuiManagerEvent &event)
{
	wxAuiPaneInfo *cPane = event.GetPane();
	// rearrange if a toolbar
	if(cPane->IsToolbar())
	{
		wxAuiPaneInfo& cPaneDevC = mMainUI.GetPane(wxS("devcTool"));
		if(cPaneDevC.IsOk()&&cPaneDevC.IsDocked()&&cPaneDevC.IsShown())
			cPaneDevC.Position(TOOL_DEVC_POS);
		wxAuiPaneInfo& cPaneProc = mMainUI.GetPane(wxS("procTool"));
		if(cPaneProc.IsOk()&&cPaneProc.IsDocked()&&cPaneProc.IsShown())
			cPaneProc.Position(TOOL_PROC_POS);
	}
	// browse for mini mem viewer!
	my1MiniViewer *pViewer = mFirstViewer, *pPrev = 0x0;
	while(pViewer)
	{
		wxString cPanelName = wxS("miniMV")
			+ wxString::Format(wxS("%04X"),pViewer->mStart);
		wxAuiPaneInfo &tPane = mMainUI.GetPane(cPanelName);
		if(cPane==&tPane)
		{
			if(!pPrev)
				mFirstViewer = pViewer->mNext;
			else
				pPrev->mNext = pViewer->mNext;
			delete pViewer;
			break;
		}
		pPrev = pViewer;
		pViewer = pViewer->mNext;
	}
}

void my1Form::OnShowSystem(wxCommandEvent &event)
{
	mShowSystem = event.IsChecked();
	wxMenuBar *mainMenu = this->GetMenuBar();
	wxMenuItem *pMenuItem = mainMenu->FindItem(MY1ID_SYSTEM,0x0);
	if(pMenuItem) pMenuItem->Check(mShowSystem);
	mainMenu->EnableTop(mainMenu->FindMenu(wxS("System")),mShowSystem);
	pMenuItem = mainMenu->FindItem(MY1ID_SIMULATE,0x0);
	if(pMenuItem) pMenuItem->Enable(mShowSystem);
	wxAuiToolBar *pTool = (wxAuiToolBar*) this->FindWindow(MY1ID_PROCTOOL);
	pTool->EnableTool(MY1ID_SIMULATE,mShowSystem);
	if(mFileTool->GetToolToggled(MY1ID_SYSTEM)!=mShowSystem)
		mFileTool->ToggleTool(MY1ID_SYSTEM, mShowSystem);
	wxAuiPaneInfo& cPaneSyst = mMainUI.GetPane(wxS("systPanel"));
	if(cPaneSyst.IsOk())
		cPaneSyst.Dock().Left().Layer(AUI_OUTER_LAYER).Show(mShowSystem);
	wxAuiPaneInfo& cPaneRegs = mMainUI.GetPane(wxS("regsPanel"));
	if(cPaneRegs.IsOk())
		cPaneRegs.Dock().Left().Layer(AUI_EXTER_LAYER).Show(mShowSystem);
	wxAuiPaneInfo& cPaneIntr = mMainUI.GetPane(wxS("intrPanel"));
	if(cPaneIntr.IsOk())
		cPaneIntr.Dock().Top().Show(mShowSystem);
	wxAuiPaneInfo& cPaneDevC = mMainUI.GetPane(wxS("devcTool"));
	if(cPaneDevC.IsOk())
		cPaneDevC.Dock().Top().Position(TOOL_DEVC_POS).Show(mShowSystem);
	// delete created devices/controls?
	if(!mShowSystem) this->RemoveControls();
	mMainUI.Update();
}

void my1Form::OnShowPanel(wxCommandEvent &event)
{
	wxString cToolName = wxS("");
	switch(event.GetId())
	{
		case MY1ID_VIEW_SYSTPANE:
			cToolName = wxS("systPanel");
			break;
		case MY1ID_VIEW_REGSPANE:
			cToolName = wxS("regsPanel");
			break;
		case MY1ID_VIEW_INTRPANE:
			cToolName = wxS("intrPanel");
			break;
		case MY1ID_VIEW_CONSPANE:
			cToolName = wxS("consPanel");
			break;
		case MY1ID_VIEW_TERMPANE:
			cToolName = wxS("termPanel");
			break;
		case MY1ID_CREATE_MINIMV:
			this->CreateMemoryMiniPanel();
			break;
		case MY1ID_CREATE_DV7SEG:
			this->CreateDevice7SegPanel();
			break;
		case MY1ID_CREATE_DVKPAD:
			this->CreateDeviceKPadPanel();
			break;
		case MY1ID_CREATE_DEVLED:
			this->CreateDeviceLEDPanel();
			break;
		case MY1ID_CREATE_DEVSWI:
			this->CreateDeviceSWIPanel();
			break;
		case MY1ID_CREATE_DEVBUT:
			this->CreateDeviceBUTPanel();
			break;
		case MY1ID_CREATE_DEVLVD:
			this->CreateDeviceLEDPanel(wxEmptyString,true);
			break;
	}
	if(cToolName.Length()>0)
	{
		wxAuiPaneInfo& cPane = mMainUI.GetPane(cToolName);
		if(cPane.IsOk())
		{
			cPane.Show();
			mMainUI.Update();
		}
	}
	return;
}

void my1Form::OnCheckOptions(wxCommandEvent &event)
{
	my1OptionDialog *prefDialog = new my1OptionDialog(this,
		wxS("Options"),this->mOptions);
	prefDialog->ShowModal();
	prefDialog->Destroy();
	if(this->mOptions.mChanged)
	{
		this->mOptions.mChanged = false;
		int cCount = mNoteBook->GetPageCount();
		for(int cLoop=0;cLoop<cCount;cLoop++)
		{
			// set for all opened editor?
			wxWindow *cTarget = mNoteBook->GetPage(cLoop);
			if(cTarget->IsKindOf(wxCLASSINFO(my1CodeEdit)))
			{
				my1CodeEdit *cEditor = (my1CodeEdit*) cTarget;
				cEditor->SetViewEOL(this->mOptions.mEdit_ViewEOL);
				cEditor->SetViewWhiteSpace(this->mOptions.mEdit_ViewWS?1:0);
				cEditor->Refresh();
			}
		}
	}
}

void my1Form::OnStatusTimer(wxTimerEvent& event)
{
	this->SetStatusText(wxS(""),STATUS_MSG_INDEX);
}

void my1Form::OnSimExeTimer(wxTimerEvent& event)
{
	bool cWasHalted = m8085.Halted();
	if(m8085.Simulate())
	{
		my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
		cEditor->ExecLine(m8085.GetCodexLine()-1,mSimulationStepping);
		if(mOptions.mSims_ShowRunInfo)
			m8085.PrintCodexPrev();
		if(cEditor->IsBreakLine(m8085.GetCodexLine()-1))
			mSimulationStepping = true;
		if(m8085.NoCodex())
		{
			this->PrintInfoMessage("No code @ address!");
			mSimulationStepping = true;
		}
		else if(m8085.Halted())
		{
			if(!cWasHalted) this->PrintInfoMessage("System HALTED!");
			mSimulationStepping = mOptions.mSims_PauseOnHALT;
		}
		else if(m8085.Interrupted())
		{
			this->PrintInfoMessage("System Interrupt!");
			mSimulationStepping = mOptions.mSims_PauseOnINTR;
		}
	}
	else
	{
		my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
		wxMessageBox(wxS("Simulation Terminated!"),wxS("[SIM Error]"),
			wxOK|wxICON_EXCLAMATION);
		mSimulationRunning = false;
		this->SimulationMode(false);
		cEditor->SetReadOnly(mSimulationMode);
	}
	if(mSimulationRunning&&!mSimulationStepping)
		mSimExecTimer->Start(SIM_EXEC_PERIOD,wxTIMER_ONE_SHOT);
}

void my1Form::OnPageChanging(wxAuiNotebookEvent &event)
{
	if(mSimulationMode)
		event.Veto();
}

void my1Form::OnPageChanged(wxAuiNotebookEvent &event)
{
	wxWindow *cTarget = mNoteBook->GetPage(event.GetSelection());
	if(!cTarget) return;
	m8085.SetCodeLink((void*)0x0);
	bool cEditMode = cTarget->IsKindOf(wxCLASSINFO(my1CodeEdit));
	wxMenuBar *cMenuBar = this->GetMenuBar();
	cMenuBar->EnableTop(cMenuBar->FindMenu(wxS("Tool")),cEditMode);
	wxAuiPaneInfo& cPaneProc = mMainUI.GetPane(wxS("procTool"));
	if(cPaneProc.IsOk())
	{
		wxAuiToolBar *pTool = (wxAuiToolBar*)
			this->FindWindow(MY1ID_PROCTOOL);
		pTool->EnableTool(MY1ID_ASSEMBLE,cEditMode);
		pTool->EnableTool(MY1ID_GENERATE,cEditMode);
		pTool->EnableTool(MY1ID_SIMULATE,cEditMode&&mShowSystem);
		cPaneProc.Dock().Top().Position(TOOL_PROC_POS).
			Show(cEditMode||mShowSystem);
	}
	mMainUI.Update();
}

void my1Form::OnPageClosing(wxAuiNotebookEvent &event)
{
	wxWindow *cTarget = mNoteBook->GetPage(event.GetSelection());
	if(cTarget->IsKindOf(wxCLASSINFO(my1CodeEdit)))
	{
		my1CodeEdit *cEditor = (my1CodeEdit*) cTarget;
		if(cEditor->GetModify())
		{
			int cGoSave = wxMessageBox(wxS("Save Before Closing?"),
				wxS("Code Modified!"),wxYES_NO|wxCANCEL,this);
			if(cGoSave==wxYES)
				this->SaveEdit(cTarget);
			else if(cGoSave==wxCANCEL)
				event.Veto();
		}
	}
	else if(!cTarget->IsKindOf(wxCLASSINFO(wxTextCtrl)))
	{
		event.Veto(); // welcome page is always visible!
	}
}

void my1Form::OnBITPanelClick(wxMouseEvent &event)
{
	mPortPanel = 0x0;
	if(event.RightDown())
	{
		wxMenu *cMenuPop = this->GetDevicePortMenu();
		if(!cMenuPop) { event.Skip(); return; }
		mPortPanel = (wxPanel*) FindWindowById(event.GetId(),this);
		if(!mPortPanel) { event.Skip(); return; }
		mPortPanel->Connect(wxID_ANY,wxEVT_COMMAND_MENU_SELECTED,
			WX_CEH(my1Form::OnBITPortClick),NULL,this);
		mPortPanel->PopupMenu(cMenuPop);
	}
	else if(event.LeftDClick())
	{
		wxWindow* pTarget = FindWindowById(event.GetId(),this);
		wxAuiPaneInfo& cPane = mMainUI.GetPane(pTarget);
		if(cPane.IsOk())
		{
			wxString cLabel = mMainUI.SavePaneInfo(cPane);
			cLabel = cLabel.Mid(cLabel.First(wxS("caption=")));
			cLabel = cLabel.BeforeFirst(';');
			cLabel = cLabel.AfterFirst('=');
			wxTextEntryDialog* cDialog = new wxTextEntryDialog(this,
				wxS("Enter new caption"), wxS("Changing Caption - ")+cLabel);
			if(cDialog->ShowModal()!=wxID_OK)
				return;
			wxString cCaption = cDialog->GetValue();
			if(cCaption.Length())
			{
				cPane.Caption(cCaption);
				cPane.CaptionVisible();
				mMainUI.Update();
			}
			else
			{
				cPane.CaptionVisible(false);
				mMainUI.Update();
			}
		}
	}
	else event.Skip();
}

void my1Form::OnBITPortClick(wxCommandEvent &event)
{
	int cCheck = event.GetId() - MY1ID_CPOT_OFFSET;
	if(cCheck<0||cCheck>=MY1ID_CBIT_OFFSET) return;
	int cDevice = cCheck/(I8255_SIZE-1);
	int cDevIdx = cCheck%(I8255_SIZE-1);
	my1Device *pDevice = m8085.Device(cDevice);
	if(!pDevice) { mPortPanel = 0x0; return; }
	my1DevicePort *pPort = pDevice->GetDevicePort(cDevIdx);
	if(!pPort) { mPortPanel = 0x0; return; }
	wxWindowList& cList = mPortPanel->GetChildren();
	if((int)cList.GetCount()<=0)  { mPortPanel = 0x0; return; }
	wxWindowList::Node *pNode = 0x0;
	for(int cLoop=I8255_DATASIZE;cLoop>0;cLoop--)
	{
		wxWindow *pTarget = 0x0;
		do
		{
			if(!pNode) pNode = cList.GetFirst();
			else pNode = pNode->GetNext();
			if(!pNode) break;
			pTarget = (wxWindow*) pNode->GetData();
			if(pTarget->IsKindOf(wxCLASSINFO(my1BITCtrl)))
				break;
			pTarget = 0x0;
		}
		while(1);
		if(!pTarget)
		{
			this->PrintErrorMessage("Cannot Assign Port!");
			mPortPanel = 0x0;
			break;
		}
		my1BITCtrl *pCtrl = (my1BITCtrl*) pTarget;
		if(pCtrl->IsDummy()) continue;
		pCtrl->LinkBreak();
		my1BitIO *pBit = pPort->GetBitIO(cLoop-1);
		my1BitSelect cLink;
		cLink.mDevice = cDevice;
		cLink.mDevicePort = cDevIdx;
		cLink.mDeviceBit = cLoop-1;
		cLink.mDeviceAddr = pDevice->GetStart();
		cLink.mPointer = (void*) pBit;
		// assign new link
		pCtrl->LinkCheck(cLink);
	}
}

my1BitIO* my1Form::GetDeviceBit(my1BitSelect& aSelect,bool useAddress)
{
	my1BitIO *pBit = 0x0;
	// check if interrupt pin
	if(aSelect.mDevice<0)
	{
		my1BitIO& cBit = m8085.Pin(aSelect.mDeviceBit);
		cBit.Unlink();
		pBit = &cBit;
		aSelect.mPointer = (void*) pBit;
		return pBit;
	}
	int cExtra;
	my1Device *pDevice = 0x0;
	my1DeviceMap85 &pMap = m8085.DeviceMap();
	if(useAddress)
		pDevice = (my1Device*) pMap.Object((aword)aSelect.mDeviceAddr,&cExtra);
	else
		pDevice = (my1Device*) pMap.Object(aSelect.mDevice,&cExtra);
	if(pDevice)
	{
		my1DevicePort *pPort = pDevice->GetDevicePort(aSelect.mDevicePort);
		if(pPort) pBit = pPort->GetBitIO(aSelect.mDeviceBit);
		if(pBit)
		{
			aSelect.mPointer = (void*) pBit;
			if(useAddress) aSelect.mDevice = cExtra;
			else aSelect.mDeviceAddr = cExtra;
		}
	}
	else
		wxMessageBox(wxString::Format("No device? (%02x)",cExtra),
			wxS("[DEBUG]"),wxOK|wxICON_INFORMATION);
	return pBit;
}

void my1Form::UpdateDeviceBit(bool unLink)
{
	my1Device *pDevice = m8085.Device(0);
	while(pDevice)
	{
		for(int cPort=0;cPort<I8255_SIZE-1;cPort++)
		{
			my1DevicePort *pPort = pDevice->GetDevicePort(cPort);
			for(int cLoop=0;cLoop<I8255_DATASIZE;cLoop++)
			{
				my1BitIO *pBitIO = pPort->GetBitIO(cLoop);
				if(unLink) { pBitIO->Unlink(); continue; }
				my1BITCtrl *pCtrl = (my1BITCtrl*) pBitIO->GetLink();
				if(pCtrl)
				{
					int cIndex;
					my1BitSelect& cLink = pCtrl->Link();
					aword cAddress = cLink.mDeviceAddr;
					if(m8085.DeviceMap().Object(cAddress,&cIndex))
						cLink.mDevice = cIndex;
					else
						pCtrl->LinkBreak();
				}
			}
		}
		pDevice = (my1Device*) pDevice->Next();
	}
}

wxMenu* my1Form::GetDevicePopupMenu(void)
{
	if(!m8085.DeviceMap().GetCount())
	{
		wxMessageBox(wxS("Build a system with PPI!"),
				wxS("System Incomplete!"),wxOK|wxICON_EXCLAMATION,this);
		return 0x0;
	}
	if(!mDevicePopupMenu)
	{
		mDevicePopupMenu = new wxMenu;
		int cDevID = MY1ID_DSEL_OFFSET;
		int cBitID = MY1ID_CBIT_OFFSET;
		my1Device *pDevice = m8085.Device(0);
		while(pDevice)
		{
			wxMenu *cMenuBit = new wxMenu;
			for(int cPort=0;cPort<I8255_SIZE-1;cPort++)
			{
				wxString cPortText = wxS("P") +
					wxString::Format(wxS("%c"),(char)(cPort+(int)'A'));
				for(int cLoop=0;cLoop<I8255_DATASIZE;cLoop++)
				{
					wxString cText = cPortText +
						wxString::Format(wxS("%01X"),cLoop);
					cMenuBit->Append(cBitID++,cText,
						wxEmptyString,wxITEM_CHECK);
				}
			}
			wxString cText = wxS("Device @") +
				wxString::Format(wxS("%02X"),pDevice->GetStart());
			mDevicePopupMenu->Append(cDevID++, cText, cMenuBit);
			pDevice = (my1Device*) pDevice->Next();
		}
		// add 8085 interrupt pins!
		{
			int cIntID = MY1ID_8085_OFFSET;
			wxMenu *cMenuBit = new wxMenu;
			wxString cText = wxS("INT: TRAP");
			cMenuBit->Append(cIntID++,cText,wxEmptyString,wxITEM_CHECK);
			cText = wxS("INT: I7.5");
			cMenuBit->Append(cIntID++,cText,wxEmptyString,wxITEM_CHECK);
			cText = wxS("INT: I6.5");
			cMenuBit->Append(cIntID++,cText,wxEmptyString,wxITEM_CHECK);
			cText = wxS("INT: I5.5");
			cMenuBit->Append(cIntID++,cText,wxEmptyString,wxITEM_CHECK);
			cText = wxS("Interrupt Pins");
			mDevicePopupMenu->AppendSeparator();
			mDevicePopupMenu->Append(cDevID++, cText, cMenuBit);
		}
		// add other options
		{
			mDevicePopupMenu->AppendSeparator();
			mDevicePopupMenu->Append(MY1ID_CHANGE_LABEL,
				wxS("Change Label"));
			mDevicePopupMenu->AppendSeparator();
			mDevicePopupMenu->AppendCheckItem(MY1ID_TOGGLE_ACTLVL,
				wxS("Active Low"));
		}
	}
	// make sure all items are unchecked? minus separator and interrupt!
	int cCountD = mDevicePopupMenu->GetMenuItemCount();
	for(int cLoopD=0;cLoopD<cCountD;cLoopD++)
	{
		wxMenuItem *cItemD = mDevicePopupMenu->FindItemByPosition(cLoopD);
		if(cItemD->IsSeparator()) break;
		wxMenu *cMenuD = cItemD->GetSubMenu();
		int cCountB = cMenuD->GetMenuItemCount();
		for(int cLoopB=0;cLoopB<cCountB;cLoopB++)
		{
			wxMenuItem *cItem = cMenuD->FindItemByPosition(cLoopB);
			cItem->Check(false);
			int cCheck = cItem->GetId() - MY1ID_CBIT_OFFSET;
			my1BitSelect cSelect;
			cSelect.UseIndex(cCheck);
			my1BitIO* pBit = this->GetDeviceBit(cSelect);
			if(pBit->GetLink())
				cItem->Enable(false);
		}
	}
	// interrupt as well?
	{
		int cIntID = MY1ID_8085_OFFSET;
		for(int cLoop=0;cLoop<4;cLoop++,cIntID++)
		{
			wxMenuItem *cItem = mDevicePopupMenu->FindItem(cIntID);
			cItem->Check(false);
			int cCheck = cIntID - MY1ID_8085_OFFSET;
			my1BitIO* pBit = &m8085.Pin(cCheck);
			if(pBit->GetLink())
				cItem->Enable(false);
		}
	}
	return mDevicePopupMenu;
}

void my1Form::ResetDevicePopupMenu(bool unLink)
{
	this->UpdateDeviceBit(unLink);
	if(mDevicePopupMenu)
	{
		delete mDevicePopupMenu;
		mDevicePopupMenu = 0x0;
	}
}

wxMenu* my1Form::GetDevicePortMenu(void)
{
	if(mDevicePortMenu)
	{
		delete mDevicePortMenu;
		mDevicePortMenu = 0x0;
	}
	if(!m8085.DeviceMap().GetCount())
	{
		wxMessageBox(wxS("Build a system with PPI!"),
				wxS("System Incomplete!"),wxOK|wxICON_EXCLAMATION,this);
		return 0x0;
	}
	mDevicePortMenu = new wxMenu;
	int cPortID = MY1ID_CPOT_OFFSET;
	my1Device *pDevice = m8085.Device(0);
	while(pDevice)
	{
		for(int cPort=0;cPort<I8255_SIZE-1;cPort++)
		{
			wxString cPortText = wxS("P") +
				wxString::Format(wxS("%c"),(char)(cPort+(int)'A')) +
				wxString::Format(wxS(" @%02X"),pDevice->GetStart()+cPort);
			wxMenuItem* cItem = mDevicePortMenu->Append(cPortID++,cPortText);
			my1DevicePort *pPort = pDevice->GetDevicePort(cPort);
			for(int cLoop=0;cLoop<I8255_DATASIZE;cLoop++)
			{
				my1BitIO* pBit = pPort->GetBitIO(cLoop);
				if(pBit->GetLink())
				{
					cItem->Enable(false);
					break;
				}
			}
		}
		pDevice = (my1Device*) pDevice->Next();
	}
	return mDevicePortMenu;
}

void my1Form::UpdateMemoryPanel(void)
{
	if(!mMemoryGrid) return;
	my1Memory *pMemory = m8085.Memory(0);
	while(pMemory)
	{
 		wxGrid *pGrid = (wxGrid*) pMemory->GetLink();
		if(!pGrid)
		{
			pGrid = mMemoryGrid;
			pMemory->SetLink((void*)pGrid);
			pMemory->DoUpdate = &this->SimUpdateMEM;
			int cAddress = pMemory->GetStart();
			for(int cLoop=0;cLoop<(int)pMemory->GetSize();cLoop++)
			{
				abyte cData = 0x00;
				pMemory->GetData(cAddress,cData);
				int cCol = cAddress%MEM_VIEW_WIDTH;
				int cRow = cAddress/MEM_VIEW_WIDTH;
				pGrid->SetCellValue(cRow,cCol,
					wxString::Format(wxS("%02X"),cData));
				cAddress++;
			}
		}
		pMemory = (my1Memory*) pMemory->Next();
	}
}

void my1Form::SimUpdateFLAG(void* simObject)
{
	// update flag if necessary?
	my1Reg85 *pReg85 = (my1Reg85*) simObject;
	wxString cFlag = wxString::Format(wxS("%01X"),
			pReg85->GetData()&I8085_FLAG_C?1:0);
	my1Panel *pText = (my1Panel*) this->FlagLink(I8085_FLAG_C).GetLink();
	pText->SetText(cFlag);
	cFlag = wxString::Format(wxS("%01X"),
			pReg85->GetData()&I8085_FLAG_P?1:0);
	pText = (my1Panel*) this->FlagLink(I8085_FLAG_P).GetLink();
	pText->SetText(cFlag);
	cFlag = wxString::Format(wxS("%01X"),
			pReg85->GetData()&I8085_FLAG_A?1:0);
	pText = (my1Panel*) this->FlagLink(I8085_FLAG_A).GetLink();
	pText->SetText(cFlag);
	cFlag = wxString::Format(wxS("%01X"),
			pReg85->GetData()&I8085_FLAG_Z?1:0);
	pText = (my1Panel*) this->FlagLink(I8085_FLAG_Z).GetLink();
	pText->SetText(cFlag);
	cFlag = wxString::Format(wxS("%01X"),
			pReg85->GetData()&I8085_FLAG_S?1:0);
	pText = (my1Panel*) this->FlagLink(I8085_FLAG_S).GetLink();
	pText->SetText(cFlag);
}

my1SimObject& my1Form::FlagLink(int aMask)
{
	my1SimObject* pObject = &mFlagLink[1]; // UNUSED IN 8085!
	int cFlag = I8085_FLAG_C;
	for(int cLoop=0;cLoop<I8085_BIT_COUNT;cLoop++)
	{
		if(cFlag&aMask)
		{
			pObject = &mFlagLink[cLoop];
			break;
		}
		cFlag <<= 1;
	}
	return *pObject;
}

bool my1Form::RemoveControls(void)
{
	bool cFlag = true;
	wxWindowList::Node *pNode = mDevPanels.GetFirst();
	while(pNode)
	{
		wxWindow *pTarget = (wxWindow*) pNode->GetData();
		pNode = pNode->GetNext();
		if(mMainUI.DetachPane(pTarget))
		{
			this->PrintInfoMessage("Deleted a Panel!");
			mDevPanels.DeleteContents(true);
			mDevPanels.DeleteObject(pTarget);
			mDevPanels.DeleteContents(false);
			mMainUI.Update();
		}
		//else  // shouldn't happen
		//{
		//	cFlag = false;
		//	wxMessageBox(wxString::Format("WinID: '%d'",pTarget->GetId()),
		//		wxS("[CANNOT DETACH PANE!]"),wxOK|wxICON_WARNING);
		//}
	}
	return cFlag;
}

bool my1Form::SystemDefault(void)
{
	bool cFlag = true;
	cFlag &= this->SystemDisconnect();
	cFlag &= this->ConnectROM();
	cFlag &= this->ConnectRAM();
	cFlag &= this->ConnectPPI();
	if(cFlag)
	{
		this->PrintInfoMessage("Default system built!");
		// default switch panel
		my1DEVPanel*  pSWIPanel = this->CreateDeviceSWIPanel();
		wxAuiPaneInfo& cPaneSWI = mMainUI.GetPane(pSWIPanel);
		cPaneSWI.Caption("PortB @80");
		mMainUI.Update();
		if(!this->LinkPanelToPort(pSWIPanel,1))
			this->PrintErrorMessage("Cannot link switch panel!");
		// default led panel
		my1DEVPanel* pLEDPanel = this->CreateDeviceLEDPanel();
		wxAuiPaneInfo& cPaneLED = mMainUI.GetPane(pLEDPanel);
		cPaneLED.Caption("PortA @80");
		mMainUI.Update();
		if(!this->LinkPanelToPort(pLEDPanel,0))
			this->PrintErrorMessage("Cannot link LED panel!");
		// update main memory display
		this->UpdateMemoryPanel();
	}
	else
		this->PrintErrorMessage("Default system build FAILED!");
	return cFlag;
}

bool my1Form::SystemDisconnect(void)
{
	this->ResetDevicePopupMenu(true);
	bool cFlag = m8085.DisconnectALL();
	cFlag &= this->RemoveControls();
	if(cFlag)
		this->PrintInfoMessage("System build reset!");
	else
		this->PrintErrorMessage("System build reset FAILED!");
	mNoteBook->SetSelection(0);
	return cFlag;
}

bool my1Form::ConnectROM(int aStart)
{
	bool cFlag = false;
	if(aStart<0) return cFlag;
	if(aStart%I2764_SIZE!=0)
	{
		wxString cTest = wxS("2764 ROM start address should be");
		cTest += wxString::Format(wxS("multiple of 0x%04X!"),I2764_SIZE);
		wxMessageBox(cTest,wxS("Anomaly Detected!"),
			wxOK|wxICON_EXCLAMATION,this);
		return false;
	}
	wxString cTag = wxString::Format(wxS("@[%04X]!"),aStart);
	if((cFlag=m8085.ConnectROM(aStart)))
		this->PrintInfoMessage(wxS("2764 ROM added ")+cTag);
	else
		this->PrintErrorMessage(wxS("Cannot add 2764 ROM ")+cTag);
	if(cFlag) this->UpdateMemoryPanel();
	return cFlag;
}

bool my1Form::ConnectRAM(int aStart)
{
	bool cFlag = false;
	if(aStart<0) return cFlag;
	if(aStart%I6264_SIZE!=0)
	{
		wxString cTest = wxS("6264 RAM start address should be");
		cTest += wxString::Format(wxS("multiple of 0x%04X!"),I6264_SIZE);
		wxMessageBox(cTest,wxS("Anomaly Detected!"),
			wxOK|wxICON_EXCLAMATION,this);
		return false;
	}
	wxString cTag = wxString::Format(wxS("@[%04X]!"),aStart);
	if((cFlag=m8085.ConnectRAM(aStart)))
		this->PrintInfoMessage(wxS("6264 RAM added ")+cTag);
	else
		this->PrintErrorMessage(wxS("Cannot add 6264 RAM ")+cTag);
	if(cFlag) this->UpdateMemoryPanel();
	return cFlag;
}

bool my1Form::ConnectPPI(int aStart)
{
	bool cFlag = false;
	if(aStart<0) return cFlag;
	if(aStart%I8255_SIZE!=0)
	{
		wxString cTest = wxS("8255 PPI start address should be");
		cTest += wxString::Format(wxS("multiple of 0x%02X!"),I8255_SIZE);
		wxMessageBox(cTest,wxS("Anomaly Detected!"),
			wxOK|wxICON_EXCLAMATION,this);
		return false;
	}
	wxString cTag = wxString::Format(wxS("@[%02X]!"),aStart);
	if((cFlag=m8085.ConnectPPI(aStart)))
		this->PrintInfoMessage(wxS("8255 PPI added ")+cTag);
	else
		this->PrintErrorMessage(wxS("Cannot add 8255 PPI ")+cTag);
	if(cFlag) this->ResetDevicePopupMenu();
	return cFlag;
}

bool my1Form::LoadSystem(const wxString& aFilename)
{
	bool cFlag = true;
	wxFileInputStream cRead(aFilename);
	wxFileConfig cSystem(cRead);
	wxString cVal, cKey=wxS("/System/my1sim85key");
	if(!cSystem.Read(cKey,&cVal)||cVal!=wxS("my1sim85chk"))
	{
		wxMessageBox(wxString::Format("File: '%s'",aFilename),
			wxS("[SYSTEM LOAD ERROR]"),wxOK|wxICON_ERROR);
		return false;
	}
	this->PrintInfoMessage(wxS("\nRemoving current system..."));
	// rebuild system here!
	this->SystemDisconnect();
	this->PrintInfoMessage(wxS("\nLoading new system..."));
	// look for memory instances
	long cValue, cCount;
	cKey = wxS("/System/CountM");
	cCount = cSystem.ReadLong(cKey,0);
	this->PrintInfoMessage(wxString::Format("Memory Count: '%ld'",cCount));
	for(int cLoop=0;cLoop<cCount;cLoop++)
	{
		cSystem.SetPath(wxString::Format(wxS("/Memory%d"),cLoop));
		cKey = wxS("Info");
		cFlag &= cSystem.Read(cKey,&cVal);
		cVal = cVal.Mid(cVal.Find(wxS("Start:")));
		cVal = cVal.AfterFirst(':');
		cVal = cVal.BeforeFirst(';');
		cVal.ToLong(&cValue,16);
		cKey = wxS("Type");
		cFlag &= cSystem.Read(cKey,&cVal);
		if(cVal==wxS("RAM")) this->ConnectRAM(cValue);
		else this->ConnectROM(cValue);
		cSystem.SetPath(wxS("/")); // just in case
	}
	// look for device instances
	cKey = wxS("/System/CountD");
	cCount = cSystem.ReadLong(cKey,0);
	this->PrintInfoMessage(wxString::Format("Device Count: '%ld'",cCount));
	for(int cLoop=0;cLoop<cCount;cLoop++)
	{
		cSystem.SetPath(wxString::Format(wxS("/Device%d"),cLoop));
		cKey = wxS("Type");
		cFlag &= cSystem.Read(cKey,&cVal);
		cKey = wxS("Info");
		cFlag &= cSystem.Read(cKey,&cVal);
		cVal = cVal.Mid(cVal.Find(wxS("Start:")));
		cVal = cVal.AfterFirst(':');
		cVal = cVal.BeforeFirst(';');
		cVal.ToLong(&cValue,16);
		this->ConnectPPI(cValue);
		cSystem.SetPath(wxS("/")); // just in case
	}
	// look for control instances
	cKey = wxS("/System/CountC");
	cCount = cSystem.ReadLong(cKey,0);
	this->PrintInfoMessage(wxString::Format("PanelD Count: '%ld'",cCount));
	for(int cLoop=0;cLoop<cCount;cLoop++)
	{
		wxString cName; bool cTest;
		my1DEVPanel* pPanel = 0x0;
		cSystem.SetPath(wxString::Format(wxS("/Control%d"),cLoop));
		cKey = wxS("Type");
		cFlag &= cSystem.Read(cKey,&cVal);
		cKey = wxS("Name");
		cFlag &= cSystem.Read(cKey,&cName);
		cKey = wxS("Flag");
		cFlag &= cSystem.Read(cKey,&cTest);
		// create the panel
		if(cVal==wxS("7SEG"))
			pPanel = (my1DEVPanel*) this->CreateDevice7SegPanel(cName);
		else if(cVal==wxS("KPAD"))
			pPanel = (my1DEVPanel*) this->CreateDeviceKPadPanel(cName);
		else if(cVal==wxS("LED"))
			pPanel = (my1DEVPanel*) this->CreateDeviceLEDPanel(cName,cTest);
		else if(cVal==wxS("SWI"))
			pPanel = (my1DEVPanel*) this->CreateDeviceSWIPanel(cName);
		else if(cVal==wxS("BUT"))
			pPanel = (my1DEVPanel*) this->CreateDeviceBUTPanel(cName);
		if(!pPanel) continue; // very unlikely!
		// link the bits
		int cBitCount = 0;
		wxWindowList& cBitList = pPanel->GetChildren();
		wxWindowList::Node *pBitNode = cBitList.GetFirst();
		while(pBitNode)
		{
			wxWindow *pBitCheck = (wxWindow*) pBitNode->GetData();
			if(pBitCheck->IsKindOf(wxCLASSINFO(my1BITCtrl)))
			{
				my1BITCtrl *pCtrl = (my1BITCtrl*) pBitCheck;
				cKey = wxString::Format("Bit%d",cBitCount++);
				cFlag &= cSystem.Read(cKey,&cVal);
				wxString cChk = cVal;
				// start convert to num
				cFlag &= cChk.BeforeFirst(':').ToLong(&cValue);
				pCtrl->Link().mDevice = cValue;
				cChk = cChk.AfterFirst(':');
				cFlag &= cChk.BeforeFirst(':').ToLong(&cValue);
				pCtrl->Link().mDevicePort = cValue;
				cChk = cChk.AfterFirst(':');
				cFlag &= cChk.BeforeFirst(':').ToLong(&cValue);
				pCtrl->Link().mDeviceBit = cValue;
				cChk = cChk.AfterFirst(':');
				cFlag &= cChk.BeforeFirst(':').ToLong(&cValue);
				pCtrl->Link().mDeviceAddr = cValue;
				cChk = cChk.AfterFirst(':');
				// use default active level if not specified (empty)
				if(!cChk.IsEmpty())
				{
					long cActive;
					cFlag &= cChk.ToLong(&cActive);
					// active level!
					pCtrl->ActiveLevel(cActive);
				}
				// make the link!
				if(!pCtrl->IsDummy()&&(cValue>=0||pCtrl->Link().mDevice<0))
				{
					my1BitIO* pBit = this->GetDeviceBit(pCtrl->Link(),true);
					if (pBit) // should get the device!
					{
						pBit->Unlink(); // just in case?
						pCtrl->LinkThis((my1BitIO*)pCtrl->Link().mPointer);
					}
				}
			}
			pBitNode = pBitNode->GetNext();
		}
		cSystem.SetPath(wxS("/")); // just in case
	}
	// load saved layout
	cSystem.SetPath(wxS("/System"));
	{
		wxString cVal, cKey = wxS("Layout");
		cFlag &= cSystem.Read(cKey,&cVal);
		cVal.Replace(wxS(":"),wxS("="));
		cVal.Replace(wxS("__"),wxS(" "));
		mMainUI.LoadPerspective(cVal);
		// check procTool status?
		wxAuiPaneInfo& cPaneProc = mMainUI.GetPane(wxS("procTool"));
		if(cPaneProc.IsOk())
		{
			wxWindow *cTarget = mNoteBook->GetCurrentPage();
			if(cTarget&&cTarget->IsKindOf(wxCLASSINFO(my1CodeEdit)))
				cPaneProc.Show(true);
			else
				cPaneProc.Show(false);
			mMainUI.Update();
		}
		cSystem.SetPath(wxS("/"));
	}
	// DEBUG
	//this->PrintInfoMessage(wxString::Format("Layout: '%s'",cVal));
	//this->PrintInfoMessage(wxString::Format("Flag: '%s'",cFlag?"true":"false"));
	if(cFlag)
	{
		wxMessageBox(wxString::Format("System Loaded!"),
			wxS("[SUCCESS]"),wxOK|wxICON_INFORMATION);
		this->PrintInfoMessage(wxString::Format("System Ready.\n"));
	}
	return cFlag;
}

bool my1Form::SaveSystem(const wxString& aFilename)
{
	bool cFlag = true;
	int cLoop;
	wxFileName cName(aFilename);
	if(!cName.FileExists())
	{
		wxFileOutputStream cTest(aFilename);
		if(!cTest.IsOk())
		{
			wxMessageBox(wxString::Format("File: '%s'",aFilename),
				wxS("[CREATE ERROR]"),wxOK|wxICON_INFORMATION);
			return false;
		}
	}
	wxFileInputStream *pFile = new wxFileInputStream(aFilename);
	wxFileConfig cSystem(*pFile);
	// delete previous system, if applicable
	cSystem.DeleteAll();
	// throw in savefile id
	cSystem.SetPath(wxS("/System"));
	{
		wxString cKey = wxS("my1sim85key");
		wxString cVal = wxS("my1sim85chk");
		cFlag &= cSystem.Write(cKey,cVal);
		cSystem.SetPath(wxS("/"));
	}
	// save memory
	cLoop = 0;
	my1Memory *pMemory = m8085.Memory(0);
	while(pMemory)
	{
		wxString cKey, cVal;
		int cValue;
		wxString cPath = wxString::Format("/Memory%d",cLoop++);
		cSystem.SetPath(cPath);
		cKey = wxS("Type");
		if(pMemory->IsReadOnly()) cVal = wxS("ROM");
		else cVal = wxS("RAM");
		cFlag &= cSystem.Write(cKey,cVal);
		cKey = wxS("Info");
		cVal = wxS("Name:");
		if(pMemory->IsReadOnly()) cVal += wxS("2764;");
		else cVal += wxS("6264;");
		cValue = (int)pMemory->IsReadOnly();
		cVal += wxString::Format(wxS("ReadOnly:%d;"),cValue);
		cVal += wxString::Format(wxS("Start:%04X;"),pMemory->GetStart());
		cVal += wxString::Format(wxS("Size:%04X;"),pMemory->GetSize());
		cFlag &= cSystem.Write(cKey,cVal);
		cSystem.SetPath(wxS("/"));
		pMemory = (my1Memory*) pMemory->Next();
	}
	// save memory count
	cSystem.SetPath(wxS("/System"));
	{
		wxString cKey = wxS("CountM");
		long cVal = cLoop;
		cFlag &= cSystem.Write(cKey,cVal);
		cSystem.SetPath(wxS("/"));
	}
	// save device
	cLoop = 0;
	my1Device *pDevice = m8085.Device(0);
	while(pDevice)
	{
		wxString cKey, cVal;
		wxString cPath = wxString::Format("/Device%d",cLoop++);
		cSystem.SetPath(cPath);
		cKey = wxS("Type");
		cVal = wxS("PPI");
		cFlag &= cSystem.Write(cKey,cVal);
		cKey = wxS("Info");
		cVal = wxS("Name:8255;");
		cVal += wxString::Format(wxS("Start:%02X;"),pDevice->GetStart());
		cVal += wxString::Format(wxS("Size:%02X;"),pDevice->GetSize());
		cFlag &= cSystem.Write(cKey,cVal);
		cSystem.SetPath(wxS("/"));
		pDevice = (my1Device*) pDevice->Next();
	}
	// save memory count
	cSystem.SetPath(wxS("/System"));
	{
		wxString cKey = wxS("CountD");
		long cVal = cLoop;
		cFlag &= cSystem.Write(cKey,cVal);
		cSystem.SetPath(wxS("/"));
	}
	// save all controls??
	cLoop = 0;
	wxWindowList& cList = this->GetChildren();
	wxWindowList::Node *pNode = cList.GetFirst();
	while(pNode)
	{
		wxWindow *pTarget = (wxWindow*) pNode->GetData();
		if(pTarget->IsKindOf(wxCLASSINFO(my1DEVPanel)))
		{
			wxString cKey, cVal;
			wxString cPath = wxString::Format("/Control%d",cLoop++);
			cSystem.SetPath(cPath);
			wxAuiPaneInfo& cPane = mMainUI.GetPane(pTarget);
			if(!cPane.IsOk()) continue;
			cVal = mMainUI.SavePaneInfo(cPane);
			wxString cCheck = cVal.Mid(cVal.First(wxS("name=")));
			cCheck = cCheck.BeforeFirst(';');
			cCheck = cCheck.AfterFirst('=');
			cFlag &= cSystem.Write(wxS("Name"),cCheck);
			// get in form devXXX[X]YY
			cCheck = cCheck.Mid(3,cCheck.Length()-5);
			cFlag &= cSystem.Write(wxS("Type"),cCheck);
			// write flag
			my1DEVPanel* pPanel = (my1DEVPanel*) pTarget;
			cFlag &= cSystem.Write(wxS("Flag"),pPanel->Flag());
			// save bit information!
			int cBitCount = 0;
			wxWindowList& cBitList = pTarget->GetChildren();
			wxWindowList::Node *pBitNode = cBitList.GetFirst();
			while(pBitNode)
			{
				wxWindow *pBitCheck = (wxWindow*) pBitNode->GetData();
				if(pBitCheck->IsKindOf(wxCLASSINFO(my1BITCtrl)))
				{
					my1BITCtrl *pCtrl = (my1BITCtrl*) pBitCheck;
					cKey = wxString::Format("Bit%d",cBitCount++);
					cVal = wxString::Format("%d:%d:%d:%d:%d",
						pCtrl->Link().mDevice, pCtrl->Link().mDevicePort,
						pCtrl->Link().mDeviceBit, pCtrl->Link().mDeviceAddr,
						pCtrl->ActiveLevel());
					cFlag &= cSystem.Write(cKey,cVal);
				}
				pBitNode = pBitNode->GetNext();
			}
			cSystem.SetPath(wxS("/"));
		}
		pNode = pNode->GetNext();
	}
	// save control count
	cSystem.SetPath(wxS("/System"));
	{
		wxString cKey = wxS("CountC");
		long cVal = cLoop;
		cFlag &= cSystem.Write(cKey,cVal);
		cSystem.SetPath(wxS("/"));
	}
	// save layout
	cSystem.SetPath(wxS("/System"));
	{
		wxString cKey = wxS("Layout");
		wxString cVal = mMainUI.SavePerspective();
		cVal.Replace(wxS("="),wxS(":"));
		cVal.Replace(wxS(" "),wxS("__"));
		cFlag &= cSystem.Write(cKey,cVal);
		cSystem.SetPath(wxS("/"));
	}
	delete pFile;
	pFile = 0x0;
	// only if no errors
	if(cFlag)
	{
		wxFileOutputStream cFile(aFilename);
		cSystem.Save(cFile);
	}
	return cFlag;
}

void my1Form::SimUpdateREG(void* simObject)
{
	// update register view
	my1Reg85 *pReg85 = (my1Reg85*) simObject;
	wxString cFormat = "%02X";
	if(pReg85->IsReg16()) cFormat = "%04X";
	cFormat = wxString::Format(cFormat,pReg85->GetData());
	my1Panel *pText = (my1Panel*) pReg85->GetLink();
	pText->SetText(cFormat);
	if(pReg85->GetID()==I8085_REG_F)
	{
		my1Form* pForm = (my1Form*) pText->GetGrandParent();
		pForm->SimUpdateFLAG(pReg85);
	}
}

void my1Form::SimUpdateMEM(void* simObject)
{
	// update memory view
	my1Memory *pMemory = (my1Memory*) simObject;
	wxGrid *pGrid = (wxGrid*) pMemory->GetLink();
	int cGridWidth = pGrid->GetNumberCols();
	int cAddress = pMemory->GetLastUsed();
	int cCol = cAddress%cGridWidth;
	int cRow = cAddress/cGridWidth;
	int cData = pMemory->GetLastData();
	pGrid->SetCellValue(cRow,cCol,wxString::Format(wxS("%02X"),cData));
	// find mini viewers
	wxWindow* pParent = pGrid->GetGrandParent(); // get RegsPanel
	my1Form* pForm =  (my1Form*) pParent->GetParent(); // get the form!
	my1MiniViewer* pViewer = pForm->mFirstViewer;
	while(pViewer)
	{
		if(pViewer->IsSelected(cAddress))
		{
			int cIndex = cAddress - pViewer->mStart;
			cGridWidth = pViewer->pGrid->GetNumberCols();
			cCol = cIndex%cGridWidth;
			cRow = cIndex/cGridWidth;
			pViewer->pGrid->SetCellValue(cRow,cCol,
				wxString::Format(wxS("%02X"),cData));
		}
		pViewer = pViewer->mNext;
	}
}

void my1Form::SimDoUpdate(void* simObject)
{
	// microprocessor level update?
	// only useful if low-level sim (state machine?)
}

void my1Form::SimDoDelay(void* simObject, int aCount)
{
	//my1Sim85* mySim = (my1Sim85*) simObject;
	//my1Form* myForm = (my1Form*) mySim->GetLink();
	//wxMicroSleep(aCount);
}
