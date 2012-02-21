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
#include <wx/aboutdlg.h>

#define MACRO_WXBMP(bmp) wxBitmap(bmp##_xpm)
#define MACRO_WXICO(bmp) wxIcon(bmp##_xpm)

#include "../res/apps.xpm"
#include "../res/exit.xpm"
#include "../res/newd.xpm"
#include "../res/open.xpm"
#include "../res/save.xpm"
#include "../res/binary.xpm"
#include "../res/option.xpm"

#include <iostream>
#include <iomanip>
#include <ctime>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define INFO_PANEL_WIDTH 200
#define CONS_PANEL_HEIGHT 100
#define INFO_DEV_SPACER 5
#define STATUS_COUNT 2
#define STATUS_FIX_WIDTH INFO_PANEL_WIDTH
#define STATUS_MSG_INDEX 1
#define STATUS_MSG_PERIOD 3000
#define SIM_START_ADDR 0x2000

my1Form::my1Form(const wxString &title)
	: wxFrame( NULL, MY1ID_MAIN, title, wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_FRAME_STYLE), m8085(true)
{
	// simulation stuffs
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
	mOptions.mSims_FreeRunning = true;
	mOptions.mSims_StartADDR = SIM_START_ADDR;

	// assign function pointers :p
	//m8085.DoUpdate = &this->SimDoUpdate;
	//m8085.DoDelay = &this->SimDoDelay;

	// minimum window size... duh!
	this->SetMinSize(wxSize(WIN_WIDTH,WIN_HEIGHT));

	// status bar
	this->CreateStatusBar(STATUS_COUNT);
	this->SetStatusText(wxT("Welcome to my1sim85!"));
	const int cWidths[STATUS_COUNT] = { STATUS_FIX_WIDTH, -1 };
	wxStatusBar* cStatusBar = this->GetStatusBar();
	cStatusBar->SetStatusWidths(STATUS_COUNT,cWidths);
	mDisplayTimer = new wxTimer(this, MY1ID_STAT_TIMER);

	// some handy pointers
	mConsole = 0x0;
	mCommand = 0x0;

	// setup image
	wxInitAllImageHandlers();
	wxIcon mIconApps = MACRO_WXICO(apps);
	this->SetIcon(mIconApps);

	// menu bar
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(MY1ID_LOAD, wxT("&Load\tF2"));
	fileMenu->Append(MY1ID_SAVE, wxT("&Save\tF3"));
	fileMenu->Append(MY1ID_NEW, wxT("&Clear\tF4"));
	fileMenu->AppendSeparator();
	fileMenu->Append(MY1ID_EXIT, wxT("E&xit"), wxT("Quit program"));
	wxMenu *editMenu = new wxMenu;
	editMenu->Append(MY1ID_OPTIONS, wxT("&Preferences...\tF8"));
	wxMenu *viewMenu = new wxMenu;
	viewMenu->Append(MY1ID_VIEW_INITPAGE, wxT("View Welcome Page"));
	viewMenu->Append(MY1ID_VIEW_INFOPANE, wxT("View Info Panel"));
	viewMenu->Append(MY1ID_VIEW_LOGSPANE, wxT("View Log Panel"));
	wxMenu *procMenu = new wxMenu;
	procMenu->Append(MY1ID_ASSEMBLE, wxT("&Assemble\tF5"));
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
	mNoteBook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE);
	mNoteBook->AddPage(CreateMainPanel(mNoteBook), wxT("Welcome"), true);
	mMainUI.AddPane(mNoteBook, wxAuiPaneInfo().Name(wxT("codeBook")).
		CenterPane().Layer(3).PaneBorder(false));
	// tool bar - proc
	mMainUI.AddPane(CreateProcToolBar(), wxAuiPaneInfo().Name(wxT("procTool")).
		ToolbarPane().Top().LeftDockable(false).RightDockable(false).BottomDockable(false));
	// tool bar - edit
	mMainUI.AddPane(CreateEditToolBar(), wxAuiPaneInfo().Name(wxT("editTool")).
		ToolbarPane().Top().LeftDockable(false).RightDockable(false).BottomDockable(false));
	// tool bar - file
	mMainUI.AddPane(CreateFileToolBar(), wxAuiPaneInfo().Name(wxT("fileTool")).
		ToolbarPane().Top().LeftDockable(false).RightDockable(false).BottomDockable(false));
	// info panel
	mMainUI.AddPane(CreateInfoPanel(), wxAuiPaneInfo().Name(wxT("infoPanel")).
		Caption(wxT("Information")).DefaultPane().Left().Layer(2).
		TopDockable(false).RightDockable(false).BottomDockable(false).
		MinSize(wxSize(INFO_PANEL_WIDTH,0)));
	// reg panel
	mMainUI.AddPane(CreateREGPanel(this), wxAuiPaneInfo().Name(wxT("regsPanel")).
		Caption(wxT("Registers")).DefaultPane().Left().Layer(2).
		TopDockable(false).RightDockable(true).BottomDockable(false).
		MinSize(wxSize(INFO_PANEL_WIDTH,0)));
	// dev panel
	mMainUI.AddPane(CreateDEVPanel(this), wxAuiPaneInfo().Name(wxT("devsPanel")).
		Caption(wxT("Devices")).DefaultPane().Right().Layer(2).
		TopDockable(false).LeftDockable(true).BottomDockable(false).
		MinSize(wxSize(INFO_PANEL_WIDTH,0)));
	// simulation panel
	mMainUI.AddPane(CreateSimsPanel(), wxAuiPaneInfo().Name(wxT("simsPanel")).
		Caption(wxT("Simulation")).DefaultPane().Right().Layer(2).
		TopDockable(false).BottomDockable(false).LeftDockable(false).
		Float().Center().Hide().CloseButton(false));
	mMainUI.AddPane(CreateLogsPanel(), wxAuiPaneInfo().Name(wxT("logsPanel")).
		Caption(wxT("Logs Panel")).DefaultPane().Bottom().
		MaximizeButton(true).Position(0).
		TopDockable(false).RightDockable(false).LeftDockable(false).
		MinSize(wxSize(0,CONS_PANEL_HEIGHT)));
	// commit changes!
	mMainUI.Update();

