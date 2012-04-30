/**
*
* wxform.cpp
*
* - implementation for main wx-based form
*
**/

#include "wxmain.hpp"
#include "wxform.hpp"
#include "wxcode.hpp"
#include "wxled.hpp"
#include "wxswitch.hpp"

#include "wx/gbsizer.h"
#include "wx/aboutdlg.h"

#include <ctime>

#define MACRO_WXBMP(bmp) wxBitmap(bmp##_xpm)
#define MACRO_WXICO(bmp) wxIcon(bmp##_xpm)

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

// handy alias
#define WX_CEH wxCommandEventHandler
#define WX_KEH wxKeyEventHandler
#define WX_MEH wxMouseEventHandler
#define WX_TEH wxTimerEventHandler

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define REGS_PANEL_WIDTH 200
#define CONS_PANEL_HEIGHT 150
#define INFO_REG_SPACER 5
#define INFO_DEV_SPACER 5
#define SEG7_NUM_SPACER 5
#define DEVC_POP_SPACER 5
#define STATUS_COUNT 2
#define STATUS_FIX_WIDTH REGS_PANEL_WIDTH
#define STATUS_MSG_INDEX 1
#define STATUS_MSG_PERIOD 3000
#define SIM_START_ADDR 0x0000
#define SIM_EXEC_PERIOD 1
#define TOOL_FILE_POS 0
#define TOOL_EDIT_POS 1
#define TOOL_PROC_POS 2
#define TITLE_FONT_SIZE 24
#define EMAIL_FONT_SIZE 8
#define PANEL_FONT_SIZE 10
#define INFO_FONT_SIZE 8
#define LOGS_FONT_SIZE 8
#define SIMS_FONT_SIZE 8
#define GRID_FONT_SIZE 8
#define CONS_FONT_SIZE 8
#define FLOAT_INIT_X 40
#define FLOAT_INIT_Y 40
#define MEM_VIEW_WIDTH 16
#define MEM_VIEW_HEIGHT (MAX_MEMSIZE/MEM_VIEW_WIDTH)
#define MEM_MINIVIEW_WIDTH 8
#define MEM_MINIVIEW_HEIGHT 4
#define DOT_SIZE 9
#define DEFSIZE_7SEG 1
#define AUI_EXTER_LAYER 3
#define AUI_OUTER_LAYER 2
#define AUI_INNER_LAYER 1

