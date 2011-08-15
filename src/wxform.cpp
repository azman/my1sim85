/**
*
* wxform.cpp
*
* - implementation for main wx-based form
*
**/

#include "wxform.hpp"
#include "wxcode.hpp"
#include "wxled.hpp"
#include "wxswitch.hpp"

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
#define SIM_EXEC_PERIOD 1
#define SIM_START_ADDR 0x2000
// ms scale?
#define MY1CLOCK_DIV 1000

#if USE_XPM_BITMAPS
	#define MY1BITMAP_EXIT   "res/exit.xpm"
	#define MY1BITMAP_NEW    "res/new.xpm"
	#define MY1BITMAP_OPEN   "res/open.xpm"
	#define MY1BITMAP_SAVE   "res/save.xpm"
	#define MY1BITMAP_BINARY "res/binary.xpm"
	#define MY1BITMAP_OPTION "res/option.xpm"
#else
	#define MY1BITMAP_EXIT   "exit"
	#define MY1BITMAP_NEW    "new"
	#define MY1BITMAP_OPEN   "open"
	#define MY1BITMAP_SAVE   "save"
	#define MY1BITMAP_BINARY "binary"
	#define MY1BITMAP_OPTION "option"
#endif

my1Form::my1Form(const wxString &title)
	: wxFrame( NULL, MY1ID_MAIN, title, wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_FRAME_STYLE), m8085(true)
{
	// simulation stuffs
	mSimulationRun = false;
	mSimulationCycle = 0.0;
	mSimulationCycleDefault = 0.0;
	this->CalculateSimCycle();

	// default option?
	mOptions.mChanged = false;
	mOptions.mEdit_ViewWS = false;
	mOptions.mEdit_ViewEOL = false;
	mOptions.mConv_UnixEOL = false;
	mOptions.mSims_StartADDR = SIM_START_ADDR;

	// assign function pointers :p
	m8085.DoUpdate = &this->SimDoUpdate;
	m8085.DoDelay = &this->SimDoDelay;

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

	// setup simulation timer
	mSimulationTimer = new wxTimer(this, MY1ID_SIM_TIMER);

	// setup image
	wxInitAllImageHandlers();
	this->SetIcon(wxIcon(wxT(MY1APP_ICON)));

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
	// simulation panel
	mMainUI.AddPane(CreateSimsPanel(), wxAuiPaneInfo().Name(wxT("simsPanel")).
		Caption(wxT("Simulation")).DefaultPane().Right().Layer(2).
		TopDockable(false).BottomDockable(false).LeftDockable(false).
		Float().Center().Hide());
	mMainUI.AddPane(CreateLogsPanel(), wxAuiPaneInfo().Name(wxT("logsPanel")).
		Caption(wxT("Logs Panel")).DefaultPane().Bottom().
		MaximizeButton(true).Position(0).
		TopDockable(false).RightDockable(false).LeftDockable(false).
		MinSize(wxSize(0,CONS_PANEL_HEIGHT)));
	// commit changes!
	mMainUI.Update();

	// actions & events!
	this->Connect(MY1ID_EXIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnQuit));
	this->Connect(MY1ID_LOAD, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnLoad));
	this->Connect(MY1ID_SAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnSave));
	this->Connect(MY1ID_VIEW_INITPAGE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnShowInitPage));
	this->Connect(MY1ID_VIEW_INFOPANE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnShowPanel));
	this->Connect(MY1ID_VIEW_LOGSPANE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnShowPanel));
	this->Connect(wxID_ANY, wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler(my1Form::OnClosePane));
	this->Connect(wxID_ANY, wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(my1Form::OnPageChanged));
	this->Connect(wxID_ANY, wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, wxAuiNotebookEventHandler(my1Form::OnPageClosing));
	this->Connect(MY1ID_OPTIONS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnCheckOptions));
	this->Connect(MY1ID_ASSEMBLE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnAssemble));
	this->Connect(MY1ID_STAT_TIMER, wxEVT_TIMER, wxTimerEventHandler(my1Form::OnStatusTimer));
	this->Connect(MY1ID_SIM_TIMER, wxEVT_TIMER, wxTimerEventHandler(my1Form::OnSimTimer));
	this->Connect(MY1ID_CONSEXEC, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnExecuteConsole));
	this->Connect(MY1ID_SIMSEXEC, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnSimulate));
	this->Connect(MY1ID_SIMSSTEP, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnSimulate));
	this->Connect(MY1ID_SIMSINFO, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1Form::OnSimulationInfo));

	// position this!
	//this->Centre();
	this->Maximize();
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
	mSimulationCycleDefault /= (double) CLOCKS_PER_SEC * MY1CLOCK_DIV;
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
}