/*
	std::cout << "DEBUG1!" << std::endl;
	std::cout << "DEBUG2!" << std::endl;
	std::cout << "DEBUG3!" << std::endl;
*/

	// actions & events!
	this->Connect(MY1ID_EXIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnQuit));
	this->Connect(MY1ID_NEW, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnNew));
	this->Connect(MY1ID_LOAD, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnLoad));
	this->Connect(MY1ID_SAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnSave));
	this->Connect(MY1ID_ABOUT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnAbout));
	this->Connect(MY1ID_VIEW_INITPAGE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnShowInitPage));
	this->Connect(MY1ID_VIEW_INFOPANE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnShowPanel));
	this->Connect(MY1ID_VIEW_LOGSPANE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnShowPanel));
	this->Connect(wxID_ANY, wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler(my1Form::OnClosePane));
	this->Connect(wxID_ANY, wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(my1Form::OnPageChanged));
	this->Connect(wxID_ANY, wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, wxAuiNotebookEventHandler(my1Form::OnPageClosing));
	this->Connect(MY1ID_OPTIONS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnCheckOptions));
	this->Connect(MY1ID_ASSEMBLE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnAssemble));
	this->Connect(MY1ID_STAT_TIMER, wxEVT_TIMER, wxTimerEventHandler(my1Form::OnStatusTimer));
	this->Connect(MY1ID_CONSCOMM, wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(my1Form::OnExecuteConsole));
	this->Connect(MY1ID_CONSEXEC, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnExecuteConsole));
	this->Connect(MY1ID_SIMSEXEC, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnSimulate));
	this->Connect(MY1ID_SIMSSTEP, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnSimulate));
	this->Connect(MY1ID_SIMSINFO, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnSimulationInfo));
	this->Connect(MY1ID_SIMSEXIT, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnSimulationExit));

	// position this!
	//this->Centre();
	this->Maximize();
	if(mCommand) mCommand->SetFocus();
}