my1Form::my1Form(const wxString &title)
	: wxFrame( NULL, MY1ID_MAIN, title, wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_FRAME_STYLE)
{
	mBuildMode = false;
	// simulation stuffs
	mSimulationMode = false;
	mSimulationRunning = false;
	mSimulationStepping = false;
	this->CalculateSimCycle();
	// default option?
	mOptions.mChanged = false;
	mOptions.mEdit_ViewWS = false;
	mOptions.mEdit_ViewEOL = false;
	mOptions.mConv_UnixEOL = false;
	mOptions.mSims_FreeRunning = false;
	mOptions.mSims_ShowRunInfo = false;
	mOptions.mSims_PauseOnINTR = false;
	mOptions.mSims_StartADDR = SIM_START_ADDR;
	// assign function pointers :p
	m8085.SetLink((void*)this);
	m8085.DoUpdate = &this->SimDoUpdate;
	m8085.DoDelay = &this->SimDoDelay;
	// reset mini-viewers (dual-link-list?)
	mFirstViewer = 0x0;
	// minimum window size... duh!
	this->SetMinSize(wxSize(WIN_WIDTH,WIN_HEIGHT));
	// status bar
	this->CreateStatusBar(STATUS_COUNT);
	this->SetStatusText(wxT("Welcome to my1sim85!"));
	const int cWidths[STATUS_COUNT] = { STATUS_FIX_WIDTH, -1 };
	wxStatusBar* cStatusBar = this->GetStatusBar();
	cStatusBar->SetStatusWidths(STATUS_COUNT,cWidths);
	mDisplayTimer = new wxTimer(this, MY1ID_STAT_TIMER);
	mSimExecTimer = new wxTimer(this, MY1ID_SIMX_TIMER);
	// console command history
	mCmdHistory.Clear();
	mCmdHistory.Alloc(CMD_HISTORY_COUNT+1);
	mCmdHistIndex = 0;
	// some handy pointers
	mConsole = 0x0;
	mCommand = 0x0;
	mDevicePopupMenu = 0x0;
	mDevicePortMenu = 0x0;
	mPortPanel = 0x0;
	mLEDPanel = 0x0;
	mSWIPanel = 0x0;
	// setup image
	//wxInitAllImageHandlers();
	wxIcon mIconApps = MACRO_WXICO(apps);
	this->SetIcon(mIconApps);
	// menu bar
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(MY1ID_LOAD, wxT("&Open\tF2"));
	fileMenu->Append(MY1ID_SAVE, wxT("&Save\tF3"));
	fileMenu->Append(MY1ID_SAVEAS, wxT("Save &As..."));
	fileMenu->Append(MY1ID_NEW, wxT("&New\tF4"));
	fileMenu->AppendSeparator();
	fileMenu->Append(MY1ID_EXIT, wxT("E&xit"), wxT("Quit program"));
	wxMenu *editMenu = new wxMenu;
	editMenu->Append(MY1ID_BUILDINIT, wxT("System &Build...\tF5"));
	editMenu->Append(MY1ID_OPTIONS, wxT("&Preferences...\tF8"));
	wxMenu *viewMenu = new wxMenu;
	viewMenu->Append(MY1ID_VIEW_REGSPANE, wxT("View Register Panel"));
	viewMenu->Append(MY1ID_VIEW_INTRPANE, wxT("View Interrupt Panel"));
	viewMenu->Append(MY1ID_VIEW_CONSPANE, wxT("View Console/Info Panel"));
	viewMenu->Append(MY1ID_CREATE_MINIMV, wxT("Create miniMV Panel"));
	viewMenu->Append(MY1ID_CREATE_DV7SEG, wxT("Create dv7SEG Panel"));
	viewMenu->Append(MY1ID_CREATE_DEVLED, wxT("Create devLED Panel"));
	viewMenu->Append(MY1ID_CREATE_DEVSWI, wxT("Create devSWI Panel"));
	wxMenu *procMenu = new wxMenu;
	procMenu->Append(MY1ID_ASSEMBLE, wxT("&Assemble\tF5"));
	procMenu->Append(MY1ID_SIMULATE, wxT("&Simulate\tF6"));
	procMenu->Append(MY1ID_GENERATE, wxT("&Generate\tF7"));
	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(MY1ID_ABOUT, wxT("&About"), wxT("About This Program"));
	wxMenuBar *mainMenu = new wxMenuBar;
	mainMenu->Append(fileMenu, wxT("&File"));
	mainMenu->Append(editMenu, wxT("&Edit"));
	mainMenu->Append(viewMenu, wxT("&View"));
	mainMenu->Append(procMenu, wxT("&Tool"));
	mainMenu->Append(helpMenu, wxT("&Help"));
	this->SetMenuBar(mainMenu);
	mainMenu->EnableTop(mainMenu->FindMenu(wxT("Tool")),false);
	// using AUI manager...
	mMainUI.SetManagedWindow(this);
	// create notebook for main/editor panel
	mNoteBook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE);
	mNoteBook->AddPage(CreateMainPanel(mNoteBook), wxT("Welcome"), true);
	// create initial pane for main view
	mMainUI.AddPane(mNoteBook, wxAuiPaneInfo().Name(wxT("codeBook")).
		CenterPane().MaximizeButton(true).PaneBorder(false));
	// tool bar - file
	mMainUI.AddPane(CreateFileToolBar(), wxAuiPaneInfo().
		Name(wxT("fileTool")).Caption(wxT("File")).Floatable(false).
		ToolbarPane().Top().Position(TOOL_FILE_POS).BottomDockable(false));
	// tool bar - edit
	mMainUI.AddPane(CreateEditToolBar(), wxAuiPaneInfo().
		Name(wxT("editTool")).Caption(wxT("Edit")).Floatable(false).
		ToolbarPane().Top().Position(TOOL_EDIT_POS).BottomDockable(false));
	// tool bar - proc
	mMainUI.AddPane(CreateProcToolBar(), wxAuiPaneInfo().
		Name(wxT("procTool")).Caption(wxT("Process")).
		ToolbarPane().Top().Position(TOOL_PROC_POS).BottomDockable(false));
	// reg panel
	mMainUI.AddPane(CreateRegsPanel(), wxAuiPaneInfo().
		Name(wxT("regsPanel")).Caption(wxT("Registers")).
		DefaultPane().Left().Layer(AUI_EXTER_LAYER).
		Dockable(false).LeftDockable(true).RightDockable(true).
		MinSize(wxSize(REGS_PANEL_WIDTH,0)));
	// interrupt panel
	mMainUI.AddPane(CreateInterruptPanel(), wxAuiPaneInfo().
		Name(wxT("intrPanel")).Caption(wxT("Interrupts")).
		DefaultPane().Top().
		Dockable(false).TopDockable(true).BottomDockable(true));
	// simulation panel
	mMainUI.AddPane(CreateSimsPanel(), wxAuiPaneInfo().
		Name(wxT("simsPanel")).Caption(wxT("Simulation")).
		DefaultPane().Right().Dockable(false).RightDockable(true).
		Layer(AUI_INNER_LAYER).CloseButton(false).Hide());
	// system build panel
	mMainUI.AddPane(CreateBuildPanel(), wxAuiPaneInfo().
		Name(wxT("buildPanel")).Caption(wxT("System Build")).
		DefaultPane().Right().Dockable(false).RightDockable(true).
		Layer(AUI_INNER_LAYER).CloseButton(false).Hide());
	// log panel
	mMainUI.AddPane(CreateConsPanel(), wxAuiPaneInfo().
		Name(wxT("consPanel")).Caption(wxT("Console/Info Panel")).
		DefaultPane().Bottom().Layer(AUI_OUTER_LAYER).
		Dockable(false).BottomDockable(true).
		MaximizeButton(true).MinSize(wxSize(0,CONS_PANEL_HEIGHT)));
	// commit changes!
	mMainUI.Update();
	// actions & events! - (int, wxEventType, wxObjectEventFunction)
	wxEventType cEventType = wxEVT_COMMAND_TOOL_CLICKED;
	this->Connect(MY1ID_EXIT,cEventType,WX_CEH(my1Form::OnQuit));
	this->Connect(MY1ID_LOAD,cEventType,WX_CEH(my1Form::OnLoad));
	this->Connect(MY1ID_SAVE,cEventType,WX_CEH(my1Form::OnSave));
	this->Connect(MY1ID_SAVEAS,cEventType,WX_CEH(my1Form::OnSave));
	this->Connect(MY1ID_NEW,cEventType,WX_CEH(my1Form::OnNew));
	this->Connect(MY1ID_ABOUT,cEventType,WX_CEH(my1Form::OnAbout));
	this->Connect(MY1ID_VIEW_REGSPANE,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_VIEW_INTRPANE,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_VIEW_CONSPANE,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_OPTIONS,cEventType,WX_CEH(my1Form::OnCheckOptions));
	this->Connect(MY1ID_ASSEMBLE,cEventType,WX_CEH(my1Form::OnAssemble));
	this->Connect(MY1ID_SIMULATE,cEventType,WX_CEH(my1Form::OnSimulate));
	this->Connect(MY1ID_GENERATE,cEventType,WX_CEH(my1Form::OnGenerate));
	this->Connect(MY1ID_BUILDINIT,cEventType,WX_CEH(my1Form::OnBuildSelect));
	this->Connect(MY1ID_CREATE_MINIMV,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_CREATE_DV7SEG,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_CREATE_DEVLED,cEventType,WX_CEH(my1Form::OnShowPanel));
	this->Connect(MY1ID_CREATE_DEVSWI,cEventType,WX_CEH(my1Form::OnShowPanel));
	cEventType = wxEVT_COMMAND_BUTTON_CLICKED;
	this->Connect(MY1ID_CONSEXEC,cEventType,WX_CEH(my1Form::OnExecuteConsole));
	this->Connect(MY1ID_SIMSEXEC,cEventType,WX_CEH(my1Form::OnSimulationPick));
	this->Connect(MY1ID_SIMSSTEP,cEventType,WX_CEH(my1Form::OnSimulationPick));
	this->Connect(MY1ID_SIMSINFO,cEventType,WX_CEH(my1Form::OnSimulationInfo));
	this->Connect(MY1ID_SIMSPREV,cEventType,WX_CEH(my1Form::OnSimulationInfo));
	this->Connect(MY1ID_SIMRESET,cEventType,WX_CEH(my1Form::OnSimulationInfo));
	this->Connect(MY1ID_SIMSMIMV,cEventType,WX_CEH(my1Form::OnSimulationInfo));
	this->Connect(MY1ID_SIMSBRKP,cEventType,WX_CEH(my1Form::OnSimulationInfo));
	this->Connect(MY1ID_SIMSEXIT,cEventType,WX_CEH(my1Form::OnSimulationExit));
	this->Connect(MY1ID_BUILDINIT,cEventType,WX_CEH(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDRST,cEventType,WX_CEH(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDDEF,cEventType,WX_CEH(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDNFO,cEventType,WX_CEH(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDROM,cEventType,WX_CEH(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDRAM,cEventType,WX_CEH(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDPPI,cEventType,WX_CEH(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDOUT,cEventType,WX_CEH(my1Form::OnBuildSelect));
	cEventType = wxEVT_TIMER;
	this->Connect(MY1ID_STAT_TIMER,cEventType,WX_TEH(my1Form::OnStatusTimer));
	this->Connect(MY1ID_SIMX_TIMER,cEventType,WX_TEH(my1Form::OnSimExeTimer));
	// AUI-related events
	this->Connect(wxID_ANY,wxEVT_AUI_PANE_CLOSE,
		wxAuiManagerEventHandler(my1Form::OnClosePane));
	this->Connect(wxID_ANY,wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING,
		wxAuiNotebookEventHandler(my1Form::OnPageChanging));
	this->Connect(wxID_ANY,wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED,
		wxAuiNotebookEventHandler(my1Form::OnPageChanged));
	this->Connect(wxID_ANY,wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE,
		wxAuiNotebookEventHandler(my1Form::OnPageClosing));
	// position this!
	this->Maximize(); //this->Centre();
	// try to redirect standard console to gui console
	mRedirector = 0x0;
	if(mConsole) mRedirector = new wxStreamToTextRedirector(mConsole);
	// build default system
	this->SystemDefault();
	// let console command get focus by dfault
	if(mCommand) mCommand->SetFocus();
}

my1Form::~my1Form()
{
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

timespec timespec_diff(timespec beg, timespec end)
{
	timespec temp;
	if((end.tv_nsec-beg.tv_nsec)<0)
	{
		temp.tv_sec = end.tv_sec-beg.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-beg.tv_nsec;
	}
	else
	{
		temp.tv_sec = end.tv_sec-beg.tv_sec;
		temp.tv_nsec = end.tv_nsec-beg.tv_nsec;
	}
	return temp;
}

void my1Form::CalculateSimCycle(void)
{
	timespec cTime1, cTime2, cTimeD;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&cTime1);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&cTime2);
	cTimeD = timespec_diff(cTime1,cTime2);
	if(cTimeD.tv_sec) std::cout << "Large Delay! (" << cTimeD.tv_sec << ")\n";
	mSimulationCycleDefault = cTimeD.tv_nsec; // in nanoseconds!
	mSimulationCycle = mSimulationCycleDefault;
	mSimulationDelay = 1000; // default 1 microsec delay? best is ~500ns?
}

bool my1Form::ScaleSimCycle(double aScale)
{
	bool cScaled = false;
	double cTest = mSimulationCycle*aScale;
	if(cTest>=mSimulationCycleDefault)
	{
		mSimulationCycle = cTest;
		mSimulationDelay = (unsigned long) mSimulationCycle;
		if(!mSimulationDelay) mSimulationDelay = 1;
		cScaled = true;
	}
	return cScaled;
}

double my1Form::GetSimCycle(void)
{
	return mSimulationCycle;
}

unsigned long my1Form::GetSimDelay(void)
{
	return mSimulationDelay;
}

void my1Form::SimulationMode(bool aGo)
{
	wxMenuBar *cMainMenu = this->GetMenuBar();
	wxAuiToolBar *cFileTool = (wxAuiToolBar*) this->FindWindow(MY1ID_FILETOOL);
	wxAuiToolBar *cEditTool = (wxAuiToolBar*) this->FindWindow(MY1ID_EDITTOOL);
	wxAuiToolBar *cProcTool = (wxAuiToolBar*) this->FindWindow(MY1ID_PROCTOOL);
	cMainMenu->Enable(!aGo);
	cFileTool->Enable(!aGo);
	cEditTool->Enable(!aGo);
	cProcTool->Enable(!aGo);
	wxString cToolName = wxT("simsPanel");
	wxAuiPaneInfo& cPane = mMainUI.GetPane(cToolName);
	if(aGo)
	{
		wxPoint cPoint = this->GetScreenPosition();
		cPane.FloatingPosition(cPoint.x+FLOAT_INIT_X,cPoint.y+FLOAT_INIT_Y);
	}
	cPane.Show(aGo);
	mSimulationMode = aGo;
	mMainUI.Update();
}

void my1Form::BuildMode(bool aGo)
{
	wxMenuBar *cMainMenu = this->GetMenuBar();
	wxAuiToolBar *cFileTool = (wxAuiToolBar*) this->FindWindow(MY1ID_FILETOOL);
	wxAuiToolBar *cEditTool = (wxAuiToolBar*) this->FindWindow(MY1ID_EDITTOOL);
	wxButton *cButtonBuild = (wxButton*) this->FindWindow(MY1ID_BUILDINIT);
	cMainMenu->Enable(!aGo);
	cFileTool->Enable(!aGo);
	cEditTool->Enable(!aGo);
	cButtonBuild->Enable(!aGo);
	mCommand->Enable(!aGo);
	wxString cToolName = wxT("buildPanel");
	wxAuiPaneInfo& cPane = mMainUI.GetPane(cToolName);
	if(aGo)
	{
		mNoteBook->SetSelection(0);
		wxPoint cPoint = this->GetScreenPosition();
		cPane.FloatingPosition(cPoint.x+FLOAT_INIT_X,cPoint.y+FLOAT_INIT_Y);
	}
	cPane.Show(aGo);
	mBuildMode = aGo;
	mMainUI.Update();
}

bool my1Form::GetUniqueName(wxString& aName)
{
	wxString cName;
	int cIndex = 0;
	while(cIndex<0x100) // 256 max
	{
		cName = aName + wxString::Format(wxT("%02X"),cIndex++);
		wxAuiPaneInfo& rPane = mMainUI.GetPane(cName);
		if(!rPane.IsOk())
		{
			aName = cName;
			return true;
		}
	}
	return false;
}

my1Device* my1Form::Device(int anIndex)
{
	return m8085.Device(anIndex);
}

wxPanel* my1Form::PortPanel(void)
{
	return mPortPanel;
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
	wxAuiToolBar* fileTool = new wxAuiToolBar(this, MY1ID_FILETOOL,
		wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	fileTool->SetToolBitmapSize(wxSize(16,16));
	fileTool->AddTool(MY1ID_EXIT, wxT("Exit"), mIconExit, wxT("Exit"));
	fileTool->AddSeparator();
	fileTool->AddTool(MY1ID_NEW, wxT("Clear"), mIconNewd, wxT("New"));
	fileTool->AddTool(MY1ID_LOAD, wxT("Open"), mIconLoad, wxT("Open"));
	fileTool->AddTool(MY1ID_SAVE, wxT("Save"), mIconSave, wxT("Save"));
	fileTool->Realize();
	return fileTool;
}

wxAuiToolBar* my1Form::CreateEditToolBar(void)
{
	wxBitmap mIconBuild = MACRO_WXBMP(build);
	wxBitmap mIconOptions = MACRO_WXBMP(option);
	wxBitmap mIconMiniMV = MACRO_WXBMP(target);
	wxAuiToolBar* editTool = new wxAuiToolBar(this, MY1ID_EDITTOOL,
		wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	editTool->SetToolBitmapSize(wxSize(16,16));
	editTool->AddTool(MY1ID_BUILDINIT, wxT("BuildSys"),
		mIconBuild, wxT("Build System"));
	editTool->AddTool(MY1ID_CREATE_MINIMV, wxT("MiniMV"),
		mIconMiniMV, wxT("Create Mini MemViewer"));
	editTool->AddTool(MY1ID_OPTIONS, wxT("Options"),
		mIconOptions, wxT("Options"));
	editTool->Realize();
	return editTool;
}

wxAuiToolBar* my1Form::CreateProcToolBar(void)
{
	wxBitmap mIconAssemble = MACRO_WXBMP(binary);
	wxBitmap mIconSimulate = MACRO_WXBMP(simx);
	wxBitmap mIconGenerate = MACRO_WXBMP(hexgen);
	wxAuiToolBar* procTool = new wxAuiToolBar(this, MY1ID_PROCTOOL,
		wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	procTool->SetToolBitmapSize(wxSize(16,16));
	procTool->AddTool(MY1ID_ASSEMBLE, wxT("Assemble"),
		mIconAssemble, wxT("Assemble"));
	procTool->AddTool(MY1ID_SIMULATE, wxT("Simulate"),
		mIconSimulate, wxT("Simulate"));
	procTool->AddTool(MY1ID_GENERATE, wxT("Generate"),
		mIconGenerate, wxT("Generate"));
	procTool->Realize();
	procTool->Enable(false); // disabled by default!
	return procTool;
}

wxBoxSizer* my1Form::CreateFLAGView(wxWindow* aParent,
	const wxString& aString, int anID)
{
	wxString cDefault = wxT("0");
	wxStaticText *cLabel = new wxStaticText(aParent, wxID_ANY, aString);
	wxTextCtrl *cValue = new wxTextCtrl(aParent, wxID_ANY, cDefault,
		wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
	this->FlagLink(anID).SetLink((void*)cValue);
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->Add(cLabel,1,wxALIGN_CENTER);
	cBoxSizer->Add(cValue,0,wxALIGN_RIGHT);
	return cBoxSizer;
}

wxBoxSizer* my1Form::CreateREGSView(wxWindow* aParent,
	const wxString& aString, int anID)
{
	wxString cDefault = wxT("00");
	my1Reg85 *pReg85 = m8085.Register(anID);
	if(pReg85->IsReg16()) cDefault += wxT("00");
	wxStaticText *cLabel = new wxStaticText(aParent, wxID_ANY, aString);
	wxTextCtrl *cValue = new wxTextCtrl(aParent, wxID_ANY, cDefault,
		wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
	pReg85->SetLink((void*)cValue);
	pReg85->DoUpdate = &this->SimUpdateREG;
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->Add(cLabel,1,wxALIGN_CENTER);
	cBoxSizer->Add(cValue,0,wxALIGN_RIGHT);
	return cBoxSizer;
}

wxPanel* my1Form::CreateMainPanel(wxWindow *parent)
{
	wxPanel *cPanel = new wxPanel(parent);
	wxFont cFont(PANEL_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxStaticText *cLabel = new wxStaticText(cPanel, wxID_ANY, wxT("MY1 Sim85"));
	wxFont tFont(TITLE_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cLabel->SetFont(tFont);
	wxButton *cButtonBuild = new wxButton(cPanel, MY1ID_BUILDINIT,
		wxT("BUILD SYSTEM"), wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->Add(cLabel,1,wxALIGN_CENTER|wxALIGN_BOTTOM);
	wxBoxSizer *eBoxSizer = new wxBoxSizer(wxVERTICAL);
	eBoxSizer->Add(cBoxSizer,1,wxALIGN_CENTRE);
	eBoxSizer->Add(cButtonBuild,0,wxALIGN_CENTRE|wxALIGN_TOP);
	eBoxSizer->AddStretchSpacer();
	wxStaticText *dLabel = new wxStaticText(cPanel, wxID_ANY,
		wxT("by azman@my1matrix.net"));
	wxFont eFont(EMAIL_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	dLabel->SetFont(eFont);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	pBoxSizer->Add(eBoxSizer,1,wxALIGN_CENTRE);
	pBoxSizer->Add(dLabel,0,wxALIGN_BOTTOM|wxALIGN_RIGHT);
	cPanel->SetSizerAndFit(pBoxSizer);
	return cPanel;
}

wxPanel* my1Form::CreateRegsPanel(void)
{
	int cRegID;
	wxString cRegNAME;
	wxPanel *cPanel = new wxPanel(this);
	wxFont cFont(PANEL_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	cRegNAME = wxT("Register B"); cRegID = I8085_REG_B;
	pBoxSizer->Add(CreateREGSView(cPanel,cRegNAME,cRegID),0,wxEXPAND);
	cRegNAME = wxT("Register C"); cRegID = I8085_REG_C;
	pBoxSizer->Add(CreateREGSView(cPanel,cRegNAME,cRegID),0,wxEXPAND);
	cRegNAME = wxT("Register D"); cRegID = I8085_REG_D;
	pBoxSizer->Add(CreateREGSView(cPanel,cRegNAME,cRegID),0,wxEXPAND);
	cRegNAME = wxT("Register E"); cRegID = I8085_REG_E;
	pBoxSizer->Add(CreateREGSView(cPanel,cRegNAME,cRegID),0,wxEXPAND);
	cRegNAME = wxT("Register H"); cRegID = I8085_REG_H;
	pBoxSizer->Add(CreateREGSView(cPanel,cRegNAME,cRegID),0,wxEXPAND);
	cRegNAME = wxT("Register L"); cRegID = I8085_REG_L;
	pBoxSizer->Add(CreateREGSView(cPanel,cRegNAME,cRegID),0,wxEXPAND);
	cRegNAME = wxT("Register A"); cRegID = I8085_REG_A;
	pBoxSizer->Add(CreateREGSView(cPanel,cRegNAME,cRegID),0,wxEXPAND);
	cRegNAME = wxT("Register F"); cRegID = I8085_REG_F;
	pBoxSizer->Add(CreateREGSView(cPanel,cRegNAME,cRegID),0,wxEXPAND);
	cRegNAME = wxT("Program Counter"); cRegID = I8085_RP_PC+I8085_REG_COUNT;
	pBoxSizer->Add(CreateREGSView(cPanel,cRegNAME,cRegID),0,wxEXPAND);
	cRegNAME = wxT("Stack Pointer"); cRegID = I8085_RP_SP+I8085_REG_COUNT;
	pBoxSizer->Add(CreateREGSView(cPanel,cRegNAME,cRegID),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_REG_SPACER);
	cRegID = I8085_FLAG_C; cRegNAME = wxT("CY Flag");
	pBoxSizer->Add(CreateFLAGView(cPanel,cRegNAME,cRegID),0,wxEXPAND);
	cRegID = I8085_FLAG_P; cRegNAME = wxT("Parity Flag");
	pBoxSizer->Add(CreateFLAGView(cPanel,cRegNAME,cRegID),0,wxEXPAND);
	cRegID = I8085_FLAG_A; cRegNAME = wxT("AC Flag");
	pBoxSizer->Add(CreateFLAGView(cPanel,cRegNAME,cRegID),0,wxEXPAND);
	cRegID = I8085_FLAG_Z; cRegNAME = wxT("Zero Flag");
	pBoxSizer->Add(CreateFLAGView(cPanel,cRegNAME,cRegID),0,wxEXPAND);
	cRegID = I8085_FLAG_S; cRegNAME = wxT("Sign Flag");
	pBoxSizer->Add(CreateFLAGView(cPanel,cRegNAME,cRegID),0,wxEXPAND);
	cPanel->SetSizerAndFit(pBoxSizer);
	return cPanel;
}

wxPanel* my1Form::CreateInterruptPanel(void)
{
	wxPanel *cPanel = new wxPanel(this);
	wxFont cFont(SIMS_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	int cInterruptID = I8085_PIN_TRAP;
	for(int cLoop=0;cLoop<I8085_PIN_COUNT;cLoop++)
	{
		my1SWICtrl* pCtrl = new my1SWICtrl(cPanel,wxID_ANY);
		wxString cLabel, cType;
		switch(cInterruptID)
		{
			case I8085_PIN_TRAP:
				cType = wxT("Trap");
				cLabel = wxString::Format(wxT("TRAP [0x%04X]"),I8085_ISR_TRP);
				break;
			case I8085_PIN_I7P5:
				cType = wxT("I7.5");
				cLabel = wxString::Format(wxT("I7.5 [0x%04X]"),I8085_ISR_7P5);
				break;
			case I8085_PIN_I6P5:
				cType = wxT("I6.5");
				cLabel = wxString::Format(wxT("I6.5 [0x%04X]"),I8085_ISR_6P5);
				break;
			case I8085_PIN_I5P5:
				cType = wxT("I5.5");
				cLabel = wxString::Format(wxT("I5.5 [0x%04X]"),I8085_ISR_5P5);
				break;
		}
		pCtrl->SetLabel(cLabel);
		// get interrupt index & link, anID should be >=0 && <I8085_PIN_COUNT
		my1BitIO& rBitIO = m8085.Pin(cInterruptID);
		my1BitSelect cLink(cInterruptID,(void*) &rBitIO);
		pCtrl->LinkCheck(cLink);
		pBoxSizer->Add((wxWindow*)pCtrl,0,wxALIGN_CENTER);
		wxStaticText *cText = new wxStaticText(cPanel,wxID_ANY,cType);
		pBoxSizer->Add((wxWindow*)cText,0,wxALIGN_CENTER);
		cInterruptID++;
	}
	cPanel->SetSizerAndFit(pBoxSizer);
	return cPanel;
}

wxPanel* my1Form::CreateConsPanel(void)
{
	wxPanel *cPanel = new wxPanel(this);
	wxFont cTestFont(LOGS_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cTestFont);
	// duh?!
	cPanel->SetMinSize(wxSize(REGS_PANEL_WIDTH,0));
	// main view - logbook
	wxNotebook *cLogBook = new wxNotebook(cPanel, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxNB_LEFT);
	// add the pages
	cLogBook->AddPage(CreateConsolePanel(cLogBook),wxT("Console"),true);
	cLogBook->AddPage(CreateMemoryPanel(cLogBook),wxT("Memory"),true);
	cLogBook->SetSelection(0);
	// main box-sizer
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	pBoxSizer->Add(cLogBook, 1,
		wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
	cPanel->SetSizerAndFit(pBoxSizer);
	// return wxpanel object
	return cPanel;
}

wxPanel* my1Form::CreateSimsPanel(void)
{
	wxPanel *cPanel = new wxPanel(this);
	wxFont cFont(SIMS_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxButton *cButtonStep = new wxButton(cPanel, MY1ID_SIMSSTEP, wxT("Step"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonExec = new wxButton(cPanel, MY1ID_SIMSEXEC, wxT("Run"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonRset = new wxButton(cPanel, MY1ID_SIMRESET, wxT("Reset"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonBRKP = new wxButton(cPanel, MY1ID_SIMSBRKP, wxT("Break"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonInfo = new wxButton(cPanel, MY1ID_SIMSINFO, wxT("Info"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonPrev = new wxButton(cPanel, MY1ID_SIMSPREV, wxT("Prev"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonMini = new wxButton(cPanel, MY1ID_SIMSMIMV, wxT("miniMV"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonExit = new wxButton(cPanel, MY1ID_SIMSEXIT, wxT("Exit"),
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

wxPanel* my1Form::CreateBuildPanel(void)
{
	wxPanel *cPanel = new wxPanel(this);
	wxFont cFont(SIMS_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxButton *cButtonRST = new wxButton(cPanel, MY1ID_BUILDRST, wxT("Reset"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonDEF = new wxButton(cPanel, MY1ID_BUILDDEF, wxT("Default"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonNFO = new wxButton(cPanel, MY1ID_BUILDNFO, wxT("Current"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonROM = new wxButton(cPanel, MY1ID_BUILDROM, wxT("Add ROM"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonRAM = new wxButton(cPanel, MY1ID_BUILDRAM, wxT("Add RAM"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonPPI = new wxButton(cPanel, MY1ID_BUILDPPI, wxT("Add PPI"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonOUT = new wxButton(cPanel, MY1ID_BUILDOUT, wxT("EXIT"),
		wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	pBoxSizer->Add(cButtonRST, 1, wxEXPAND);
	pBoxSizer->Add(cButtonDEF, 1, wxEXPAND);
	pBoxSizer->Add(cButtonNFO, 1, wxEXPAND);
	pBoxSizer->Add(cButtonROM, 1, wxEXPAND);
	pBoxSizer->Add(cButtonRAM, 1, wxEXPAND);
	pBoxSizer->Add(cButtonPPI, 1, wxEXPAND);
	pBoxSizer->Add(cButtonOUT, 1, wxEXPAND);
	cPanel->SetSizerAndFit(pBoxSizer);
	return cPanel;
}

wxPanel* my1Form::CreateConsolePanel(wxWindow* aParent)
{
	wxPanel *cPanel = new wxPanel(aParent);
	wxTextCtrl *cConsole = new wxTextCtrl(cPanel, wxID_ANY,
		wxT("Welcome to MY1Sim85\n"), wxDefaultPosition, wxDefaultSize,
		wxTE_AUTO_SCROLL|wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator);
	wxPanel *cComsPanel = new wxPanel(cPanel);
	wxTextCtrl *cCommandText = new wxTextCtrl(cComsPanel, MY1ID_CONSCOMM,
		wxT(""), wxDefaultPosition, wxDefaultSize,wxTE_PROCESS_ENTER);
	cCommandText->Connect(MY1ID_CONSCOMM,wxEVT_KEY_DOWN,
		WX_KEH(my1Form::OnCheckConsole),NULL,this);
	wxButton *cButton = new wxButton(cComsPanel, MY1ID_CONSEXEC,
		wxT("Execute"));
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
	// 'remember' main console
	if(!mConsole) mConsole = cConsole;
	if(!mCommand) mCommand = cCommandText;
	return cPanel;
}

wxPanel* my1Form::CreateMemoryPanel(wxWindow* aParent)
{
	wxGrid *pGrid = 0x0;
	wxPanel *cPanel = CreateMemoryGridPanel(aParent,0x0000,
		MEM_VIEW_WIDTH,MEM_VIEW_HEIGHT,&pGrid);
	wxFont cFont(PANEL_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	my1Memory *pMemory = m8085.Memory(0);
	while(pMemory)
	{
		pMemory->SetLink((void*)pGrid);
		pMemory->DoUpdate = &this->SimUpdateMEM;
		pMemory = (my1Memory*) pMemory->Next();
	}
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
			wxString::Format(wxT("%04X"),aStart+cRow*aWidth));
	for(int cCol=0;cCol<aWidth;cCol++)
		pGrid->SetColLabelValue(cCol,wxString::Format(wxT("%02X"),cCol));
	for(int cRow=0;cRow<aHeight;cRow++)
		for(int cCol=0;cCol<aWidth;cCol++)
			pGrid->SetCellValue(cRow,cCol,wxString::Format(wxT("%02X"),0x0));
	pGrid->DisableCellEditControl();
	pGrid->EnableEditing(false);
	pGrid->SetRowLabelSize(wxGRID_AUTOSIZE);
	pGrid->AutoSize();
	pBoxSizer->Add(pGrid,1,wxEXPAND);
	cPanel->SetSizerAndFit(pBoxSizer);
	*ppGrid = pGrid;
	return cPanel;
}

wxPanel* my1Form::CreateMemoryMiniPanel(int cAddress)
{
	if(cAddress<0)
	{
		cAddress = this->GetBuildAddress(wxT("Start Address for miniMV"));
		if(cAddress<0) return 0x0;
	}
	if(cAddress%8!=0)
	{
		cAddress = cAddress-cAddress%8;
		wxString cStatus = wxT("[miniMV] Address must be in multiples of 8!") +
			wxString::Format(wxT(" Using [0x%04X]"),cAddress);
		this->PrintMessage(cStatus.ToAscii());
	}
	my1Memory* pMemory = (my1Memory*) m8085.MemoryMap().Object((aword)cAddress);
	if(!pMemory)
	{
		wxString cStatus = wxT("[miniMV] Creation Error!");
		cStatus += wxT(" No memory object at address ") +
			wxString::Format(wxT("0x%04X!"),cAddress);
		this->PrintMessage(cStatus.ToAscii());
		return 0x0;
	}
	wxString cPanelName = wxT("miniMV") +
		wxString::Format(wxT("%04X"),cAddress);
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
	wxFont cFont(SIMS_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	// update grid?
	aword cStart = cAddress;
	abyte cData;
	for(int cRow=0;cRow<MEM_MINIVIEW_HEIGHT;cRow++)
	{
		for(int cCol=0;cCol<MEM_MINIVIEW_WIDTH;cCol++)
		{
			if(pMemory->GetData(cStart,cData))
				pGrid->SetCellValue(cRow,cCol,
					wxString::Format(wxT("%02X"),(int)cData));
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

wxPanel* my1Form::CreateDevice7SegPanel(void)
{
	// create unique panel name
	wxString cPanelName=wxT("dev7SEG");
	wxString cPanelCaption=wxT("7-Segment");
	if(!this->GetUniqueName(cPanelName)) return 0x0;
	// create (DEFSIZE_7SEG x 7-segment) panel
	wxPanel *cPanel = new wxPanel(this);
	wxFont cFont(SIMS_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	for(int cLoop=0;cLoop<DEFSIZE_7SEG;cLoop++)
	{
		my1LED7Seg *cTemp;
		wxGBPosition cPosGB;
		wxString cLabel;
		wxGridBagSizer *pGridBagSizer = new wxGridBagSizer(); // vgap,hgap
		// this is 'msb' - for panel port linking (inserted below!)
		my1LEDCtrl* cTest = new my1LEDCtrl(cPanel, wxID_ANY,
			true, DOT_SIZE, DOT_SIZE); // dot
		// bit-6 => 'g'
		cTemp = new my1LED7Seg(cPanel, wxID_ANY, false); // mid horiz
		cLabel = wxT("g"); cTemp->SetLabel(cLabel);
		cPosGB.SetRow(2); cPosGB.SetCol(1);
		pGridBagSizer->Add((wxWindow*)cTemp,cPosGB);
		// bit-5 => 'f'
		cTemp = new my1LED7Seg(cPanel, wxID_ANY, true); // top-left vert
		cLabel = wxT("f"); cTemp->SetLabel(cLabel);
		cPosGB.SetRow(1); cPosGB.SetCol(0);
		pGridBagSizer->Add((wxWindow*)cTemp,cPosGB);
		// bit-4 => 'e'
		cTemp = new my1LED7Seg(cPanel, wxID_ANY, true); // bot-left vert
		cLabel = wxT("e"); cTemp->SetLabel(cLabel);
		cPosGB.SetRow(3); cPosGB.SetCol(0);
		pGridBagSizer->Add((wxWindow*)cTemp,cPosGB);
		// bit-3 => 'd'
		cTemp = new my1LED7Seg(cPanel, wxID_ANY, false); // bot horiz
		cLabel = wxT("d"); cTemp->SetLabel(cLabel);
		cPosGB.SetRow(4); cPosGB.SetCol(1);
		pGridBagSizer->Add((wxWindow*)cTemp,cPosGB);
		// bit-2 => 'c'
		cTemp = new my1LED7Seg(cPanel, wxID_ANY, true); // bot-right vert
		cLabel = wxT("c"); cTemp->SetLabel(cLabel);
		cPosGB.SetRow(3); cPosGB.SetCol(2);
		pGridBagSizer->Add((wxWindow*)cTemp,cPosGB);
		// bit-1 => 'b'
		cTemp = new my1LED7Seg(cPanel, wxID_ANY, true); // top-right vert
		cLabel = wxT("b"); cTemp->SetLabel(cLabel);
		cPosGB.SetRow(1); cPosGB.SetCol(2);
		pGridBagSizer->Add((wxWindow*)cTemp,cPosGB);
		// bit-0 => 'a'
		cTemp = new my1LED7Seg(cPanel, wxID_ANY, false); // top horiz
		cLabel = wxT("a"); cTemp->SetLabel(cLabel);
		cPosGB.SetRow(0); cPosGB.SetCol(1);
		pGridBagSizer->Add((wxWindow*)cTemp,cPosGB);
		// add in here!
		cLabel = wxT("dot"); cTemp->SetLabel(cLabel);
		cPosGB.SetRow(4); cPosGB.SetCol(3);
		pGridBagSizer->Add((wxWindow*)cTest,cPosGB);
		// add this!
		pBoxSizer->AddSpacer(SEG7_NUM_SPACER);
		pBoxSizer->Add(pGridBagSizer, 0, wxEXPAND);
	}
	cPanel->SetSizerAndFit(pBoxSizer);
	// pass to aui manager
	mMainUI.AddPane(cPanel,wxAuiPaneInfo().Name(cPanelName).
		Caption(cPanelCaption).DefaultPane().Fixed().Bottom().DestroyOnClose());
	mMainUI.Update();
	// port selector menu
	cPanel->Connect(cPanel->GetId(),wxEVT_RIGHT_DOWN,
		WX_MEH(my1Form::OnBITPanelClick),NULL,this);
	// return pointer to panel
	return cPanel;
}

wxPanel* my1Form::CreateDeviceLEDPanel(void)
{
	// create unique panel name
	wxString cPanelName=wxT("devLED");
	wxString cPanelCaption=wxT("LED Panel");
	if(!this->GetUniqueName(cPanelName)) return 0x0;
	// create the panel
	wxPanel *cPanel = new wxPanel(this);
	wxFont cFont(SIMS_FONT_SIZE,wxFONTFAMILY_SWISS,
		wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
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
		Caption(cPanelCaption).DefaultPane().Fixed().Top().DestroyOnClose());
	mMainUI.Update();
	// port selector menu
	cPanel->Connect(cPanel->GetId(),wxEVT_RIGHT_DOWN,
		WX_MEH(my1Form::OnBITPanelClick),NULL,this);
	// return pointer to panel
	return cPanel;
}

wxPanel* my1Form::CreateDeviceSWIPanel(void)
{
	// create unique panel name
	wxString cPanelName=wxT("devSWI");
	wxString cPanelCaption=wxT("Switch Panel");
	if(!this->GetUniqueName(cPanelName)) return 0x0;
	// create the panel
	wxPanel *cPanel = new wxPanel(this);
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
		Caption(cPanelCaption).DefaultPane().Fixed().Top().DestroyOnClose());
	mMainUI.Update();
	// port selector menu
	cPanel->Connect(cPanel->GetId(),wxEVT_RIGHT_DOWN,
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
		cTempFile = wxT("unnamed");
	mNoteBook->AddPage(cCodeEdit, cTempFile,true);
	if(mOptions.mConv_UnixEOL)
		cCodeEdit->ConvertEOLs(2);
	wxString cStatus = wxT("File ") +
		cCodeEdit->GetFileName() + wxT(" loaded!");
	this->ShowStatus(cStatus);
}

void my1Form::SaveEdit(wxWindow* cEditPane, bool aSaveAs)
{
	wxString cFileName;
	my1CodeEdit *cEditor = (my1CodeEdit*) cEditPane;
	if(aSaveAs||!cEditor->GetFileName().Length())
	{
		wxFileDialog *cSelect = new wxFileDialog(this,wxT("Assign File Name"),
			wxT(""),wxT(""),wxT("Any file (*.*)|*.*"),
			wxFD_SAVE|wxFD_OVERWRITE_PROMPT|wxFD_CHANGE_DIR);
		cSelect->SetWildcard("ASM files (*.asm)|*.asm|"
			"8085 ASM files (*.8085)|*.8085|Any file (*.*)|*.*");
		if(cSelect->ShowModal()!=wxID_OK) return;
		cFileName = cSelect->GetPath();
	}
	cEditor->SaveEdit(cFileName);
	wxString cStatus = wxT("File ") + cEditor->GetFileName() + wxT(" saved!");
	this->ShowStatus(cStatus);
}

void my1Form::ShowStatus(wxString& aString)
{
	this->SetStatusText(aString,STATUS_MSG_INDEX);
	mDisplayTimer->Start(STATUS_MSG_PERIOD,wxTIMER_ONE_SHOT);
}

void my1Form::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

void my1Form::OnNew(wxCommandEvent& WXUNUSED(event))
{
	wxString cFileName = wxT("");
	this->OpenEdit(cFileName);
}

void my1Form::OnLoad(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog *cSelect = new wxFileDialog(this,wxT("Select code file"),
		wxT(""),wxT(""),wxT("Any file (*.*)|*.*"),
		wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_CHANGE_DIR);
	cSelect->SetWildcard("ASM files (*.asm)|*.asm|"
		"8085 ASM files (*.8085)|*.8085|Any file (*.*)|*.*");
	if(cSelect->ShowModal()!=wxID_OK) return;
	wxString cFileName = cSelect->GetPath();
	this->OpenEdit(cFileName);
}

void my1Form::OnSave(wxCommandEvent &event)
{
	int cSelect = mNoteBook->GetSelection();
	if(cSelect<0) return;
	wxWindow *cTarget = mNoteBook->GetPage(cSelect);
	if(!cTarget->IsKindOf(CLASSINFO(my1CodeEdit))) return;
	bool cSaveAs = false;
	if(event.GetId()==MY1ID_SAVEAS) cSaveAs = true;
	this->SaveEdit(cTarget,cSaveAs);
}

void my1Form::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxAboutDialogInfo cAboutInfo;
	cAboutInfo.SetName(MY1APP_PROGNAME);
	cAboutInfo.SetVersion(MY1APP_PROGVERS);
	cAboutInfo.SetDescription(wxT("8085 Microprocessor System Simulator"));
	cAboutInfo.SetCopyright("(C) 2011-2012 Azman M. Yusof");
	cAboutInfo.SetWebSite("http://www.my1matrix.org");
	cAboutInfo.AddDeveloper("Azman M. Yusof <azman@my1matrix.net>");
	wxAboutBox(cAboutInfo,this);
}

void my1Form::OnAssemble(wxCommandEvent &event)
{
	my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
	if(!cEditor)
	{
		int cSelect = mNoteBook->GetSelection();
		wxWindow *cTarget = mNoteBook->GetPage(cSelect);
		if(!cTarget->IsKindOf(CLASSINFO(my1CodeEdit))) return;
		cEditor = (my1CodeEdit*) cTarget;
	}
	if(cEditor->GetModify())
	{
		int cGoSave = wxMessageBox(wxT("Save & Continue?"),
			wxT("File modified!"),wxOK|wxCANCEL,this);
		if(cGoSave==wxCANCEL) return;
		this->SaveEdit((wxWindow*)cEditor);
	}
	wxString cStatus = wxT("Processing ") + cEditor->GetFileName() + wxT("...");
	this->ShowStatus(cStatus);
	if(m8085.Assemble(cEditor->GetFullName().ToAscii()))
	{
		cStatus = wxT("[SUCCESS] Code in ") +
			cEditor->GetFileName() + wxT(" processed!");
		this->ShowStatus(cStatus);
		m8085.SetCodeLink((void*)cEditor);
	}
	else
	{
		cStatus = wxT("[ERROR] Check start address?");
		this->ShowStatus(cStatus);
	}
}

void my1Form::OnSimulate(wxCommandEvent &event)
{
	my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
	if(!cEditor||cEditor->GetModify())
		this->OnAssemble(event);
	cEditor = (my1CodeEdit*) m8085.GetCodeLink();
	if(!cEditor) return;
	wxString cStatus = wxT("Preparing ") + cEditor->GetFileName() + wxT("...");
	this->ShowStatus(cStatus);
	m8085.SetStartAddress(mOptions.mSims_StartADDR);
	if(m8085.Simulate(0)) // force a reset!
	{
		if(m8085.NoCodex())
		{
			cStatus = wxT("[INFO] No code @ address 0x") +
				wxString::Format(wxT("%04X"),mOptions.mSims_StartADDR);
			this->PrintMessage(cStatus.ToAscii());
			return;
		}
		cStatus = wxT("[SUCCESS] Ready for Simulation!");
		this->ShowStatus(cStatus);
		this->SimulationMode();
		cEditor->SetReadOnly(mSimulationMode);
		if(!mOptions.mSims_FreeRunning)
			cEditor->ExecLine(m8085.GetCodexLine()-1);
		mCommand->SetFocus();
	}
	else
	{
		cStatus = wxT("[ERROR] No memory @ start address?");
		this->ShowStatus(cStatus);
	}
}

void my1Form::OnGenerate(wxCommandEvent &event)
{
	my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
	if(!cEditor||cEditor->GetModify())
		this->OnAssemble(event);
	cEditor = (my1CodeEdit*) m8085.GetCodeLink();
	if(!cEditor) return;
	wxString cFileHEX = cEditor->GetFileNoXT() + wxT(".HEX");
	wxString cStatus = wxT("Processing ") + cEditor->GetFileName() + wxT("...");
	this->ShowStatus(cStatus);
	if(m8085.Generate(cFileHEX.ToAscii()))
	{
		cStatus = wxT("[SUCCESS] HEX file ") + cFileHEX + wxT(" written!");
		this->ShowStatus(cStatus);
	}
	else
	{
		cStatus = wxT("[ERROR] Cannot generate HEX file!");
		this->ShowStatus(cStatus);
	}
}

void my1Form::PrintValueDEC(int aValue, int aWidth, bool aNewline)
{
	std::cout << std::setw(aWidth) << std::setbase(10) << aValue;
	if(aNewline) std::cout << std::endl;
}

void my1Form::PrintValueHEX(int aValue, int aWidth, bool aNewline)
{
	std::cout << std::setw(aWidth) << std::setfill('0') << std::hex << aValue;
	if(aNewline) std::cout << std::endl;
}

void my1Form::PrintMessage(const wxString& aMessage, bool aNewline)
{
	std::cout << aMessage;
	if(aNewline) std::cout << std::endl;
}

void my1Form::PrintTaggedMessage(const wxString& aTag, const wxString& aMessage)
{
	wxString cString = wxT("\n[") + aTag + wxT("] ") + aMessage;
	this->PrintMessage(cString,true);
}

void my1Form::PrintInfoMessage(const wxString& aMessage)
{
	this->PrintTaggedMessage(wxT("INFO"),aMessage);
}

void my1Form::PrintErrorMessage(const wxString& aMessage)
{
	this->PrintTaggedMessage(wxT("ERROR"),aMessage);
}

void my1Form::PrintAddressMessage(const wxString& aMessage, unsigned long aStart)
{
	this->PrintMessage("\n@[");
	this->PrintValueHEX(aStart,4);
	this->PrintMessage("] : ");
	this->PrintMessage(aMessage,true);
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
	std::cout << "\n";
	// print header!
	std::cout << "\n\xa9\xc4\xc4\xc4\xc4\xc4\xc4";
	for(int cLoop=0;cLoop<PRINT_BPL_COUNT;cLoop++)
		std::cout << "\xc2\xc4\xc4\xc4\xc4";
	std::cout << "\xaa";
	std::cout << "\n|      | ";
	for(int cLoop=0;cLoop<PRINT_BPL_COUNT;cLoop++)
		std::cout << my1ValueHEX(cLoop,2) << " | ";
	// print table!
	while(cCount<aSize&&cAddress<MAX_MEMSIZE)
	{
		if(!m8085.MemoryMap().Read(cAddress,cData))
		{
			this->PrintMessage("\n[R/W ERROR] Cannot read from address 0x");
			this->PrintValueHEX(cAddress,4,true);
			break;
		}
		if(cCount%PRINT_BPL_COUNT==0)
		{
			std::cout << "\n---------";
			for(int cLoop=0;cLoop<PRINT_BPL_COUNT;cLoop++)
				std::cout << "-----";
			std::cout << "\n| " << my1ValueHEX(cAddress,4) << " | ";
		}
		std::cout << my1ValueHEX(cData,2) << " | ";
		cCount++; cAddress++;
	}
	std::cout << "\n---------";
	for(int cLoop=0;cLoop<PRINT_BPL_COUNT;cLoop++)
		std::cout << "-----";
	std::cout << "\n";
}

void my1Form::PrintPeripheralInfo(void)
{
	std::cout << "\n";
	std::cout << "Memory Count: " << m8085.MemoryMap().GetCount() << "\n";
	my1Memory* cMemory = m8085.Memory(0);
	while(cMemory)
	{
		std::cout << "(Memory) Name: " << cMemory->GetName() << ", ";
		std::cout << "Read-Only: " << cMemory->IsReadOnly() << ", ";
		std::cout << "Start: 0x" << std::setw(4) << std::setfill('0')
			<< std::setbase(16) << cMemory->GetStart() << ", ";
		std::cout << "Size: 0x" << std::setw(4) << std::setfill('0')
			<< std::setbase(16) << cMemory->GetSize() << "\n";
		cMemory = (my1Memory*) cMemory->Next();
	}
	std::cout << "Device Count: " << m8085.DeviceMap().GetCount() << "\n";
	my1Device* cDevice = m8085.Device(0);
	while(cDevice)
	{
		std::cout << "(Device) Name: " << cDevice->GetName() << ", ";
		std::cout << "Start: 0x" << std::setw(2) << std::setfill('0')
			<< cDevice->GetStart() << ", ";
		std::cout << "Size: 0x" << std::setw(2) << std::setfill('0')
			<< std::setbase(16) << cDevice->GetSize() << "\n";
		cDevice = (my1Device*) cDevice->Next();
	}
}

void my1Form::PrintSimInfo(void)
{
	std::cout << "\nSimulation Info";
	std::cout << "SimCycleDefault=" << mSimulationCycleDefault << "ns,";
	std::cout << "SimCycle=" << mSimulationCycle << "ns\n";
}

void my1Form::PrintHelp(void)
{
	std::cout << "\nAvailable command(s):" << "\n";
	std::cout << "- show [system|mem=?|minimv=?]" << "\n";
	std::cout << "  > system (print system info)" << "\n";
	std::cout << "  > mem=? (show memory starting from given addr)" << "\n";
	std::cout << "  > minimv=? (show memory on mini memory viewer)" << "\n";
	std::cout << "- sim [info|addr=?|mark=?|break=?]" << "\n";
	std::cout << "  > info (simulation timing info)" << "\n";
	std::cout << "  > addr=? (set simulation start addr)" << "\n";
	std::cout << "  > mark=? (show/hide line marker)" << "\n";
	std::cout << "  > break=? (toggle breakpoint at line)" << "\n";
	std::cout << "- build [info|default|reset|rom=?|ram=?|ppi=?]" << "\n";
	std::cout << "  > info (print system info)" << "\n";
	std::cout << "  > default (build default system)" << "\n";
	std::cout << "  > reset (reset system build)" << "\n";
	std::cout << "  > rom=? (add 2764 ROM @given addr)" << "\n";
	std::cout << "  > ram=? (add 6264 RAM @given addr)" << "\n";
	std::cout << "  > ppi=? (add 8255 PPI @given addr)" << "\n";
	std::cout << "- help" << "\n";
	std::cout << "  > show this text" << "\n";
}

void my1Form::PrintUnknownCommand(const wxString& aCommand)
{
	std::cout << "\nUnknown command '" << aCommand << "'\n";
}

void my1Form::PrintUnknownParameter(const wxString& aParam,
	const wxString& aCommand)
{
	std::cout << "\nUnknown parameter '" << aParam
		<< "' for [" << aCommand << "]\n";
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
	if(!cCommandWord.Cmp(wxT("show")))
	{
		wxString cParam = cParameters.BeforeFirst(' ');
		int cEqual = cParam.Find('=');
		if(cEqual==wxNOT_FOUND)
		{
			if(!cParam.Cmp(wxT("system")))
			{
				this->PrintPeripheralInfo();
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
			if(!cKey.Cmp(wxT("mem")))
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
			else if(!cKey.Cmp(wxT("minimv")))
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
	else if(!cCommandWord.Cmp(wxT("sim")))
	{
		wxString cParam = cParameters.BeforeFirst(' ');
		int cEqual = cParam.Find('=');
		if(cEqual==wxNOT_FOUND)
		{
			if(!cParam.Cmp(wxT("info")))
			{
				if(mSimulationMode)
					m8085.PrintCodexInfo();
				else
					this->PrintSimInfo();
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
			if(!cKey.Cmp(wxT("addr")))
			{
				unsigned long cStart;
				if(cValue.ToULong(&cStart,16)&&cStart<0x1000)
				{
					mOptions.mSims_StartADDR = cStart;
					this->PrintMessage("\nSimulation Start Addr Changed to 0x");
					this->PrintValueHEX(cStart,4,true);
					cValidCommand = true;
				}
				else
				{
					this->PrintErrorMessage("Cannot Change Start Address\n");
					this->PrintUnknownParameter(cValue,cKey);
				}
			}
			else if(!cKey.Cmp(wxT("mark")))
			{
				unsigned long cStart;
				cValue.ToULong(&cStart,10);
				if(cStart)
				{
					mOptions.mSims_FreeRunning = false;
					this->PrintMessage("Sim Marker: ON!");
				}
				else
				{
					mOptions.mSims_FreeRunning = true;
					this->PrintMessage("Sim Marker: OFF!");
				}
				cValidCommand = true;
			}
			else if(!cKey.Cmp(wxT("break")))
			{
				if(!mSimulationMode)
				{
					this->PrintMessage("This feature "
						"is only available in simulation mode!");
					return;
				}
				my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
				if(!cEditor)
				{
					this->PrintMessage("[BREAK ERROR] "
						"Cannot get editor link!");
					return;
				}
				unsigned long cStart;
				if(cValue.ToULong(&cStart,10)&&
					(int)cStart>0&&
					(int)cStart<=cEditor->GetLineCount())
				{
					cEditor->ToggleBreak(cStart-1);
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
	else if(!cCommandWord.Cmp(wxT("build")))
	{
		if(mSimulationMode)
		{
			this->PrintMessage("Build mode disabled during simulation!");
			return;
		}
		wxString cParam = cParameters.BeforeFirst(' ');
		int cEqual = cParam.Find('=');
		if(cEqual==wxNOT_FOUND)
		{
			if(!cParam.Cmp(wxT("info")))
			{
				this->PrintPeripheralInfo();
				cValidCommand = true;
			}
			else if(!cParam.Cmp(wxT("default")))
			{
				this->SystemDefault();
				cValidCommand = true;
			}
			else if(!cParam.Cmp(wxT("reset")))
			{
				this->SystemReset();
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
			if(!cKey.Cmp(wxT("rom")))
			{
				unsigned long cStart;
				if(cValue.ToULong(&cStart,16)&&cStart<=0xFFFF)
					this->AddROM(cStart);
				else
					this->PrintUnknownParameter(cValue,cKey);
				cValidCommand = true;
			}
			else if(!cKey.Cmp(wxT("ram")))
			{
				unsigned long cStart;
				if(cValue.ToULong(&cStart,16)&&cStart<=0xFFFF)
					this->AddRAM(cStart);
				else
					this->PrintUnknownParameter(cValue,cKey);
				cValidCommand = true;
			}
			else if(!cKey.Cmp(wxT("ppi")))
			{
				unsigned long cStart;
				if(cValue.ToULong(&cStart,16)&&cStart<=0xFF)
					this->AddPPI(cStart);
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
	else if(!cCommandWord.Cmp(wxT("help")))
	{
		this->PrintHelp();
		cValidCommand = true;
	}
	else if(!cCommandWord.Cmp(wxT("test")))
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
			mSimulationStepping = true;
			mSimulationRunning = true;
			break;
		default:
			mSimulationStepping = false;
			mSimulationRunning = false;
	}
	if(mSimulationRunning&&!mSimExecTimer->IsRunning())
		mSimExecTimer->Start(SIM_EXEC_PERIOD,wxTIMER_ONE_SHOT);
}

void my1Form::OnSimulationInfo(wxCommandEvent &event)
{
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
		m8085.Simulate(0);
		my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
		if(!cEditor)
		{
			this->PrintMessage("[RESET] Cannot get editor link!");
			return;
		}
		cEditor->ExecLine(m8085.GetCodexLine()-1);
	}
	else if(event.GetId()==MY1ID_SIMSBRKP)
	{
		my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
		if(!cEditor)
		{
			this->PrintMessage("[BREAK] Cannot get editor link!");
			return;
		}
		cEditor->ToggleBreak(cEditor->GetCurrentLine());
	}
}

void my1Form::OnSimulationExit(wxCommandEvent &event)
{
	if(event.GetId()==MY1ID_SIMSEXIT)
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
		wxT("Enter Address in HEX"), aString);
	if(cDialog->ShowModal()!=wxID_OK)
		return -1;
	unsigned long cStart = 0x0;
	cDialog->GetValue().ToULong(&cStart,16);
	return cStart;
}

void my1Form::OnBuildSelect(wxCommandEvent &event)
{
	int cAddress;
	switch(event.GetId())
	{
		case MY1ID_BUILDINIT:
			this->BuildMode();
			break;
		case MY1ID_BUILDRST:
			this->SystemReset();
			break;
		case MY1ID_BUILDDEF:
			this->SystemDefault();
			break;
		case MY1ID_BUILDNFO:
			this->PrintPeripheralInfo();
			break;
		case MY1ID_BUILDROM:
			cAddress = this->GetBuildAddress(wxT("[BUILD] Adding 2764 ROM"));
			if(cAddress<0) return;
			this->AddROM(cAddress);
			break;
		case MY1ID_BUILDRAM:
			cAddress = this->GetBuildAddress(wxT("[BUILD] Adding 6264 RAM"));
			if(cAddress<0) return;
			this->AddRAM(cAddress);
			break;
		case MY1ID_BUILDPPI:
			cAddress = this->GetBuildAddress(wxT("[BUILD] Adding 8255 PPI"));
			if(cAddress<0) return;
			this->AddPPI(cAddress);
			break;
		case MY1ID_BUILDOUT:
		default:
			this->BuildMode(false);
			break;
	}
}

void my1Form::OnClosePane(wxAuiManagerEvent &event)
{
	wxAuiPaneInfo *cPane = event.GetPane();
	wxAuiPaneInfo &rPane = mMainUI.GetPane("simsPanel");
	if(cPane==&rPane)
	{
		event.Veto();
		return;
	}
	// browse for mini mem viewer!
	my1MiniViewer *pViewer = mFirstViewer, *pPrev = 0x0;
	while(pViewer)
	{
		wxString cPanelName = wxT("miniMV")
			+ wxString::Format(wxT("%04X"),pViewer->mStart);
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

void my1Form::OnShowPanel(wxCommandEvent &event)
{
	wxString cToolName = wxT("");
	switch(event.GetId())
	{
		case MY1ID_VIEW_REGSPANE:
			cToolName = wxT("regsPanel");
			break;
		case MY1ID_VIEW_INTRPANE:
			cToolName = wxT("intrPanel");
			break;
		case MY1ID_VIEW_CONSPANE:
			cToolName = wxT("consPanel");
			break;
		case MY1ID_CREATE_MINIMV:
			this->CreateMemoryMiniPanel();
			break;
		case MY1ID_CREATE_DV7SEG:
			this->CreateDevice7SegPanel();
			break;
		case MY1ID_CREATE_DEVLED:
			this->CreateDeviceLEDPanel();
			break;
		case MY1ID_CREATE_DEVSWI:
			this->CreateDeviceSWIPanel();
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
		wxT("Options"),this->mOptions);
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
			if(cTarget->IsKindOf(CLASSINFO(my1CodeEdit)))
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
	this->SetStatusText(wxT(""),STATUS_MSG_INDEX);
}

void my1Form::OnSimExeTimer(wxTimerEvent& event)
{
	if(m8085.Simulate())
	{
		my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
		if(!mOptions.mSims_FreeRunning)
			cEditor->ExecLine(m8085.GetCodexLine()-1);
		else
			cEditor->ExecDone();
		if(mOptions.mSims_ShowRunInfo)
			m8085.PrintCodexPrev();
		if(cEditor->IsBreakLine())
			mSimulationStepping = true;
		if(m8085.NoCodex())
		{
			this->PrintMessage("[INFO] No code @ address!");
			mSimulationStepping = true;
		}
		else if(m8085.Halted())
		{
			this->PrintMessage("[INFO] System HALTED!");
			mSimulationStepping = true;
		}
		else if(m8085.Interrupted())
		{
			this->PrintMessage("[INFO] System Interrupt!");
			mSimulationStepping = mOptions.mSims_PauseOnINTR;
		}
	}
	else
	{
		my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
		wxMessageBox(wxT("Simulation Terminated!"),wxT("[SIM Error]"),
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
	if(mSimulationMode||mBuildMode)
	{
		event.Veto();
	}
}

void my1Form::OnPageChanged(wxAuiNotebookEvent &event)
{
	wxMenuBar *cMenuBar = this->GetMenuBar();
	wxAuiToolBar *cProcTool = (wxAuiToolBar*) this->FindWindow(MY1ID_PROCTOOL);
	int cSelect = event.GetSelection();
	wxWindow *cTarget = mNoteBook->GetPage(cSelect);
	if(!cTarget) return;
	m8085.SetCodeLink((void*)0x0);
	if(cTarget->IsKindOf(CLASSINFO(my1CodeEdit)))
	{
		cMenuBar->EnableTop(cMenuBar->FindMenu(wxT("Tool")),true);
		cProcTool->Enable();
	}
	else
	{
		cMenuBar->EnableTop(cMenuBar->FindMenu(wxT("Tool")),false);
		cProcTool->Enable(false);
	}
}

void my1Form::OnPageClosing(wxAuiNotebookEvent &event)
{
	wxWindow *cTarget = mNoteBook->GetPage(event.GetSelection());
	if(cTarget->IsKindOf(CLASSINFO(my1CodeEdit)))
	{
		my1CodeEdit *cEditor = (my1CodeEdit*) cTarget;
		if(cEditor->GetModify())
		{
			int cGoSave = wxMessageBox(wxT("Save Before Closing?"),
				wxT("Code Modified!"),wxYES|wxNO|wxCANCEL,this);
			if(cGoSave==wxYES)
				this->SaveEdit(cTarget);
			else if(cGoSave==wxCANCEL)
				event.Veto();
		}
	}
	else // must be welcome page?
	{
		event.Veto();
	}
}

void my1Form::OnBITPanelClick(wxMouseEvent &event)
{
	mPortPanel = 0x0;
	if(event.RightDown())
	{
		wxMenu *cMenuPop = this->GetDevicePortMenu();
		if(!cMenuPop) return;
		mPortPanel = (wxPanel*) FindWindowById(event.GetId(),this);
		if(!mPortPanel) return;
		mPortPanel->Connect(wxID_ANY,wxEVT_COMMAND_MENU_SELECTED,
			WX_CEH(my1Form::OnBITPortClick),NULL,this);
		mPortPanel->PopupMenu(cMenuPop);
	}
}

void my1Form::OnBITPortClick(wxCommandEvent &event)
{
	int cCheck = event.GetId() - MY1ID_CPOT_OFFSET;
	if(cCheck<0||cCheck>=MY1ID_CBIT_OFFSET) return;
	int cDevice = cCheck/(I8255_SIZE-1);
	int cDevIdx = cCheck%(I8255_SIZE-1);
	my1Device *pDevice = this->Device(cDevice);
	if(!pDevice) { mPortPanel = 0x0; return; }
	my1DevicePort *pPort = pDevice->GetDevicePort(cDevIdx);
	if(!pPort) { mPortPanel = 0x0; return; }
	wxWindowList& cList = this->PortPanel()->GetChildren();
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
			if(pTarget->IsKindOf(CLASSINFO(my1BITCtrl)))
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

my1BitIO* my1Form::GetDeviceBit(my1BitSelect& aSelect)
{
	int cAddress;
	my1BitIO *pBit = 0x0;
	my1Device *pDevice = (my1Device*) m8085.DeviceMap().
		Object(aSelect.mDevice,&cAddress);
	if(pDevice)
	{
		my1DevicePort *pPort = pDevice->GetDevicePort(aSelect.mDevicePort);
		if(pPort) pBit = pPort->GetBitIO(aSelect.mDeviceBit);
		if(pBit)
		{
			aSelect.mPointer = (void*) pBit;
			aSelect.mDeviceAddr = cAddress;
		}
	}
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
		wxMessageBox(wxT("Build a system with PPI!"),
				wxT("System Incomplete!"),wxOK|wxICON_EXCLAMATION,this);
		return 0x0;
	}
	if(!mDevicePopupMenu)
	{
		mDevicePopupMenu = new wxMenu;
		int cDevID = MY1ID_DSEL_OFFSET+MY1ID_DEVC_OFFSET;
		//int cPotID = MY1ID_DSEL_OFFSET+MY1ID_PORT_OFFSET;
		int cBitID = MY1ID_DSEL_OFFSET+MY1ID_DBIT_OFFSET;
		my1Device *pDevice = m8085.Device(0);
		while(pDevice)
		{
			//wxMenu *cMenuPort = new wxMenu;
			wxMenu *cMenuBit = new wxMenu;
			for(int cPort=0;cPort<I8255_SIZE-1;cPort++)
			{
				//wxMenu *cMenuBit = new wxMenu;
				wxString cPortText = wxT("P") +
					wxString::Format(wxT("%c"),(char)(cPort+(int)'A'));
				for(int cLoop=0;cLoop<I8255_DATASIZE;cLoop++)
				{
					wxString cText = cPortText +
						wxString::Format(wxT("%01X"),cLoop);
					cMenuBit->Append(cBitID++,cText,
						wxEmptyString,wxITEM_CHECK);
				}
				//wxString cText = wxT("Port ") +
				//	wxString::Format(wxT("%c"),(char)(cPort+(int)'A'));
				//cMenuPort->Append(cPotID++, cText, cMenuBit);
			}
			wxString cText = wxT("Device @") +
				wxString::Format(wxT("%02X"),pDevice->GetStart());
			//mDevicePopupMenu->Append(cDevID++, cText, cMenuPort);
			mDevicePopupMenu->Append(cDevID++, cText, cMenuBit);
			pDevice = (my1Device*) pDevice->Next();
		}
	}
	// make sure all items are unchecked?
	int cCountD = mDevicePopupMenu->GetMenuItemCount();
	for(int cLoopD=0;cLoopD<cCountD;cLoopD++)
	{
		wxMenuItem *cItemD = mDevicePopupMenu->FindItemByPosition(cLoopD);
		wxMenu *cMenuD = cItemD->GetSubMenu();
		//int cCountP = cMenuD->GetMenuItemCount();
		//for(int cLoopP=0;cLoopP<cCountP;cLoopP++)
		//{
			//wxMenuItem *cItemP = cMenuD->FindItemByPosition(cLoopP);
			//wxMenu *cMenuP = cItemP->GetSubMenu();
			//int cCountB = cMenuP->GetMenuItemCount();
			int cCountB = cMenuD->GetMenuItemCount();
			for(int cLoopB=0;cLoopB<cCountB;cLoopB++)
			{
				//wxMenuItem *cItem = cMenuP->FindItemByPosition(cLoopB);
				wxMenuItem *cItem = cMenuD->FindItemByPosition(cLoopB);
				cItem->Check(false);
				int cCheck = cItem->GetId() -
					(MY1ID_DSEL_OFFSET+MY1ID_DBIT_OFFSET);
				my1BitSelect cSelect;
				cSelect.UseIndex(cCheck);
				my1BitIO* pBit = this->GetDeviceBit(cSelect);
				if(pBit->GetLink())
					cItem->Enable(false);
			}
		//}
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
		wxMessageBox(wxT("Build a system with PPI!"),
				wxT("System Incomplete!"),wxOK|wxICON_EXCLAMATION,this);
		return 0x0;
	}
	mDevicePortMenu = new wxMenu;
	int cPortID = MY1ID_DSEL_OFFSET+MY1ID_PORT_OFFSET;
	my1Device *pDevice = m8085.Device(0);
	while(pDevice)
	{
		for(int cPort=0;cPort<I8255_SIZE-1;cPort++)
		{
			wxString cPortText = wxT("P") +
				wxString::Format(wxT("%c"),(char)(cPort+(int)'A')) +
				wxString::Format(wxT(" @%02X"),pDevice->GetStart()+cPort);
			wxMenuItem* cItem = mDevicePortMenu->Append(cPortID++,cPortText,
				wxEmptyString,wxITEM_CHECK);
			cItem->Check(false); // unchecked
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

void my1Form::SimUpdateFLAG(void* simObject)
{
	// update flag if necessary?
	my1Reg85 *pReg85 = (my1Reg85*) simObject;
	wxString cFlag = wxString::Format(wxT("%01X"),
			pReg85->GetData()&I8085_FLAG_C?1:0);
	wxTextCtrl *pText = (wxTextCtrl*) this->FlagLink(I8085_FLAG_C).GetLink();
	pText->ChangeValue(cFlag);
	cFlag = wxString::Format(wxT("%01X"),
			pReg85->GetData()&I8085_FLAG_P?1:0);
	pText = (wxTextCtrl*) this->FlagLink(I8085_FLAG_P).GetLink();
	pText->ChangeValue(cFlag);
	cFlag = wxString::Format(wxT("%01X"),
			pReg85->GetData()&I8085_FLAG_A?1:0);
	pText = (wxTextCtrl*) this->FlagLink(I8085_FLAG_A).GetLink();
	pText->ChangeValue(cFlag);
	cFlag = wxString::Format(wxT("%01X"),
			pReg85->GetData()&I8085_FLAG_Z?1:0);
	pText = (wxTextCtrl*) this->FlagLink(I8085_FLAG_Z).GetLink();
	pText->ChangeValue(cFlag);
	cFlag = wxString::Format(wxT("%01X"),
			pReg85->GetData()&I8085_FLAG_S?1:0);
	pText = (wxTextCtrl*) this->FlagLink(I8085_FLAG_S).GetLink();
	pText->ChangeValue(cFlag);
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

bool my1Form::SystemDefault(void)
{
	this->ResetDevicePopupMenu(true);
	bool cFlag = m8085.BuildDefault();
	if(cFlag)
	{
		this->PrintInfoMessage("Default system built!");
		// default switch panel
		if(!mSWIPanel)  mSWIPanel = this->CreateDeviceSWIPanel();
		if(!this->LinkPanelToPort(mSWIPanel,1))
			this->PrintErrorMessage("Cannot link switch panel!");
		// default led panel
		if(!mLEDPanel)  mLEDPanel = this->CreateDeviceLEDPanel();
		if(!this->LinkPanelToPort(mLEDPanel,0))
			this->PrintErrorMessage("Cannot link LED panel!");
	}
	else
		this->PrintErrorMessage("Default system build FAILED!");
	return cFlag;
}

bool my1Form::SystemReset(void)
{
	this->ResetDevicePopupMenu(true);
	bool cFlag = m8085.BuildReset();
	if(cFlag)
		this->PrintInfoMessage("System build reset!");
	else
		this->PrintErrorMessage("System build reset FAILED!");
	return cFlag;
}

bool my1Form::AddROM(int aStart)
{
	bool cFlag = false;
	if((cFlag=m8085.AddROM(aStart)))
		this->PrintAddressMessage(wxT("2764 ROM added!"),aStart);
	else
		this->PrintAddressMessage(wxT("FAILED to add 2764 ROM!"),aStart);
	return cFlag;
}

bool my1Form::AddRAM(int aStart)
{
	bool cFlag = false;
	if((cFlag=m8085.AddRAM(aStart)))
		this->PrintAddressMessage(wxT("6264 RAM added!"),aStart);
	else
		this->PrintAddressMessage(wxT("FAILED to add 6264 RAM!"),aStart);
	return cFlag;
}

bool my1Form::AddPPI(int aStart)
{
	bool cFlag = m8085.AddPPI(aStart);
	if(cFlag)
		this->PrintAddressMessage(wxT("8255 PPI added!"),aStart);
	else
		this->PrintAddressMessage(wxT("FAILED to add 8255 PPI!"),aStart);
	if(cFlag) this->ResetDevicePopupMenu();
	return cFlag;
}

void my1Form::SimUpdateREG(void* simObject)
{
	// update register view
	my1Reg85 *pReg85 = (my1Reg85*) simObject;
	wxString cFormat = "%02X";
	if(pReg85->IsReg16()) cFormat = "%04X";
	cFormat = wxString::Format(cFormat,pReg85->GetData());
	wxTextCtrl *pText = (wxTextCtrl*) pReg85->GetLink();
	pText->ChangeValue(cFormat);
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
	pGrid->SetCellValue(cRow,cCol,wxString::Format(wxT("%02X"),cData));
	// find mini viewers
	wxWindow* pParent = pGrid->GetGrandParent(); // get infobook
	my1Form* pForm =  (my1Form*) pParent->GetGrandParent(); // get the form!
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
				wxString::Format(wxT("%02X"),cData));
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
	my1Sim85* mySim = (my1Sim85*) simObject;
	my1Form* myForm = (my1Form*) mySim->GetLink();
	wxMicroSleep(myForm->GetSimDelay()*aCount);
}