wxAuiToolBar* my1Form::CreateFileToolBar(void)
{
	wxIcon mIconExit(wxT(MY1BITMAP_EXIT));
	wxIcon mIconNew(wxT(MY1BITMAP_NEW));
	wxIcon mIconLoad(wxT(MY1BITMAP_OPEN));
	wxIcon mIconSave(wxT(MY1BITMAP_SAVE));
	wxAuiToolBar* fileTool = new wxAuiToolBar(this, MY1ID_FILETOOL, wxDefaultPosition,
		wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	fileTool->SetToolBitmapSize(wxSize(16,16));
	fileTool->AddTool(MY1ID_EXIT, wxT("Exit"), mIconExit);
	fileTool->AddSeparator();
	fileTool->AddTool(MY1ID_NEW, wxT("Clear"), mIconNew);
	fileTool->AddTool(MY1ID_LOAD, wxT("Load"), mIconLoad);
	fileTool->AddTool(MY1ID_SAVE, wxT("Save"), mIconSave);
	fileTool->Realize();
	return fileTool;
}

wxAuiToolBar* my1Form::CreateEditToolBar(void)
{
	wxIcon mIconOptions(wxT(MY1BITMAP_OPTION));
	wxAuiToolBar* editTool = new wxAuiToolBar(this, MY1ID_EDITTOOL, wxDefaultPosition,
		wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	editTool->SetToolBitmapSize(wxSize(16,16));
	editTool->AddTool(MY1ID_OPTIONS, wxT("Options"), mIconOptions);
	editTool->Realize();
	return editTool;
}

wxAuiToolBar* my1Form::CreateProcToolBar(void)
{
	wxIcon mIconAssemble(wxT(MY1BITMAP_BINARY));
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
	// welcome panel?
	return cPanel;
}

wxPanel* my1Form::CreateInfoPanel(void)
{
	wxPanel *cPanel = new wxPanel(this,MY1ID_INFOPANEL);
	cPanel->SetMinSize(wxSize(INFO_PANEL_WIDTH,0));
	wxFont cFont(8,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cFont);
	wxNotebook *cInfoBook = new wxNotebook(cPanel,MY1ID_LOGBOOK);
	cInfoBook->AddPage(CreateREGPanel(cInfoBook),wxT("Registers"),true);
	cInfoBook->AddPage(CreateDEVPanel(cInfoBook),wxT("I/O Devices"),true);
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->Add(cInfoBook,1,wxEXPAND);
	cPanel->SetSizer(cBoxSizer);
	cBoxSizer->SetSizeHints(cPanel);
	my1Form::SimDoUpdate((void*)&m8085);
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
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxVERTICAL);
	cBoxSizer->Add(cButtonStep, 0, wxALIGN_TOP);
	cBoxSizer->Add(cButtonExec, 0, wxALIGN_TOP);
	cBoxSizer->Add(cButtonInfo, 0, wxALIGN_TOP);
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
	wxTextCtrl *cCommandText = new wxTextCtrl(cComsPanel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize);
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
	// dummy page
	wxPanel *cChkPanel = new wxPanel(cLogBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	// add the pages
	cLogBook->AddPage(cConsPanel, wxT("Console"), true);
	cLogBook->AddPage(cChkPanel, wxT("Void"), false);
	// 'remember' main console
	if(!mConsole) mConsole = cConsole;
	// main box-sizer
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->Add(cLogBook, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
	cPanel->SetSizer(cBoxSizer);
	cBoxSizer->SetSizeHints(cPanel);

	return cPanel;
}

wxBoxSizer* my1Form::CreateREGView(wxWindow* aParent, const wxString& aString, int anID, bool aReg16)
{
	wxString cDefault = wxT("00");
	if(aReg16) cDefault += wxT("00");
	wxStaticText *cLabel = new wxStaticText(aParent, wxID_ANY, aString);
	wxTextCtrl *cValue = new wxTextCtrl(aParent, anID, cDefault,
		wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->Add(cLabel,1,wxALIGN_CENTER);
	cBoxSizer->Add(cValue,0,wxALIGN_RIGHT);
	return cBoxSizer;
}

wxPanel* my1Form::CreateREGPanel(wxWindow* aParent)
{
	wxPanel *cPanel = new wxPanel(aParent, wxID_ANY);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Register B"),MY1ID_REGB_VAL),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Register C"),MY1ID_REGC_VAL),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Register D"),MY1ID_REGD_VAL),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Register E"),MY1ID_REGE_VAL),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Register H"),MY1ID_REGH_VAL),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Register L"),MY1ID_REGL_VAL),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Register A"),MY1ID_REGA_VAL),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Register F"),MY1ID_REGF_VAL),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Program Counter"),MY1ID_REGPC_VAL,true),0,wxEXPAND);
	pBoxSizer->Add(CreateREGView(cPanel,wxT("Stack Pointer"),MY1ID_REGSP_VAL,true),0,wxEXPAND);
	cPanel->SetSizerAndFit(pBoxSizer);
	return cPanel;
}