my1Form::~my1Form()
{
	mMainUI.UnInit();
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
	cPane.Show(aGo);
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
	fileTool->AddTool(MY1ID_EXIT, wxT("Exit"), mIconExit);
	fileTool->AddSeparator();
	fileTool->AddTool(MY1ID_NEW, wxT("Clear"), mIconNewd);
	fileTool->AddTool(MY1ID_LOAD, wxT("Load"), mIconLoad);
	fileTool->AddTool(MY1ID_SAVE, wxT("Save"), mIconSave);
	fileTool->Realize();
	return fileTool;
}

wxAuiToolBar* my1Form::CreateEditToolBar(void)
{
	wxBitmap mIconOptions = MACRO_WXBMP(option);
	wxAuiToolBar* editTool = new wxAuiToolBar(this, MY1ID_EDITTOOL, wxDefaultPosition,
		wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	editTool->SetToolBitmapSize(wxSize(16,16));
	editTool->AddTool(MY1ID_OPTIONS, wxT("Options"), mIconOptions);
	editTool->Realize();
	return editTool;
}

wxAuiToolBar* my1Form::CreateProcToolBar(void)
{
	wxBitmap mIconAssemble = MACRO_WXBMP(binary);
	wxAuiToolBar* procTool = new wxAuiToolBar(this, MY1ID_PROCTOOL, wxDefaultPosition,
		wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	procTool->SetToolBitmapSize(wxSize(16,16));
	procTool->AddTool(MY1ID_ASSEMBLE, wxT("Assemble"), mIconAssemble);
	procTool->Realize();
	procTool->Enable(false); // disabled by default!
	return procTool;
}

wxPanel* my1Form::CreateMainPanel(wxWindow *parent)
{
	wxPanel *cPanel = new wxPanel(parent, wxID_ANY);
	wxStaticText *cLabel = new wxStaticText(cPanel, wxID_ANY, wxT("MY1 Sim85"));
	wxFont cFont(24,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cLabel->SetFont(cFont);
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->Add(cLabel,1,wxALIGN_CENTER);
	wxStaticText *dLabel = new wxStaticText(cPanel, wxID_ANY, wxT("by azman@my1matrix.net"));
	wxFont dFont(8,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	dLabel->SetFont(dFont);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	pBoxSizer->Add(cBoxSizer,1,wxALIGN_CENTRE);
	pBoxSizer->Add(dLabel,0,wxALIGN_BOTTOM|wxALIGN_RIGHT);
	cPanel->SetSizerAndFit(pBoxSizer);
	pBoxSizer->SetSizeHints(cPanel);
	return cPanel;
}

wxPanel* my1Form::CreateInfoPanel(void)
{
	wxPanel *cPanel = new wxPanel(this,MY1ID_INFOPANEL);
	cPanel->SetMinSize(wxSize(INFO_PANEL_WIDTH,0));
	wxFont cFont(8,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	//wxNotebook *cInfoBook = new wxNotebook(cPanel,MY1ID_LOGBOOK);
	//cInfoBook->AddPage(CreateREGPanel(cInfoBook),wxT("Registers"),true);
	//cInfoBook->AddPage(CreateDEVPanel(cInfoBook),wxT("I/O Devices"),true);
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	//cBoxSizer->Add(cInfoBook,1,wxEXPAND);
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
	wxButton *cButtonExit = new wxButton(cPanel, MY1ID_SIMSEXIT, wxT("Exit"),
		wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxVERTICAL);
	cBoxSizer->Add(cButtonStep, 0, wxALIGN_TOP);
	cBoxSizer->Add(cButtonExec, 0, wxALIGN_TOP);
	cBoxSizer->Add(cButtonInfo, 0, wxALIGN_TOP);
	cBoxSizer->Add(cButtonExit, 0, wxALIGN_TOP);
	cPanel->SetSizer(cBoxSizer);
	cBoxSizer->SetSizeHints(cPanel);
	return cPanel;
}

wxPanel* my1Form::CreateLogsPanel(void)
{
	wxPanel *cPanel = new wxPanel(this, MY1ID_INFOPANEL,
		wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxFont cTestFont(8,wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
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

	return cPanel;
}

wxBoxSizer* my1Form::CreateREGView(wxWindow* aParent, const wxString& aString, int anID)
{
	wxString cDefault = wxT("00");
	my1Reg85 *pReg85 = m8085.GetRegister(anID);
	if(pReg85->IsDoubleSize()) cDefault += wxT("00");
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

wxPanel* my1Form::CreateREGPanel(wxWindow* aParent)
{
	wxPanel *cPanel = new wxPanel(aParent, wxID_ANY);
	wxFont cFont(8,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Register B"),I8085_REG_B),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Register C"),I8085_REG_C),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Register D"),I8085_REG_D),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Register E"),I8085_REG_E),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Register H"),I8085_REG_H),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Register L"),I8085_REG_L),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Register A"),I8085_REG_A),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Register F"),I8085_REG_F),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Program Counter"),I8085_RP_PC+I8085_REG_COUNT),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Stack Pointer"),I8085_RP_SP+I8085_REG_COUNT),0,wxEXPAND);
	cPanel->SetSizerAndFit(pBoxSizer);
	return cPanel;
}

