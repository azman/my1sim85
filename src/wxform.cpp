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

#include "wx/aboutdlg.h"
#include "wx/grid.h"

#define MACRO_WXBMP(bmp) wxBitmap(bmp##_xpm)
#define MACRO_WXICO(bmp) wxIcon(bmp##_xpm)

#include "../res/apps.xpm"
#include "../res/exit.xpm"
#include "../res/newd.xpm"
#include "../res/open.xpm"
#include "../res/save.xpm"
#include "../res/binary.xpm"
#include "../res/option.xpm"
//#include "../res/gear.xpm"
#include "../res/hexgen.xpm"
#include "../res/simx.xpm"
#include "../res/target.xpm"

#include <iostream>
#include <iomanip>
#include <ctime>

// bug when placing at screen edge? gtk only?
#define AUI_GO_FLOAT true

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define INFO_PANEL_WIDTH 200
#define DEVC_PANEL_WIDTH 100
#define CONS_PANEL_HEIGHT 100
#define INFO_REG_SPACER 5
#define INFO_DEV_SPACER 5
#define STATUS_COUNT 2
#define STATUS_FIX_WIDTH INFO_PANEL_WIDTH
#define STATUS_MSG_INDEX 1
#define STATUS_MSG_PERIOD 3000
#define SIM_START_ADDR 0x0000
#define SIM_EXEC_PERIOD 1
#define TOOL_FILE_POS 1
#define TOOL_EDIT_POS 2
#define TOOL_PROC_POS 3
#define TOOL_REGI_POS 1
#define TOOL_MEMO_POS 2
#define TITLE_FONT_SIZE 24
#define EMAIL_FONT_SIZE 8
#define INFO_FONT_SIZE 8
#define LOGS_FONT_SIZE 8
#define FLOAT_INIT_X 40
#define FLOAT_INIT_Y 40

my1Form::my1Form(const wxString &title)
	: wxFrame( NULL, MY1ID_MAIN, title, wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_FRAME_STYLE)
{
	mBuildMode = false;
	// simulation stuffs
	mSimulationMode = false;
	mSimulationRunning = false;
	mSimulationStepping = false;
	mSimulationCycle = 0.0;
	mSimulationCycleDefault = 0.0;
	this->CalculateSimCycle();

	// default option?
	mOptions.mChanged = false;
	mOptions.mEdit_ViewWS = false;
	mOptions.mEdit_ViewEOL = false;
	mOptions.mConv_UnixEOL = false;
	mOptions.mSims_FreeRunning = false;
	mOptions.mSims_StartADDR = SIM_START_ADDR;

	// assign function pointers :p
	m8085.SetLink((void*)this);
	m8085.DoUpdate = &this->SimDoUpdate;
	m8085.DoDelay = &this->SimDoDelay;
	m8085.BuildDefault();

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

	// some handy pointers
	mConsole = 0x0;
	mCommand = 0x0;
	mDevicePopupMenu = 0x0;

	// setup image
	wxInitAllImageHandlers();
	wxIcon mIconApps = MACRO_WXICO(apps);
	this->SetIcon(mIconApps);

	// menu bar
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(MY1ID_LOAD, wxT("&Open\tF2"));
	fileMenu->Append(MY1ID_SAVE, wxT("&Save\tF3"));
	fileMenu->Append(MY1ID_NEW, wxT("&Clear\tF4"));
	fileMenu->AppendSeparator();
	fileMenu->Append(MY1ID_EXIT, wxT("E&xit"), wxT("Quit program"));
	wxMenu *editMenu = new wxMenu;
	editMenu->Append(MY1ID_OPTIONS, wxT("&Preferences...\tF8"));
	wxMenu *viewMenu = new wxMenu;
	viewMenu->Append(MY1ID_VIEW_INFOPANE, wxT("View Info Panel"));
	viewMenu->Append(MY1ID_VIEW_LOGSPANE, wxT("View Log Panel"));
	viewMenu->Append(MY1ID_VIEW_MINIMV, wxT("View miniMV Panel"));
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
	// create initial pane for main view
	mNoteBook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE);
	mNoteBook->AddPage(CreateMainPanel(mNoteBook), wxT("Welcome"), true);
	mMainUI.AddPane(mNoteBook, wxAuiPaneInfo().Name(wxT("codeBook")).
		CenterPane().Layer(3).PaneBorder(false));
	// tool bar - file
	mMainUI.AddPane(CreateFileToolBar(), wxAuiPaneInfo().Name(wxT("fileTool")).
		ToolbarPane().Top().Position(TOOL_FILE_POS).Floatable(AUI_GO_FLOAT).
		LeftDockable(false).RightDockable(false).BottomDockable(false));
	// tool bar - edit
	mMainUI.AddPane(CreateEditToolBar(), wxAuiPaneInfo().Name(wxT("editTool")).
		ToolbarPane().Top().Position(TOOL_EDIT_POS).Floatable(AUI_GO_FLOAT).
		LeftDockable(false).RightDockable(false).BottomDockable(false));
	// tool bar - proc
	mMainUI.AddPane(CreateProcToolBar(), wxAuiPaneInfo().Name(wxT("procTool")).
		ToolbarPane().Top().Position(TOOL_PROC_POS).Floatable(AUI_GO_FLOAT).
		LeftDockable(false).RightDockable(false).BottomDockable(false));
	// reg panel
	mMainUI.AddPane(CreateRegsPanel(this), wxAuiPaneInfo().Name(wxT("regsPanel")).
		Caption(wxT("Registers")).DefaultPane().Left().Position(TOOL_REGI_POS).
		Layer(2).Floatable(AUI_GO_FLOAT).
		TopDockable(false).RightDockable(true).BottomDockable(false).
		MinSize(wxSize(INFO_PANEL_WIDTH,0)));
	// info panel
	mMainUI.AddPane(CreateInfoPanel(), wxAuiPaneInfo().Name(wxT("infoPanel")).
		Caption(wxT("Information")).DefaultPane().Left().Position(TOOL_MEMO_POS).
		Layer(2).Floatable(AUI_GO_FLOAT).
		TopDockable(false).RightDockable(false).BottomDockable(false).
		MinSize(wxSize(INFO_PANEL_WIDTH,0)));
	// dev panel
	mMainUI.AddPane(CreateDEVPanel(this), wxAuiPaneInfo().Name(wxT("devsPanel")).
		Caption(wxT("Devices")).DefaultPane().Right().Layer(2).Floatable(AUI_GO_FLOAT).
		TopDockable(false).LeftDockable(true).BottomDockable(false).
		MinSize(wxSize(DEVC_PANEL_WIDTH,0)));
	// simulation panel
	mMainUI.AddPane(CreateSimsPanel(), wxAuiPaneInfo().Name(wxT("simsPanel")).
		Caption(wxT("Simulation")).DefaultPane().Float().
		TopDockable(false).BottomDockable(false).
		LeftDockable(false).RightDockable(false).
		CloseButton(true).Hide());
	// system build panel
	mMainUI.AddPane(CreateBuildPanel(), wxAuiPaneInfo().Name(wxT("buildPanel")).
		Caption(wxT("System Build")).DefaultPane().Float().
		TopDockable(false).BottomDockable(false).
		LeftDockable(false).RightDockable(false).
		CloseButton(false).Hide());
	// log panel
	mMainUI.AddPane(CreateLogsPanel(), wxAuiPaneInfo().Name(wxT("logsPanel")).
		Caption(wxT("Logs Panel")).DefaultPane().Bottom().
		MaximizeButton(true).Position(0).Floatable(AUI_GO_FLOAT).
		TopDockable(false).RightDockable(false).LeftDockable(false).
		MinSize(wxSize(0,CONS_PANEL_HEIGHT)));
	// commit changes!
	mMainUI.Update();

	// actions & events!
	this->Connect(MY1ID_EXIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnQuit));
	this->Connect(MY1ID_NEW, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnNew));
	this->Connect(MY1ID_LOAD, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnLoad));
	this->Connect(MY1ID_SAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnSave));
	this->Connect(MY1ID_ABOUT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnAbout));
	this->Connect(MY1ID_VIEW_INFOPANE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnShowPanel));
	this->Connect(MY1ID_VIEW_LOGSPANE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnShowPanel));
	this->Connect(wxID_ANY, wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler(my1Form::OnClosePane));
	this->Connect(wxID_ANY, wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(my1Form::OnPageChanged));
	this->Connect(wxID_ANY, wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, wxAuiNotebookEventHandler(my1Form::OnPageClosing));
	this->Connect(MY1ID_OPTIONS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnCheckOptions));
	this->Connect(MY1ID_ASSEMBLE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnAssemble));
	this->Connect(MY1ID_SIMULATE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnSimulate));
	this->Connect(MY1ID_GENERATE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnGenerate));
	this->Connect(MY1ID_STAT_TIMER, wxEVT_TIMER, wxTimerEventHandler(my1Form::OnStatusTimer));
	this->Connect(MY1ID_SIMX_TIMER, wxEVT_TIMER, wxTimerEventHandler(my1Form::OnSimExeTimer));
	this->Connect(MY1ID_CONSCOMM, wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(my1Form::OnExecuteConsole));
	this->Connect(MY1ID_CONSEXEC, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnExecuteConsole));
	this->Connect(MY1ID_SIMSEXEC, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnSimulationPick));
	this->Connect(MY1ID_SIMSSTEP, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnSimulationPick));
	this->Connect(MY1ID_SIMSINFO, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnSimulationInfo));
	this->Connect(MY1ID_SIMSBRKP, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnSimulationInfo));
	this->Connect(MY1ID_SIMSEXIT, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnSimulationExit));
	this->Connect(MY1ID_BUILDINIT, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDRST, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDDEF, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDNFO, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDROM, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDRAM, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDPPI, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnBuildSelect));
	this->Connect(MY1ID_BUILDOUT, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnBuildSelect));
	this->Connect(MY1ID_VIEW_MINIMV, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnShowMiniMV));

	// position this!
	//this->Centre();
	this->Maximize();
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
}