wxBoxSizer* my1Form::CreateLEDView(wxWindow* aParent, const wxString& aString, int anID)
{
	wxStaticText *cLabel = new wxStaticText(aParent, wxID_ANY, aString);
	my1LEDCtrl *cValue = new my1LEDCtrl(aParent, anID);
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
	my1SWICtrl *cValue = new my1SWICtrl(aParent, anID);
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
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateLEDView(cPanel,wxT("LED0"),MY1ID_LED0_VAL),0,wxEXPAND);
	pBoxSizer->AddSpacer(INFO_DEV_SPACER);
	pBoxSizer->Add(CreateSWIView(cPanel,wxT("SWI0"),MY1ID_SWI0_VAL),0,wxEXPAND);
	cPanel->SetSizerAndFit(pBoxSizer);
	return cPanel;
}

void my1Form::OpenEdit(wxString& cFileName)
{
	int cTestID = wxID_ANY;
	my1CodeEdit *cCodeEdit = new my1CodeEdit(mNoteBook, cTestID, cFileName, this->mOptions);
	mNoteBook->AddPage(cCodeEdit, cCodeEdit->GetFileName(),true);
	if(mOptions.mConv_UnixEOL)
		cCodeEdit->ConvertEOLs(2);
	wxString cStatus = wxT("File ") + cCodeEdit->GetFileName() + wxT(" loaded!");
	this->ShowStatus(cStatus);
}

void my1Form::SaveEdit(wxWindow* cEditPane)
{
	my1CodeEdit *cEditor = (my1CodeEdit*) cEditPane;
	cEditor->SaveEdit();
	wxString cStatus = wxT("File ") + cEditor->GetFileName() + wxT(" saved!");
	this->ShowStatus(cStatus);
}

void my1Form::ShowStatus(wxString& aString)
{
	this->SetStatusText(aString,STATUS_MSG_INDEX);
	mDisplayTimer->Start(STATUS_MSG_PERIOD,wxTIMER_ONE_SHOT);
}

void my1Form::UpdateRegValue(wxWindow* aWindow, int aWhich, bool aReg16)
{
	wxString cFormat = "%02X";
	if(aReg16) cFormat = "%04X";
	wxTextCtrl *pText = (wxTextCtrl*) aWindow;
	pText->ChangeValue(wxString::Format(cFormat,m8085.GetRegValue(aWhich,aReg16)));
}

void my1Form::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
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

void my1Form::OnAssemble(wxCommandEvent &event)
{
	int cSelect = mNoteBook->GetSelection();
	if(cSelect<0) return;
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
		wxString cToolName = wxT("simsPanel");
		wxAuiPaneInfo& cPane = mMainUI.GetPane(cToolName);
		cPane.Show();
		mMainUI.Update();
		this->SimulationMode();
		my1Form::SimDoUpdate((void*)&m8085);
		cEditor->ExecMode();
		cEditor->ExecLine(m8085.GetCodexLine()-1);
	}
}