wxBoxSizer* my1Form::CreateLEDView(wxWindow* aParent, const wxString& aString, int anID)
{
	wxStaticText *cLabel = new wxStaticText(aParent, wxID_ANY, aString);
	my1LEDCtrl *cValue = new my1LEDCtrl(aParent, wxID_ANY);
	my1Device *pDevice = m8085.GetDevice(0);
	my1DevicePort *pPort = pDevice->GetDevicePort(anID/4);
	my1BitIO *pBitIO = pPort->GetBitIO(anID%8);
	pBitIO->GetLink();
	pBitIO->SetLink((void*)cValue);
	pBitIO->DoUpdate = &my1LEDCtrl::DoUpdate;
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
	my1BitIO *pBitIO = m8085.GetDevice(0)->GetDevicePort(anID/4)->GetBitIO(anID%4);
	pBitIO->SetLink((void*)cValue);
	pBitIO->DoDetect = &my1SWICtrl::DoDetect;
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->AddSpacer(INFO_DEV_SPACER);
	cBoxSizer->Add(cValue,0,wxALIGN_LEFT);
	cBoxSizer->AddSpacer(INFO_DEV_SPACER);
	cBoxSizer->Add(cLabel,1,wxALIGN_CENTER);
	return cBoxSizer;
}

wxPanel* my1Form::CreateDEVPanel(wxWindow* aParent)
{
	wxPanel *cPanel = new wxPanel(aParent, wxID_ANY);
	wxFont cFont(8,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED0 - PA0"),I8255_PIN_PA0),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED1 - PA1"),I8255_PIN_PA1),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED2 - PA2"),I8255_PIN_PA2),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED3 - PA3"),I8255_PIN_PA3),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED4 - PA4"),I8255_PIN_PA4),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED5 - PA5"),I8255_PIN_PA5),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED6 - PA6"),I8255_PIN_PA6),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED7 - PA7"),I8255_PIN_PA7),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI0 - PB0"),I8255_PIN_PB0),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI1 - PB1"),I8255_PIN_PB1),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI2 - PB2"),I8255_PIN_PB2),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI3 - PB3"),I8255_PIN_PB3),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI4 - PB4"),I8255_PIN_PB4),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI5 - PB5"),I8255_PIN_PB5),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI6 - PB6"),I8255_PIN_PB6),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI7 - PB7"),I8255_PIN_PB7),0,wxEXPAND);
	cPanel->SetSizerAndFit(pBoxSizer);
	return cPanel;
}