void my1Form::CalculateSimCycle(void)
{
	std::clock_t cTime1, cTime2;
	cTime1 = cTime2 = std::clock();
	while(cTime2==cTime1)
		cTime2 = std::clock();
	mSimulationCycleDefault = (cTime2-cTime1);
	mSimulationCycleDefault /= CLOCKS_PER_SEC;
	mSimulationCycle = mSimulationCycleDefault;
}

bool my1Form::ScaleSimCycle(double aScale)
{
	bool cScaled = false;
	double cTest = mSimulationCycle*aScale;
	if(cTest>=mSimulationCycleDefault)
	{
		mSimulationCycle = cTest;
		cScaled = true;
	}
	return cScaled;
}

double my1Form::GetSimCycle(void)
{
	return mSimulationCycle;
}

void my1Form::SimulationMode(bool aGo)
{
	wxMenuBar *cMainMenu = this->GetMenuBar();
	wxAuiToolBar *cFileTool = (wxAuiToolBar*) this->FindWindow(MY1ID_FILETOOL);
	wxAuiToolBar *cEditTool = (wxAuiToolBar*) this->FindWindow(MY1ID_EDITTOOL);
	wxAuiToolBar *cProcTool = (wxAuiToolBar*) this->FindWindow(MY1ID_PROCTOOL);
	mNoteBook->Enable(!aGo);
	cMainMenu->Enable(!aGo);
	cFileTool->Enable(!aGo);
	cEditTool->Enable(!aGo);
	cProcTool->Enable(!aGo);
	wxString cToolName = wxT("simsPanel");
	wxAuiPaneInfo& cPane = mMainUI.GetPane(cToolName);
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
	wxAuiToolBar *cProcTool = (wxAuiToolBar*) this->FindWindow(MY1ID_PROCTOOL);
	mNoteBook->Enable(!aGo);
	cMainMenu->Enable(!aGo);
	cFileTool->Enable(!aGo);
	cEditTool->Enable(!aGo);
	cProcTool->Enable(!aGo);
	wxString cToolName = wxT("buildPanel");
	wxAuiPaneInfo& cPane = mMainUI.GetPane(cToolName);
	if(aGo)
	{
		wxPoint cPoint = this->GetScreenPosition();
		cPane.FloatingPosition(cPoint.x+FLOAT_INIT_X,cPoint.y+FLOAT_INIT_Y);
	}
	cPane.Show(aGo);
	mBuildMode = aGo;
	mMainUI.Update();
}