void my1Form::OnExecuteConsole(wxCommandEvent &event)
{
	wxStreamToTextRedirector cRedirect(mConsole);
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
}

void my1Form::OnSimulate(wxCommandEvent &event)
{
	if(event.GetId()==MY1ID_SIMSEXEC)
		mSimulationRun = true;
	else
		mSimulationRun = false;
	mSimulationTimer->Start(SIM_EXEC_PERIOD,wxTIMER_ONE_SHOT);
}

void my1Form::OnSimulationInfo(wxCommandEvent &event)
{
	if(event.GetId()==MY1ID_SIMSINFO)
	{
		wxStreamToTextRedirector cRedirect(mConsole);
		m8085.PrintCodexInfo();
	}
}

void my1Form::OnClosePane(wxAuiManagerEvent &event)
{
	wxAuiPaneInfo *cPane = event.GetPane();
	wxAuiPaneInfo &rPane = mMainUI.GetPane("simsPanel");
	if(cPane==&rPane)
	{
		int cSelect = this->mNoteBook->GetSelection();
		if(cSelect<0) return; // shouldn't get here!
		wxWindow *cTarget = this->mNoteBook->GetPage(cSelect);
		if(!cTarget->IsKindOf(CLASSINFO(my1CodeEdit))) return; // error? shouldn't get here!
		my1CodeEdit *cEditor = (my1CodeEdit*) cTarget;
		cEditor->ExecDone();
		this->SimulationMode(false);
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

void my1Form::OnSimTimer(wxTimerEvent& event)
{
	int cSelect = this->mNoteBook->GetSelection();
	if(cSelect<0) return; // shouldn't get here!
	wxWindow *cTarget = this->mNoteBook->GetPage(cSelect);
	if(!cTarget->IsKindOf(CLASSINFO(my1CodeEdit))) return; // error? shouldn't get here!
	my1CodeEdit *cEditor = (my1CodeEdit*) cTarget;
	wxStreamToTextRedirector cRedirect(mConsole);
	if(m8085.Simulate())
	{
		if(cEditor->ExecLine(m8085.GetCodexLine()-1)) // breakpoint found!
			mSimulationRun = false;
		if(mSimulationRun)
			mSimulationTimer->Start(SIM_EXEC_PERIOD,wxTIMER_ONE_SHOT);
	}
	else
	{
		wxMessageBox(wxT("Simulation Terminated!"),wxT("Error!"));
		wxString cToolName = wxT("simsPanel");
		wxAuiPaneInfo& cPane = mMainUI.GetPane(cToolName);
		cPane.Hide();
		mMainUI.Update();
		this->SimulationMode(false);
	}
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

void my1Form::SimDoUpdate(void* simObject)
{
	//my1Sim85* mySim = (my1Sim85*) simObject;
	wxWindow *pWindow = FindWindowById(MY1ID_MAIN);
	my1Form* myForm = (my1Form*) pWindow;
	// update register view???
	myForm->UpdateRegValue(FindWindowById(MY1ID_REGB_VAL),I8085_REG_B);
	myForm->UpdateRegValue(FindWindowById(MY1ID_REGC_VAL),I8085_REG_C);
	myForm->UpdateRegValue(FindWindowById(MY1ID_REGD_VAL),I8085_REG_D);
	myForm->UpdateRegValue(FindWindowById(MY1ID_REGE_VAL),I8085_REG_E);
	myForm->UpdateRegValue(FindWindowById(MY1ID_REGH_VAL),I8085_REG_H);
	myForm->UpdateRegValue(FindWindowById(MY1ID_REGL_VAL),I8085_REG_L);
	myForm->UpdateRegValue(FindWindowById(MY1ID_REGA_VAL),I8085_REG_A);
	myForm->UpdateRegValue(FindWindowById(MY1ID_REGF_VAL),I8085_REG_F);
	myForm->UpdateRegValue(FindWindowById(MY1ID_REGPC_VAL),I8085_RP_PC,true);
	myForm->UpdateRegValue(FindWindowById(MY1ID_REGSP_VAL),I8085_RP_SP,true);
	// update memory/device view???
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
		cTest = (double) (cTime2-cTime1) / CLOCKS_PER_SEC * MY1CLOCK_DIV;
	}
	while(cTest<cTotal);
}