wxPanel* my1Form::CreateMEMPanel(wxWindow* aParent)
{
	wxPanel *cPanel = new wxPanel(aParent, wxID_ANY);
	wxFont cFont(8,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	cPanel->SetSizerAndFit(pBoxSizer);
	return cPanel;
}

void my1Form::OpenEdit(wxString& cFileName)
{
	int cTestID = wxID_ANY;
	my1CodeEdit *cCodeEdit = new my1CodeEdit(mNoteBook, cTestID, cFileName, this->mOptions);
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
			wxFD_DEFAULT_STYLE|wxFD_OVERWRITE_PROMPT|wxFD_CHANGE_DIR);
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
		wxFD_DEFAULT_STYLE|wxFD_FILE_MUST_EXIST|wxFD_CHANGE_DIR);
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
	int cSelect = mNoteBook->GetSelection();
	wxWindow *cTarget = mNoteBook->GetPage(cSelect);
	if(!cTarget->IsKindOf(CLASSINFO(my1CodeEdit))) return; // error? shouldn't get here!
	my1CodeEdit *cEditor = (my1CodeEdit*) cTarget;
	wxStreamToTextRedirector cRedirect(mConsole);
	wxString cStatus = wxT("Processing ") + cEditor->GetFileName() + wxT("...");
	this->ShowStatus(cStatus);
	m8085.SetStartAddress(mOptions.mSims_StartADDR);
	if(m8085.Assemble(cEditor->GetFullName().ToAscii()))
	{
		cStatus = wxT("Code in ") + cEditor->GetFileName() + wxT(" loaded!");
		this->ShowStatus(cStatus);
		if(!mOptions.mSims_FreeRunning)
		{
			cEditor->ExecMode();
			cEditor->ExecLine(m8085.GetCodexLine()-1);
		}
		this->SimulationMode();
		mCommand->SetFocus();
	}
}