wxAuiToolBar* my1Form::CreateFileToolBar(void)
{
	wxBitmap mIconExit = MACRO_WXBMP(exit);
	wxBitmap mIconNewd = MACRO_WXBMP(newd);
	wxBitmap mIconLoad = MACRO_WXBMP(open);
	wxBitmap mIconSave = MACRO_WXBMP(save);
	wxAuiToolBar* fileTool = new wxAuiToolBar(this, MY1ID_FILETOOL, wxDefaultPosition,
		wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
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
	wxBitmap mIconOptions = MACRO_WXBMP(option);
	wxBitmap mIconMiniMV = MACRO_WXBMP(target);
	wxAuiToolBar* editTool = new wxAuiToolBar(this, MY1ID_EDITTOOL, wxDefaultPosition,
		wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	editTool->SetToolBitmapSize(wxSize(16,16));
	editTool->AddTool(MY1ID_OPTIONS, wxT("Options"), mIconOptions, wxT("Options"));
	editTool->AddTool(MY1ID_VIEW_MINIMV, wxT("MiniMV"), mIconMiniMV, wxT("Create Mini MemViewer"));
	editTool->Realize();
	return editTool;
}

wxAuiToolBar* my1Form::CreateProcToolBar(void)
{
	wxBitmap mIconAssemble = MACRO_WXBMP(binary);
	wxBitmap mIconSimulate = MACRO_WXBMP(simx);
	wxBitmap mIconGenerate = MACRO_WXBMP(hexgen);
	wxAuiToolBar* procTool = new wxAuiToolBar(this, MY1ID_PROCTOOL, wxDefaultPosition,
		wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	procTool->SetToolBitmapSize(wxSize(16,16));
	procTool->AddTool(MY1ID_ASSEMBLE, wxT("Assemble"), mIconAssemble, wxT("Assemble"));
	procTool->AddTool(MY1ID_SIMULATE, wxT("Simulate"), mIconSimulate, wxT("Simulate"));
	procTool->AddTool(MY1ID_GENERATE, wxT("Generate"), mIconGenerate, wxT("Generate"));
	procTool->Realize();
	procTool->Enable(false); // disabled by default!
	return procTool;
}

wxBoxSizer* my1Form::CreateFLAGView(wxWindow* aParent, const wxString& aString, int anID)
{
	wxString cDefault = wxT("0");
	wxStaticText *cLabel = new wxStaticText(aParent, wxID_ANY, aString);
	wxTextCtrl *cValue = new wxTextCtrl(aParent, wxID_ANY, cDefault,
		wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
	switch(anID)
	{
		default:
		case I8085_FLAG_C:
			mFlagLink[I8085_FIDX_C].SetLink((void*)cValue);
			break;
		case I8085_FLAG_P:
			mFlagLink[I8085_FIDX_P].SetLink((void*)cValue);
			break;
		case I8085_FLAG_A:
			mFlagLink[I8085_FIDX_A].SetLink((void*)cValue);
			break;
		case I8085_FLAG_Z:
			mFlagLink[I8085_FIDX_Z].SetLink((void*)cValue);
			break;
		case I8085_FLAG_S:
			mFlagLink[I8085_FIDX_S].SetLink((void*)cValue);
			break;
	}
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->Add(cLabel,1,wxALIGN_CENTER);
	cBoxSizer->Add(cValue,0,wxALIGN_CENTER);
	return cBoxSizer;
}

wxBoxSizer* my1Form::CreateREGSView(wxWindow* aParent, const wxString& aString, int anID)
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

wxBoxSizer* my1Form::CreateLEDView(wxWindow* aParent, const wxString& aString, int anID)
{
	wxStaticText *cLabel = new wxStaticText(aParent, wxID_ANY, aString);
	my1LEDCtrl *cValue = new my1LEDCtrl(aParent, wxID_ANY);
	my1Device *pDevice = m8085.Device(0);
	if(pDevice)
	{
		my1BitSelect& cLink = cValue->Link();
		cLink.mDevice = 0;
		cLink.mDevicePort = anID/8;
		cLink.mDeviceBit = anID%8;
		my1DevicePort *pPort = pDevice->GetDevicePort(cLink.mDevicePort);
		my1BitIO *pBitIO = pPort->GetBitIO(cLink.mDeviceBit);
		pBitIO->SetLink((void*)cValue);
		pBitIO->DoUpdate = &my1LEDCtrl::DoUpdate;
		cLink.mPointer = (void*) pBitIO;
	}
	cLabel->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(my1LEDCtrl::OnMouseClick),NULL,cValue);
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->AddSpacer(INFO_DEV_SPACER);
	cBoxSizer->Add(cValue,0,wxALIGN_LEFT);
	cBoxSizer->AddSpacer(INFO_DEV_SPACER);
	cBoxSizer->Add(cLabel,1,wxALIGN_CENTER);
	return cBoxSizer;
}

wxBoxSizer* my1Form::CreateSWIView(wxWindow* aParent, const wxString& aString, int anID)
{
	wxStaticText *cLabel = new wxStaticText(aParent, wxID_ANY, aString);
	my1SWICtrl *cValue = new my1SWICtrl(aParent, wxID_ANY);
	my1Device *pDevice = m8085.Device(0);
	if(pDevice)
	{
		my1BitSelect& cLink = cValue->Link();
		cLink.mDevice = 0;
		cLink.mDevicePort = anID/8;
		cLink.mDeviceBit = anID%8;
		my1DevicePort *pPort = pDevice->GetDevicePort(anID/8);
		my1BitIO *pBitIO = pPort->GetBitIO(anID%8);
		pBitIO->SetLink((void*)cValue);
		pBitIO->DoDetect = &my1SWICtrl::DoDetect;
		cLink.mPointer = (void*) pBitIO;
	}
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->AddSpacer(INFO_DEV_SPACER);
	cBoxSizer->Add(cValue,0,wxALIGN_LEFT);
	cBoxSizer->AddSpacer(INFO_DEV_SPACER);
	cBoxSizer->Add(cLabel,1,wxALIGN_CENTER);
	return cBoxSizer;
}

wxPanel* my1Form::CreateMainPanel(wxWindow *parent)
{
	wxFont cFont(TITLE_FONT_SIZE,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	wxFont dFont(EMAIL_FONT_SIZE,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	wxPanel *cPanel = new wxPanel(parent, wxID_ANY);
	wxStaticText *cLabel = new wxStaticText(cPanel, wxID_ANY, wxT("MY1 Sim85"));
	cLabel->SetFont(cFont);
	wxButton *cButtonBuild = new wxButton(cPanel, MY1ID_BUILDINIT, wxT("BUILD SYSTEM"),
		wxDefaultPosition, wxDefaultSize);
	cButtonBuild->SetFont(dFont);
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->Add(cLabel,1,wxALIGN_CENTER|wxALIGN_BOTTOM);
	wxBoxSizer *eBoxSizer = new wxBoxSizer(wxVERTICAL);
	eBoxSizer->Add(cBoxSizer,1,wxALIGN_CENTRE);
	eBoxSizer->Add(cButtonBuild,0,wxALIGN_CENTRE|wxALIGN_TOP);
	eBoxSizer->AddStretchSpacer();
	wxStaticText *dLabel = new wxStaticText(cPanel, wxID_ANY, wxT("by azman@my1matrix.net"));
	dLabel->SetFont(dFont);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	pBoxSizer->Add(eBoxSizer,1,wxALIGN_CENTRE);
	pBoxSizer->Add(dLabel,0,wxALIGN_BOTTOM|wxALIGN_RIGHT);
	cPanel->SetSizerAndFit(pBoxSizer);
	pBoxSizer->SetSizeHints(cPanel);
	return cPanel;
}

wxPanel* my1Form::CreateRegsPanel(wxWindow* aParent)
{
	wxPanel *cPanel = new wxPanel(aParent, wxID_ANY);
	wxFont cFont(8,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	pBoxSizer->Add(CreateREGSView(cPanel,wxT("Register B"),I8085_REG_B),0,wxEXPAND);
	pBoxSizer->Add(CreateREGSView(cPanel,wxT("Register C"),I8085_REG_C),0,wxEXPAND);
	pBoxSizer->Add(CreateREGSView(cPanel,wxT("Register D"),I8085_REG_D),0,wxEXPAND);
	pBoxSizer->Add(CreateREGSView(cPanel,wxT("Register E"),I8085_REG_E),0,wxEXPAND);
	pBoxSizer->Add(CreateREGSView(cPanel,wxT("Register H"),I8085_REG_H),0,wxEXPAND);
	pBoxSizer->Add(CreateREGSView(cPanel,wxT("Register L"),I8085_REG_L),0,wxEXPAND);
	pBoxSizer->Add(CreateREGSView(cPanel,wxT("Register A"),I8085_REG_A),0,wxEXPAND);
	pBoxSizer->Add(CreateREGSView(cPanel,wxT("Register F"),I8085_REG_F),0,wxEXPAND);
	pBoxSizer->Add(CreateREGSView(cPanel,wxT("Program Counter"),I8085_RP_PC+I8085_REG_COUNT),0,wxEXPAND);
	pBoxSizer->Add(CreateREGSView(cPanel,wxT("Stack Pointer"),I8085_RP_SP+I8085_REG_COUNT),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_REG_SPACER);
	pBoxSizer->Add(CreateFLAGView(cPanel,wxT("CY Flag"),I8085_FLAG_C),0,wxEXPAND);
	pBoxSizer->Add(CreateFLAGView(cPanel,wxT("Parity Flag"),I8085_FLAG_P),0,wxEXPAND);
	pBoxSizer->Add(CreateFLAGView(cPanel,wxT("AC Flag"),I8085_FLAG_A),0,wxEXPAND);
	pBoxSizer->Add(CreateFLAGView(cPanel,wxT("Zero Flag"),I8085_FLAG_Z),0,wxEXPAND);
	pBoxSizer->Add(CreateFLAGView(cPanel,wxT("Sign Flag"),I8085_FLAG_S),0,wxEXPAND);
	cPanel->SetSizerAndFit(pBoxSizer);
	return cPanel;
}

wxPanel* my1Form::CreateInfoPanel(void)
{
	wxPanel *cPanel = new wxPanel(this,MY1ID_INFOPANEL);
	cPanel->SetMinSize(wxSize(INFO_PANEL_WIDTH,0));
	wxFont cFont(INFO_FONT_SIZE,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxNotebook *cInfoBook = new wxNotebook(cPanel,MY1ID_LOGBOOK);
	cInfoBook->AddPage(CreateMEMPanel(cInfoBook),wxT("Memory"),true);
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->Add(cInfoBook,1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
	cPanel->SetSizer(cBoxSizer);
	cBoxSizer->SetSizeHints(cPanel);
	return cPanel;
}

wxPanel* my1Form::CreateSimsPanel(void)
{
	wxPanel *cPanel = new wxPanel(this, MY1ID_SIMSPANEL,
		wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxButton *cButtonStep = new wxButton(cPanel, MY1ID_SIMSSTEP, wxT("Step"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonExec = new wxButton(cPanel, MY1ID_SIMSEXEC, wxT("Run"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonInfo = new wxButton(cPanel, MY1ID_SIMSINFO, wxT("Info"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonBRKP = new wxButton(cPanel, MY1ID_SIMSBRKP, wxT("Break"),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButtonExit = new wxButton(cPanel, MY1ID_SIMSEXIT, wxT("Exit"),
		wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxVERTICAL);
	cBoxSizer->Add(cButtonStep, 0, wxALIGN_TOP);
	cBoxSizer->Add(cButtonExec, 0, wxALIGN_TOP);
	cBoxSizer->Add(cButtonInfo, 0, wxALIGN_TOP);
	cBoxSizer->Add(cButtonBRKP, 0, wxALIGN_TOP);
	cBoxSizer->Add(cButtonExit, 0, wxALIGN_TOP);
	cPanel->SetSizer(cBoxSizer);
	cBoxSizer->SetSizeHints(cPanel);
	return cPanel;
}

wxPanel* my1Form::CreateBuildPanel(void)
{
	wxPanel *cPanel = new wxPanel(this, MY1ID_BUILDPANEL,
		wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
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
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxVERTICAL);
	cBoxSizer->Add(cButtonRST, 0, wxALIGN_TOP);
	cBoxSizer->Add(cButtonDEF, 0, wxALIGN_TOP);
	cBoxSizer->Add(cButtonNFO, 0, wxALIGN_TOP);
	cBoxSizer->Add(cButtonROM, 0, wxALIGN_TOP);
	cBoxSizer->Add(cButtonRAM, 0, wxALIGN_TOP);
	cBoxSizer->Add(cButtonPPI, 0, wxALIGN_TOP);
	cBoxSizer->Add(cButtonOUT, 0, wxALIGN_TOP);
	cPanel->SetSizer(cBoxSizer);
	cBoxSizer->SetSizeHints(cPanel);
	return cPanel;
}

wxPanel* my1Form::CreateLogsPanel(void)
{
	wxPanel *cPanel = new wxPanel(this, MY1ID_INFOPANEL,
		wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxFont cTestFont(LOGS_FONT_SIZE,wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cTestFont);
	// duh?!
	cPanel->SetMinSize(wxSize(INFO_PANEL_WIDTH,0));
	// main view - logbook
	wxNotebook *cLogBook = new wxNotebook(cPanel, MY1ID_LOGBOOK, wxDefaultPosition, wxDefaultSize, wxNB_LEFT);
	// main page - console
	wxPanel *cConsPanel = new wxPanel(cLogBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxTextCtrl *cConsole = new wxTextCtrl(cConsPanel, wxID_ANY, wxT("Welcome to MY1Sim85\n"),
		wxDefaultPosition, wxDefaultSize, wxTE_AUTO_SCROLL|wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator);
	wxPanel *cComsPanel = new wxPanel(cConsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxTextCtrl *cCommandText = new wxTextCtrl(cComsPanel, MY1ID_CONSCOMM, wxT(""), wxDefaultPosition, wxDefaultSize,wxTE_PROCESS_ENTER);
	wxButton *cButton = new wxButton(cComsPanel, MY1ID_CONSEXEC, wxT("Execute"));
	wxBoxSizer *dBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	dBoxSizer->Add(cCommandText, 1, wxEXPAND);
	dBoxSizer->Add(cButton, 0, wxALIGN_RIGHT);
	cComsPanel->SetSizer(dBoxSizer);
	dBoxSizer->Fit(cComsPanel);
	dBoxSizer->SetSizeHints(cComsPanel);
	wxBoxSizer *eBoxSizer = new wxBoxSizer(wxVERTICAL);
	eBoxSizer->Add(cConsole, 1, wxEXPAND);
	eBoxSizer->Add(cComsPanel, 0, wxALIGN_BOTTOM|wxEXPAND);
	cConsPanel->SetSizer(eBoxSizer);
	eBoxSizer->Fit(cConsPanel);
	eBoxSizer->SetSizeHints(cConsPanel);
	// add the pages
	cLogBook->AddPage(cConsPanel, wxT("Console"), true);
	// 'remember' main console
	if(!mConsole) mConsole = cConsole;
	if(!mCommand) mCommand = cCommandText;
	// main box-sizer
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->Add(cLogBook, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
	cPanel->SetSizer(cBoxSizer);
	cBoxSizer->SetSizeHints(cPanel);
	// return wxpanel object
	return cPanel;
}

wxPanel* my1Form::CreateDEVPanel(wxWindow* aParent)
{
	wxPanel *cPanel = new wxPanel(aParent, wxID_ANY);
	wxFont cFont(8,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED"),I8255_PIN_PA0),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED"),I8255_PIN_PA1),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED"),I8255_PIN_PA2),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED"),I8255_PIN_PA3),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED"),I8255_PIN_PA4),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED"),I8255_PIN_PA5),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED"),I8255_PIN_PA6),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED"),I8255_PIN_PA7),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI"),I8255_PIN_PB0),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI"),I8255_PIN_PB1),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI"),I8255_PIN_PB2),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI"),I8255_PIN_PB3),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI"),I8255_PIN_PB4),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI"),I8255_PIN_PB5),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI"),I8255_PIN_PB6),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI"),I8255_PIN_PB7),0,wxEXPAND);
	cPanel->SetSizerAndFit(pBoxSizer);
	return cPanel;
}

#define MEM_VIEW_WIDTH 8
#define MEM_VIEW_HEIGHT (MAX_MEMSIZE/MEM_VIEW_WIDTH)

wxPanel* my1Form::CreateMVGPanel(wxWindow* aParent, int aStart, int aHeight, wxGrid** ppGrid)
{
	wxPanel *cPanel = new wxPanel(aParent, wxID_ANY);
	wxFont cFont(8,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	wxGrid *pGrid = new wxGrid(cPanel, wxID_ANY);
	pGrid->CreateGrid(aHeight,MEM_VIEW_WIDTH);
	pGrid->SetFont(cFont);
	pGrid->SetLabelFont(cFont);
	//pGrid->UseNativeColHeader();
	pGrid->SetRowLabelAlignment(wxALIGN_CENTRE,wxALIGN_CENTRE);
	pGrid->SetColLabelAlignment(wxALIGN_CENTRE,wxALIGN_CENTRE);
	pGrid->SetDefaultCellAlignment(wxALIGN_CENTRE,wxALIGN_CENTRE);
	for(int cRow=0;cRow<aHeight;cRow++)
		pGrid->SetRowLabelValue(cRow,
			wxString::Format(wxT("%04X"),aStart+cRow*MEM_VIEW_WIDTH));
	for(int cCol=0;cCol<MEM_VIEW_WIDTH;cCol++)
		pGrid->SetColLabelValue(cCol,wxString::Format(wxT("%02X"),cCol));
	for(int cRow=0;cRow<aHeight;cRow++)
		for(int cCol=0;cCol<MEM_VIEW_WIDTH;cCol++)
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

wxPanel* my1Form::CreateMEMPanel(wxWindow* aParent)
{
	wxGrid *pGrid = 0x0;
	wxPanel *cPanel = CreateMVGPanel(aParent,0x0000,MEM_VIEW_HEIGHT,&pGrid);
	my1Memory *pMemory = m8085.Memory(0);
	while(pMemory)
	{
		pMemory->SetLink((void*)pGrid);
		pMemory->DoUpdate = &this->SimUpdateMEM;
		pMemory = (my1Memory*) pMemory->Next();
	}
	return cPanel;
}

void my1Form::OpenEdit(wxString& cFileName)
{
	my1CodeEdit *cCodeEdit = new my1CodeEdit(mNoteBook, wxID_ANY, cFileName, this->mOptions);
	wxString cTempFile = cCodeEdit->GetFileName();
	if(!cTempFile.Length())
		cTempFile = wxT("unnamed");
	mNoteBook->AddPage(cCodeEdit, cTempFile,true);
	if(mOptions.mConv_UnixEOL)
		cCodeEdit->ConvertEOLs(2);
	wxString cStatus = wxT("File ") + cCodeEdit->GetFileName() + wxT(" loaded!");
	this->ShowStatus(cStatus);
}

void my1Form::SaveEdit(wxWindow* cEditPane)
{
	wxString cFileName;
	my1CodeEdit *cEditor = (my1CodeEdit*) cEditPane;
	if(!cEditor->GetFileName().Length())
	{
		wxFileDialog *cSelect = new wxFileDialog(this,wxT("Assign File Name"),
			wxT(""),wxT(""),wxT("Any file (*.*)|*.*"),
			wxFD_SAVE|wxFD_OVERWRITE_PROMPT|wxFD_CHANGE_DIR);
		cSelect->SetWildcard("ASM files (*.asm)|*.asm|8085 ASM files (*.8085)|*.8085|Any file (*.*)|*.*");
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

void my1Form::DisconnectAllMemory(void)
{
	my1Memory *pMemory = m8085.Memory(0);
	while(pMemory)
	{
		pMemory->DoUpdate = 0x0;
		pMemory->DoDetect = 0x0;
		pMemory = (my1Memory*) pMemory->Next();
	}
}

void my1Form::DisconnectAllDevice(void)
{
	my1Device *pDevice = m8085.Device(0);
	while(pDevice)
	{
		// for now ALWAYS an 8255 PPI!
		for(int cPort=0;cPort<I8255_SIZE-1;cPort++)
		{
			my1DevicePort *pPort = pDevice->GetDevicePort(cPort);
			for(int cBit=0;cBit<I8255_DATASIZE;cBit++)
			{
				my1BitIO *pBitIO = pPort->GetBitIO(cBit);
				pBitIO->SetLink(0x0);
				pBitIO->DoUpdate = 0x0;
				pBitIO->DoDetect = 0x0;
			}
			pPort->DoUpdate = 0x0;
			pPort->DoDetect = 0x0;
		}
		pDevice->DoUpdate = 0x0;
		pDevice->DoDetect = 0x0;
		pDevice = (my1Device*) pDevice->Next();
	}
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
	cSelect->SetWildcard("ASM files (*.asm)|*.asm|8085 ASM files (*.8085)|*.8085|Any file (*.*)|*.*");
	if(cSelect->ShowModal()!=wxID_OK) return;
	wxString cFileName = cSelect->GetPath();
	this->OpenEdit(cFileName);
}

void my1Form::OnSave(wxCommandEvent& WXUNUSED(event))
{
	int cSelect = mNoteBook->GetSelection();
	if(cSelect<0) return;
	wxWindow *cTarget = mNoteBook->GetPage(cSelect);
	if(!cTarget->IsKindOf(CLASSINFO(my1CodeEdit))) return;
	this->SaveEdit(cTarget);
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
	wxStreamToTextRedirector cRedirect(mConsole);
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
		m8085.SetStartAddress(mOptions.mSims_StartADDR);
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
	wxStreamToTextRedirector cRedirect(mConsole);
	wxString cStatus = wxT("Preparing ") + cEditor->GetFileName() + wxT("...");
	if(m8085.Simulate(1,true)) // force a reset!
	{
		cStatus = wxT("[SUCCESS] Ready for Simulation!");
		this->ShowStatus(cStatus);
		this->SimulationMode();
		if(!mOptions.mSims_FreeRunning)
			cEditor->ExecLine(m8085.GetCodexLine()-1);
		mCommand->SetFocus();
	}
	else
	{
		cStatus = wxT("[ERROR] Check start address?");
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
	wxStreamToTextRedirector cRedirect(mConsole);
	wxString cStatus = wxT("Processing ") + cEditor->GetFileName() + wxT("...");
	wxString cFileHEX = cEditor->GetFileNoXT() + wxT(".HEX");
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

void my1Form::PrintMemoryContent(aword anAddress, int aSize)
{
	aword cAddress = anAddress;
	abyte cData;
	int cCount = 0;
	std::cout << "\n";
	while(cCount<aSize&&cAddress<MAX_MEMSIZE)
	{
		if(!m8085.MemoryMap().Read(cAddress,cData))
		{
			std::cout << "\n[R/W ERROR] Cannot read from address 0x"
				<< std::setw(4) << std::setfill('0')
				<< std::setbase(16) << cAddress << "\n";
			break;
		}
		if(cCount%PRINT_BPL_COUNT==0)
		{
			std::cout << "\n| " << std::setw(4) << std::setfill('0')
				<< std::setbase(16) << cAddress << " | ";
		}
		// print data!
		std::cout << std::setw(2) << std::setfill('0')
			<< std::setbase(16) << (int) cData << " | ";
		cCount++; cAddress++;
	}
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
		std::cout << "Start: 0x" << std::setw(4) << std::setfill('0') << std::setbase(16) << cMemory->GetStart() << ", ";
		std::cout << "Size: 0x" << std::setw(4) << std::setfill('0') << std::setbase(16) << cMemory->GetSize() << "\n";
		cMemory = (my1Memory*) cMemory->Next();
	}
	std::cout << "Device Count: " << m8085.DeviceMap().GetCount() << "\n";
	my1Device* cDevice = m8085.Device(0);
	while(cDevice)
	{
		std::cout << "(Device) Name: " << cDevice->GetName() << ", ";
		std::cout << "Start: 0x" << std::setw(2) << std::setfill('0') << cDevice->GetStart() << ", ";
		std::cout << "Size: 0x" << std::setw(2) << std::setfill('0') << std::setbase(16) << cDevice->GetSize() << "\n";
		cDevice = (my1Device*) cDevice->Next();
	}
}

void my1Form::PrintSimInfo(void)
{
	std::cout << "\nSimulation Info";
	std::cout << ": CLOCKS_PER_SEC=" << std::setbase(10) << CLOCKS_PER_SEC;
	std::cout << ", SimCycleDefault=" << mSimulationCycleDefault;
	std::cout << ", SimCycle=" << mSimulationCycle << "\n";
}

void my1Form::PrintConsoleMessage(const wxString& aMessage)
{
	std::cout << "\n" << aMessage << "\n";
}

void my1Form::PrintSimChangeStart(unsigned long aStart, bool anError)
{
	std::cout << "\n";
	if(anError)
	{
		std::cout << "[ERROR] Cannot Change Simulation Start Addr\n";
		return;
	}
	std::cout << "Simulation Start Addr Changed to ";
	std::cout << std::setw(4) << std::setfill('0') << std::hex << aStart << "\n";
}

void my1Form::PrintBuildAdd(const wxString& aMessage, unsigned long aStart)
{
	std::cout << "\n@[" << std::setw(4) << std::setfill('0')
		<< std::setbase(16) << aStart << "] : " << aMessage << "\n";
}

void my1Form::PrintHelp(void)
{
	std::cout << "\nAvailable command(s):" << "\n";
	std::cout << "- show [system|mem=?]" << "\n";
	std::cout << "  > system (print system info)" << "\n";
	std::cout << "  > mem=? (show memory starting from given addr)" << "\n";
	std::cout << "- sim [info|addr=?|mark=?]" << "\n";
	std::cout << "  > info (simulation timing info)" << "\n";
	std::cout << "  > addr=? (set simulation start addr)" << "\n";
	std::cout << "  > mark=? (show/hide line marker)" << "\n";
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

void my1Form::PrintUnknownParameter(const wxString& aParam, const wxString& aCommand)
{
	std::cout << "\nUnknown parameter '" << aParam << "' for [" << aCommand << "]\n";
}

void my1Form::OnExecuteConsole(wxCommandEvent &event)
{
	wxString cCommandLine = mCommand->GetLineText(0);
	mCommand->SelectAll(); mCommand->Cut();
	if(!cCommandLine.Length())
	{
		mConsole->AppendText("\n");
		return;
	}
	wxString cCommandWord = cCommandLine.BeforeFirst(' ');
	wxString cParameters = cCommandLine.AfterFirst(' ');
	wxStreamToTextRedirector cRedirect(mConsole);
	if(!cCommandWord.Cmp(wxT("show")))
	{
		wxString cParam = cParameters.BeforeFirst(' ');
		int cEqual = cParam.Find('=');
		if(cEqual==wxNOT_FOUND)
		{
			if(!cParam.Cmp(wxT("system")))
			{
				this->PrintPeripheralInfo();
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
					this->PrintSimChangeStart(cStart);
				}
				else
				{
					this->PrintSimChangeStart(cStart,true);
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
					this->PrintConsoleMessage("Sim Marker: ON!");
				}
				else
				{
					mOptions.mSims_FreeRunning = true;
					this->PrintConsoleMessage("Sim Marker: OFF!");
				}
			}
			else if(!cKey.Cmp(wxT("break")))
			{
				if(!mSimulationMode)
				{
					this->PrintConsoleMessage("This feature is only available in simulation mode!");
					return;
				}
				my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
				if(!cEditor)
				{
					this->PrintConsoleMessage("[BREAK ERROR] Cannot get editor link!");
					return;
				}
				unsigned long cStart;
				if(cValue.ToULong(&cStart,16)&&(int)cStart<=cEditor->GetLineCount())
				{
					cEditor->ToggleBreak(cStart);
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
			this->PrintConsoleMessage("Build mode disabled during simulation!");
			return;
		}
		wxString cParam = cParameters.BeforeFirst(' ');
		int cEqual = cParam.Find('=');
		if(cEqual==wxNOT_FOUND)
		{
			if(!cParam.Cmp(wxT("info")))
			{
				this->PrintPeripheralInfo();
			}
			else if(!cParam.Cmp(wxT("default")))
			{
				this->SystemDefault();
			}
			else if(!cParam.Cmp(wxT("reset")))
			{
				this->SystemReset();
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
			}
			else if(!cKey.Cmp(wxT("ram")))
			{
				unsigned long cStart;
				if(cValue.ToULong(&cStart,16)&&cStart<=0xFFFF)
					this->AddRAM(cStart);
				else
					this->PrintUnknownParameter(cValue,cKey);
			}
			else if(!cKey.Cmp(wxT("ppi")))
			{
				unsigned long cStart;
				if(cValue.ToULong(&cStart,16)&&cStart<=0xFF)
					this->AddPPI(cStart);
				else
					this->PrintUnknownParameter(cValue,cKey);
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
	}
	else
	{
		this->PrintUnknownCommand(cCommandWord);
	}
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
		wxStreamToTextRedirector cRedirect(mConsole);
		m8085.PrintCodexInfo();
	}
	else if(event.GetId()==MY1ID_SIMSBRKP)
	{
		my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
		if(!cEditor)
		{
			wxStreamToTextRedirector cRedirect(mConsole);
			this->PrintConsoleMessage("[BREAK ERROR] Cannot get editor link!");
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
		mSimulationRunning = false;
		mSimulationStepping = false;
		this->SimulationMode(false);
	}
}

int my1Form::GetBuildAddress(const wxString& aString)
{
	wxTextEntryDialog* cDialog = new wxTextEntryDialog(this,
		wxT("Enter Address in HEX"), aString);
	if(cDialog->ShowModal()==wxCANCEL)
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
			{
				wxStreamToTextRedirector cRedirect(mConsole);
				this->PrintPeripheralInfo();
			}
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
		case MY1ID_VIEW_INFOPANE:
			cToolName = wxT("infoPanel");
			break;
		case MY1ID_VIEW_LOGSPANE:
			cToolName = wxT("logsPanel");
			break;
	}
	wxAuiPaneInfo& cPane = mMainUI.GetPane(cToolName);
	if(cPane.IsOk())
	{
		cPane.Show();
		mMainUI.Update();
	}
	return;
}

#define MEM_MINIVIEW_HEIGHT 4

void my1Form::OnShowMiniMV(wxCommandEvent &event)
{
	int cAddress = this->GetBuildAddress(wxT("Start Address for Viewer"));
	if(cAddress<0) return;
	if(cAddress%8!=0)
	{
		wxMessageBox(wxT("Address must be in multiples of 8!"),
			wxT("Invalid Address!"),wxOK|wxICON_EXCLAMATION);
	}
	my1Memory* pMemory = (my1Memory*) m8085.MemoryMap().Object((aword)cAddress);
	if(!pMemory)
	{
		wxMessageBox(wxT("No memory object at that address!"),
			wxT("Invalid Address!"),wxOK|wxICON_EXCLAMATION);
		return;
	}
	wxString cPanelName = wxT("miniMV") + wxString::Format(wxT("%04X"),cAddress);
	wxAuiPaneInfo& cPane = mMainUI.GetPane(cPanelName);
	if(cPane.IsOk())
	{
		cPane.Show();
		mMainUI.Update();
		return;
	}
	my1MiniViewer *pViewer = new my1MiniViewer;
	wxGrid* pGrid = 0x0;
	wxPanel* cPanel = CreateMVGPanel(this,cAddress,MEM_MINIVIEW_HEIGHT,&pGrid);
	// update grid?
	aword cStart = cAddress;
	abyte cData;
	for(int cRow=0;cRow<MEM_MINIVIEW_HEIGHT;cRow++)
	{
		for(int cCol=0;cCol<MEM_VIEW_WIDTH;cCol++)
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
}

void my1Form::OnCheckOptions(wxCommandEvent &event)
{
	my1OptionDialog *prefDialog = new my1OptionDialog(this, wxT("Options"), this->mOptions);
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
	wxStreamToTextRedirector cRedirect(mConsole);
	if(m8085.Simulate())
	{
		my1CodeEdit *cEditor = (my1CodeEdit*) m8085.GetCodeLink();
		if(!mOptions.mSims_FreeRunning)
			cEditor->ExecLine(m8085.GetCodexLine()-1);
		else
			cEditor->ExecDone();
		if(cEditor->IsBreakLine())
			mSimulationStepping = true;
	}
	else
	{
		wxMessageBox(wxT("Simulation Terminated!"),wxT("[SIM Error]"));
		mSimulationRunning = false;
		this->SimulationMode(false);
	}
	if(mSimulationRunning&&!mSimulationStepping)
		mSimExecTimer->Start(SIM_EXEC_PERIOD,wxTIMER_ONE_SHOT);
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
}

my1BitIO* my1Form::GetDeviceBit(my1BitSelect& aSelect)
{
	my1BitIO *pBit = 0x0;
	my1Device *pDevice = m8085.Device(aSelect.mDevice);
	if(pDevice)
	{
		my1DevicePort *pPort = pDevice->GetDevicePort(aSelect.mDevicePort);
		if(pPort) pBit = pPort->GetBitIO(aSelect.mDeviceBit);
		if(pBit) aSelect.mPointer = (void*) pBit;
	}
	return pBit;
}

bool my1Form::UnlinkDeviceBit(my1BitIO* aBit)
{
	bool cFound = false;
	wxWindow *cTarget = (wxWindow*) aBit->GetLink();
	if(!cTarget) return true;
	if(cTarget->IsKindOf(CLASSINFO(my1LEDCtrl)))
	{
		my1LEDCtrl* pLED = (my1LEDCtrl*) cTarget;
		if(pLED->Link().mPointer==(void*)aBit)
		{
			cFound = true;
			pLED->Link().mPointer = 0x0;
		}
	}
	else if(cTarget->IsKindOf(CLASSINFO(my1SWICtrl)))
	{
		my1SWICtrl* pSWI = (my1SWICtrl*) cTarget;
		if(pSWI->Link().mPointer==(void*)aBit)
		{
			cFound = true;
			pSWI->Link().mPointer = 0x0;
		}
	}
	aBit->SetLink(0x0);
	aBit->DoUpdate = 0x0;
	aBit->DoDetect = 0x0;
	return cFound;
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
		int cPotID = MY1ID_DSEL_OFFSET+MY1ID_PORT_OFFSET;
		int cBitID = MY1ID_DSEL_OFFSET+MY1ID_DBIT_OFFSET;
		my1Device *pDevice = m8085.Device(0);
		while(pDevice)
		{
			wxMenu *cMenuPort = new wxMenu;
			for(int cPort=0;cPort<I8255_SIZE-1;cPort++)
			{
				wxMenu *cMenuBit = new wxMenu;
				wxString cPortText = wxT("P") +
					wxString::Format(wxT("%c"),(char)(cPort+(int)'A'));
				for(int cLoop=0;cLoop<I8255_DATASIZE;cLoop++)
				{
					wxString cText = cPortText +
						wxString::Format(wxT("%01X"),cLoop);
					cMenuBit->Append(cBitID++,cText,
						wxEmptyString,wxITEM_CHECK);
				}
				wxString cText = wxT("Port ") +
					wxString::Format(wxT("%c"),(char)(cPort+(int)'A'));
				cMenuPort->Append(cPotID++, cText, cMenuBit);
			}
			wxString cText = wxT("Device @") +
				wxString::Format(wxT("%02X"),pDevice->GetStart());
			mDevicePopupMenu->Append(cDevID++, cText, cMenuPort);
			pDevice = (my1Device*) pDevice->Next();
		}
	}
	else
	{
		// make sure all items are unchecked?
		int cCountD = mDevicePopupMenu->GetMenuItemCount();
		for(int cLoopD=0;cLoopD<cCountD;cLoopD++)
		{
			wxMenuItem *cItemD = mDevicePopupMenu->FindItemByPosition(cLoopD);
			wxMenu *cMenuD = cItemD->GetSubMenu();
			int cCountP = cMenuD->GetMenuItemCount();
			for(int cLoopP=0;cLoopP<cCountP;cLoopP++)
			{
				wxMenuItem *cItemP = cMenuD->FindItemByPosition(cLoopP);
				wxMenu *cMenuP = cItemP->GetSubMenu();
				int cCountB = cMenuP->GetMenuItemCount();
				for(int cLoopB=0;cLoopB<cCountB;cLoopB++)
				{
					wxMenuItem *cItem = cMenuP->FindItemByPosition(cLoopB);
					cItem->Check(false);
				}
			}
		}
	}
	return mDevicePopupMenu;
}

void my1Form::ResetDevicePopupMenu(void)
{
	if(mDevicePopupMenu)
	{
		delete mDevicePopupMenu;
		mDevicePopupMenu = 0x0;
	}
}

void my1Form::SimUpdateFLAG(void* simObject)
{
	// update flag if necessary?
	my1Reg85 *pReg85 = (my1Reg85*) simObject;
	wxString cFlag = wxString::Format(wxT("%01X"),
			pReg85->GetData()&I8085_FLAG_C?1:0);
	wxTextCtrl *pText = (wxTextCtrl*) mFlagLink[I8085_FIDX_C].GetLink();
	pText->ChangeValue(cFlag);
	cFlag = wxString::Format(wxT("%01X"),
			pReg85->GetData()&I8085_FLAG_P?1:0);
	pText = (wxTextCtrl*) mFlagLink[I8085_FIDX_P].GetLink();
	pText->ChangeValue(cFlag);
	cFlag = wxString::Format(wxT("%01X"),
			pReg85->GetData()&I8085_FLAG_A?1:0);
	pText = (wxTextCtrl*) mFlagLink[I8085_FIDX_A].GetLink();
	pText->ChangeValue(cFlag);
	cFlag = wxString::Format(wxT("%01X"),
			pReg85->GetData()&I8085_FLAG_Z?1:0);
	pText = (wxTextCtrl*) mFlagLink[I8085_FIDX_Z].GetLink();
	pText->ChangeValue(cFlag);
	cFlag = wxString::Format(wxT("%01X"),
			pReg85->GetData()&I8085_FLAG_S?1:0);
	pText = (wxTextCtrl*) mFlagLink[I8085_FIDX_S].GetLink();
	pText->ChangeValue(cFlag);
}

bool my1Form::SystemDefault(void)
{
	wxStreamToTextRedirector cRedirect(mConsole);
	bool cFlag = m8085.BuildDefault();
	if(cFlag)
	{
		this->PrintConsoleMessage("Default system built!");
		this->PrintPeripheralInfo();
	}
	else
	{
		this->PrintConsoleMessage("Default system build FAILED!");
	}
	this->ResetDevicePopupMenu();
	return cFlag;
}

bool my1Form::SystemReset(void)
{
	wxStreamToTextRedirector cRedirect(mConsole);
	my1Device *pDevice = m8085.Device(0);
	while(pDevice)
	{
		for(int cPort=0;cPort<I8255_SIZE-1;cPort++)
		{
			my1DevicePort *pPort = pDevice->GetDevicePort(cPort);
			for(int cLoop=0;cLoop<I8255_DATASIZE;cLoop++)
			{
				my1BitIO *pBitIO = pPort->GetBitIO(cLoop);
				this->UnlinkDeviceBit(pBitIO);
			}
		}
		pDevice = (my1Device*) pDevice->Next();
	}
	this->ResetDevicePopupMenu();
	bool cFlag = m8085.BuildReset();
	if(cFlag)
		this->PrintConsoleMessage("System build reset!");
	else
		this->PrintConsoleMessage("System build reset FAILED!");
	return cFlag;
}

bool my1Form::AddROM(int aStart)
{
	bool cFlag = false;
	wxStreamToTextRedirector cRedirect(mConsole);
	if((cFlag=m8085.AddROM(aStart)))
		this->PrintBuildAdd(wxT("2764 ROM added!"),aStart);
	else
		this->PrintBuildAdd(wxT("FAILED to add 2764 ROM!"),aStart);
	return cFlag;
}

bool my1Form::AddRAM(int aStart)
{
	bool cFlag = false;
	wxStreamToTextRedirector cRedirect(mConsole);
	if((cFlag=m8085.AddRAM(aStart)))
		this->PrintBuildAdd(wxT("6264 RAM added!"),aStart);
	else
		this->PrintBuildAdd(wxT("FAILED to add 6264 RAM!"),aStart);
	return cFlag;
}

bool my1Form::AddPPI(int aStart)
{
	wxStreamToTextRedirector cRedirect(mConsole);
	bool cFlag = m8085.AddPPI(aStart);
	if(cFlag)
		this->PrintBuildAdd(wxT("8255 PPI added!"),aStart);
	else
		this->PrintBuildAdd(wxT("FAILED to add 8255 PPI!"),aStart);
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
	int cAddress = pMemory->GetLastUsed();
	int cCol = cAddress%MEM_VIEW_WIDTH;
	int cRow = cAddress/MEM_VIEW_WIDTH;
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
			cCol = cIndex%MEM_VIEW_WIDTH;
			cRow = cIndex/MEM_VIEW_WIDTH;
			pViewer->pGrid->SetCellValue(cRow,cCol,
				wxString::Format(wxT("%02X"),cData));
		}
		pViewer = pViewer->mNext;
	}
}

void my1Form::SimDoUpdate(void* simObject)
{
	// microprocessor level update?
}

void my1Form::SimDoDelay(void* simObject, int aCount)
{
/*
	my1Sim85* mySim = (my1Sim85*) simObject;
	my1Form* myForm = (my1Form*) mySim->GetLink();
	std::clock_t cTime1, cTime2;
	cTime1 = cTime2 = std::clock();
	double cTest, cTotal = myForm->GetSimCycle()*aCount;
	do
	{
		cTime2 = std::clock();
		cTest = (double) (cTime2-cTime1) / CLOCKS_PER_SEC;
	}
	while(cTest<cTotal);
*/
	wxMicroSleep(aCount);
}