void my1Form::PrintPeripheralInfo(void)
{
	std::cout << "\n";
	for(int cLoop=0;cLoop<MAX_MEMCOUNT;cLoop++)
	{
		my1Memory* cMemory = m8085.GetMemory(cLoop);
		if(cMemory)
		{
			std::cout << "(Memory) Name: " << cMemory->GetName() << ", ";
			std::cout << "Read-Only: " << cMemory->IsReadOnly() << ", ";
			std::cout << "Start: 0x" << std::setw(4) << std::setfill('0') << std::setbase(16) << cMemory->GetStart() << ", ";
			std::cout << "Size: 0x" << std::setw(4) << std::setfill('0') << std::setbase(16) << cMemory->GetSize() << "\n";
		}
	}
	for(int cLoop=0;cLoop<MAX_DEVCOUNT;cLoop++)
	{
		my1Device* cDevice = m8085.GetDevice(cLoop);
		if(cDevice)
		{
			std::cout << "(Device) Name: " << cDevice->GetName() << ", ";
			std::cout << "Start: 0x" << std::setw(2) << std::setfill('0') << cDevice->GetStart() << ", ";
			std::cout << "Size: 0x" << std::setw(2) << std::setfill('0') << std::setbase(16) << cDevice->GetSize() << "\n";
		}
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

void my1Form::PrintHelp(void)
{
	std::cout << "\nAvailable command(s):" << "\n";
	std::cout << "- show [parts]" << "\n";
	std::cout << "  > print relevant info" << "\n";
	std::cout << "- sim [info|run|addr=?|mark=?]" << "\n";
	std::cout << "  > info (simulation timing info)" << "\n";
	std::cout << "  > run (execute simulation [NOT IMPLEMENTED!])" << "\n";
	std::cout << "  > addr=? (set simulation start addr)" << "\n";
	std::cout << "  > mark=? (show/hide line marker)" << "\n";
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
		cCommandLine = cCommandLine.AfterFirst(' ');
		if(!cCommandLine.BeforeFirst(' ').Cmp(wxT("parts")))
		{
			this->PrintPeripheralInfo();
		}
		else
		{
			this->PrintUnknownParameter(cParameters,cCommandWord);
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
				this->PrintSimInfo();
			}
			else if(!cParam.Cmp(wxT("run")))
			{
				// only available if sim ready?
				this->PrintConsoleMessage("Not implemented... yet!");
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
				if(cValue.ToULong(&cStart,16)&&cStart<0xFFFF)
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

void my1Form::OnSimulate(wxCommandEvent &event)
{
	mSimulationRunning = true;
	if(event.GetId()==MY1ID_SIMSEXEC)
		mSimulationStepping = false;
	else
		mSimulationStepping = true;
	int cSelect = this->mNoteBook->GetSelection();
	if(cSelect<0) return; // shouldn't get here!
	wxWindow *cTarget = this->mNoteBook->GetPage(cSelect);
	if(!cTarget->IsKindOf(CLASSINFO(my1CodeEdit))) return; // error? shouldn't get here!
	my1CodeEdit *cEditor = (my1CodeEdit*) cTarget;
	wxStreamToTextRedirector cRedirect(mConsole);
	// big loop here!
	while(mSimulationRunning)
	{
		if(m8085.Simulate())
		{
			if(!mOptions.mSims_FreeRunning)
			{
				cEditor->ExecMode();
				cEditor->ExecLine(m8085.GetCodexLine()-1);
			}
			else
			{
				cEditor->ExecDone();
			}
			if(cEditor->IsBreakLine())
				break;
			if(mSimulationStepping)
				break;
			my1AppPointer->Yield();
		}
		else
		{
			wxMessageBox(wxT("Simulation Terminated!"),wxT("Error!"));
			mSimulationRunning = false;
			break;
		}
	}
	if(!mSimulationRunning)
	{
		this->SimulationMode(false);
	}
}

void my1Form::OnSimulationInfo(wxCommandEvent &event)
{
	if(event.GetId()==MY1ID_SIMSINFO)
	{
		wxStreamToTextRedirector cRedirect(mConsole);
		m8085.PrintCodexInfo();
	}
}

void my1Form::OnSimulationExit(wxCommandEvent &event)
{
	if(event.GetId()==MY1ID_SIMSEXIT)
	{
		int cSelect = this->mNoteBook->GetSelection();
		if(cSelect<0) return; // shouldn't get here!
		wxWindow *cTarget = this->mNoteBook->GetPage(cSelect);
		if(!cTarget->IsKindOf(CLASSINFO(my1CodeEdit))) return; // error? shouldn't get here!
		my1CodeEdit *cEditor = (my1CodeEdit*) cTarget;
		cEditor->ExecDone();
		mSimulationRunning = false;
		this->SimulationMode(false);
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
}

void my1Form::OnShowInitPage(wxCommandEvent &event)
{
	if(mNoteBook->GetPageCount()>0)
	{
		wxWindow *cTarget = mNoteBook->GetPage(0); // always first!
		if(cTarget->IsKindOf(CLASSINFO(my1CodeEdit)))
		{
			mNoteBook->AddPage(CreateMainPanel(mNoteBook), wxT("Welcome"), true);
		}
	}
	return;
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

void my1Form::OnPageChanged(wxAuiNotebookEvent &event)
{
	wxMenuBar *cMenuBar = this->GetMenuBar();
	wxAuiToolBar *cProcTool = (wxAuiToolBar*) this->FindWindow(MY1ID_PROCTOOL);
	int cSelect = event.GetSelection();
	wxWindow *cTarget = mNoteBook->GetPage(cSelect);
	if(!cTarget) return;
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

void my1Form::SimUpdateREG(void* simObject)
{
	// update register view
	my1Reg85 *pReg85 = (my1Reg85*) simObject;
	wxString cFormat = "%02X";
	if(pReg85->IsDoubleSize()) cFormat = "%04X";
	cFormat = wxString::Format(cFormat,pReg85->GetData());
	wxTextCtrl *pText = (wxTextCtrl*) pReg85->GetLink();
	pText->ChangeValue(cFormat);
}

void my1Form::SimDoUpdate(void* simObject)
{
	// microprocessor level update?
}

void my1Form::SimDoDelay(void* simObject)
{
	my1Sim85* mySim = (my1Sim85*) simObject;
	wxWindow *pWindow = FindWindowById(MY1ID_MAIN);
	my1Form* myForm = (my1Form*) pWindow;
	std::clock_t cTime1, cTime2;
	cTime1 = cTime2 = std::clock();
	double cTest, cTotal = myForm->GetSimCycle()*mySim->GetStateExec();
	do
	{
		cTime2 = std::clock();
		cTest = (double) (cTime2-cTime1) / CLOCKS_PER_SEC;
	}
	while(cTest<cTotal);
	//wxMicroSleep(mySim->GetStateExec());
}
