/**
*
* wxform.cpp
*
* - implementation for main wx-based form
*
**/

#include "wxform.hpp"
#include "wxcode.hpp"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define INFO_PANEL_WIDTH 200
#define CONS_PANEL_HEIGHT 100
#define STATUS_COUNT 2
#define STATUS_FIX_WIDTH INFO_PANEL_WIDTH
#define STATUS_MSG_INDEX 1
#define STATUS_MSG_PERIOD 3000

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
	: wxFrame( NULL, wxID_ANY, title, wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_FRAME_STYLE)
{
	// default option?
	mOptions.mChanged = false;
	mOptions.mEdit_ViewWS = false;
	mOptions.mEdit_ViewEOL = false;
	mOptions.mConv_UnixEOL = false;

	// minimum window size... duh!
	this->SetMinSize(wxSize(WIN_WIDTH,WIN_HEIGHT));

	// status bar
	this->CreateStatusBar(STATUS_COUNT);
	this->SetStatusText(wxT("Welcome to my1sim85!"));
	const int cWidths[STATUS_COUNT] = { STATUS_FIX_WIDTH, -1 };
	wxStatusBar* cStatusBar = this->GetStatusBar();
	cStatusBar->SetStatusWidths(STATUS_COUNT,cWidths);
	mDisplayTimer = new wxTimer(this, MY1ID_STAT_TIMER);
	mConsole = 0x0;

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
	mMainUI.AddPane(CreateProcToolBar(), wxAuiPaneInfo().Name(wxT("procTool")).Hide().
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
		Caption(wxT("Simulation")).DefaultPane().Bottom().Hide().
		MaximizeButton(true).Position(0).
		TopDockable(false).RightDockable(false).LeftDockable(false).
		MinSize(wxSize(0,CONS_PANEL_HEIGHT)));
	mMainUI.AddPane(CreateLogsPanel(), wxAuiPaneInfo().Name(wxT("logsPanel")).
		Caption(wxT("Logs Panel")).DefaultPane().Bottom().
		MaximizeButton(true).Position(0).
		TopDockable(false).RightDockable(false).LeftDockable(false).
		MinSize(wxSize(0,CONS_PANEL_HEIGHT)));
	// commit changes!
	mMainUI.Update();

	// actions!
	this->Connect(MY1ID_EXIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnQuit));
	this->Connect(MY1ID_LOAD, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnLoad));
	this->Connect(MY1ID_SAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnSave));
	this->Connect(MY1ID_VIEW_INITPAGE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnShowInitPage));
	this->Connect(MY1ID_VIEW_INFOPANE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnShowPanel));
	this->Connect(MY1ID_VIEW_LOGSPANE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnShowPanel));
	this->Connect(wxID_ANY, wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler(my1Form::OnClosePane));
	this->Connect(wxID_ANY, wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(my1Form::OnPageChanged));
	this->Connect(MY1ID_OPTIONS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnCheckOptions));
	this->Connect(MY1ID_ASSEMBLE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnAssemble));
	this->Connect(MY1ID_STAT_TIMER, wxEVT_TIMER, wxTimerEventHandler(my1Form::OnStatusTimer));
	this->Connect(wxID_ANY, wxEVT_STC_MODIFIED, wxStyledTextEventHandler(my1Form::OnCodeChanged));

	// events!
	this->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(my1Form::OnMouseClick));
	this->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(my1Form::OnMouseClick));

	// position this!
	//this->Centre();
	this->Maximize();
}

my1Form::~my1Form()
{
	mMainUI.UnInit();
}

wxAuiToolBar* my1Form::CreateFileToolBar(void)
{
	wxIcon mIconExit(wxT(MY1BITMAP_EXIT));
	wxIcon mIconNew(wxT(MY1BITMAP_NEW));
	wxIcon mIconLoad(wxT(MY1BITMAP_OPEN));
	wxIcon mIconSave(wxT(MY1BITMAP_SAVE));
	wxAuiToolBar* fileTool = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition,
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
	wxAuiToolBar* editTool = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition,
		wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	editTool->SetToolBitmapSize(wxSize(16,16));
	editTool->AddTool(MY1ID_OPTIONS, wxT("Options"), mIconOptions);
	editTool->Realize();
	return editTool;
}

wxAuiToolBar* my1Form::CreateProcToolBar(void)
{
	wxIcon mIconAssemble(wxT(MY1BITMAP_BINARY));
	wxAuiToolBar* procTool = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition,
		wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	procTool->SetToolBitmapSize(wxSize(16,16));
	procTool->AddTool(MY1ID_ASSEMBLE, wxT("Assemble"), mIconAssemble);
	procTool->Realize();
	return procTool;
}

wxPanel* my1Form::CreateMainPanel(wxWindow *parent)
{
	wxPanel *cPanel = new wxPanel(parent, wxID_ANY);
	// welcome panel?
	return cPanel;
}

#define INFO_REGV_HEIGHT 20
#define INFO_REGV_WIDTH 40
#define INFO_REGL_HEIGHT 20
#define INFO_REGL_WIDTH 60

wxPanel* my1Form::CreateInfoPanel(void)
{
	wxPanel *cPanel = new wxPanel(this, MY1ID_INFOPANEL,
		wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	cPanel->SetMinSize(wxSize(INFO_PANEL_WIDTH,0));
	wxFont cTestFont(8,wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	cPanel->SetFont(cTestFont);
	wxNotebook *cInfoBook = new wxNotebook(cPanel, MY1ID_LOGBOOK);
	wxPanel *cRegPanel = new wxPanel(cInfoBook, wxID_ANY);
	wxTextCtrl *cLabB = new wxTextCtrl(cRegPanel, wxID_ANY, wxT("Reg B"),
		wxDefaultPosition, wxSize(INFO_REGL_WIDTH,INFO_REGL_HEIGHT), wxTE_READONLY);
	wxTextCtrl *cValB = new wxTextCtrl(cRegPanel, wxID_ANY,
		wxString::Format("%02X",m8085.GetReg8Value(I8085_REG_B)),
		wxDefaultPosition, wxSize(INFO_REGV_WIDTH,INFO_REGV_HEIGHT));
	wxBoxSizer *pBoxSizerB = new wxBoxSizer(wxHORIZONTAL);
	pBoxSizerB->Add(cLabB, 0, wxALIGN_LEFT);
	pBoxSizerB->Add(cValB, 1);
	wxTextCtrl *cLabC = new wxTextCtrl(cRegPanel, wxID_ANY, wxT("Reg C"),
		wxDefaultPosition, wxSize(INFO_REGL_WIDTH,INFO_REGL_HEIGHT), wxTE_READONLY);
	wxTextCtrl *cValC = new wxTextCtrl(cRegPanel, wxID_ANY,
		wxString::Format("%02X",m8085.GetReg8Value(I8085_REG_C)),
		wxDefaultPosition, wxSize(INFO_REGV_WIDTH,INFO_REGV_HEIGHT));
	wxBoxSizer *pBoxSizerC = new wxBoxSizer(wxHORIZONTAL);
	pBoxSizerC->Add(cLabC, 0, wxALIGN_LEFT);
	pBoxSizerC->Add(cValC, 1);
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxVERTICAL);
	pBoxSizer->Add(pBoxSizerB, 1);
	pBoxSizer->Add(pBoxSizerC, 0);
	cRegPanel->SetSizer(pBoxSizer);
	cInfoBook->AddPage(cRegPanel, wxT("Registers"), true);
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->Add(cInfoBook, 1);
	cPanel->SetSizer(cBoxSizer);
	cBoxSizer->SetSizeHints(cPanel);
	return cPanel;
}

wxPanel* my1Form::CreateSimsPanel(void)
{
	wxPanel *cPanel = new wxPanel(this, MY1ID_SIMSPANEL,
		wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	cPanel->SetMinSize(wxSize(0,CONS_PANEL_HEIGHT));
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
	wxButton *cButton = new wxButton(cComsPanel, wxID_ANY, wxT("Execute"));
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
	cLogBook->AddPage(cChkPanel, wxT("Assembler"), false);
	// 'remember' main console
	if(!mConsole) mConsole = cConsole;
	// main box-sizer
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->Add(cLogBook, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
	cPanel->SetSizer(cBoxSizer);
	cBoxSizer->SetSizeHints(cPanel);

	return cPanel;
}

wxPanel* my1Form::CreateLEDPanel(void)
{
	wxPanel *cPanel = new wxPanel(this, MY1ID_LEDPANEL);
	return cPanel;
}

wxPanel* my1Form::CreateSWIPanel(void)
{
	wxPanel *cPanel = new wxPanel(this, MY1ID_SWIPANEL);
	return cPanel;
}

void my1Form::OpenEdit(wxString& cFileName)
{
	my1CodeEdit *cCodeEdit = new my1CodeEdit(mNoteBook, MY1ID_EDITMODIFIED, cFileName, this->mOptions);
	mNoteBook->AddPage(cCodeEdit, cCodeEdit->GetFileName(),true);
	if(mOptions.mConv_UnixEOL)
		cCodeEdit->ConvertEOLs(2);
	wxString cStatus = wxT("File ") + cCodeEdit->GetFileName() + wxT(" loaded!");
	this->ShowStatus(cStatus);
}

void my1Form::SaveEdit(wxWindow * cEditPane)
{
	my1CodeEdit *cEditor = (my1CodeEdit*) cEditPane;
	cEditor->SaveFile(cEditor->GetFullName());
	wxString cStatus = wxT("File ") + cEditor->GetFileName() + wxT(" saved!");
	this->ShowStatus(cStatus);
}

void my1Form::ShowStatus(wxString &aString)
{
	this->SetStatusText(aString,STATUS_MSG_INDEX);
	mDisplayTimer->Start(STATUS_MSG_PERIOD,wxTIMER_ONE_SHOT);
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
	if(m8085.Assemble(cEditor->GetFullName().ToAscii()))
	{
		cEditor->SetLockedLoad();
		*mConsole << "File " << cEditor->GetFileName() << " assembled!\n";
		wxString cToolName = wxT("simsPanel");
		wxAuiPaneInfo& cPane = mMainUI.GetPane(cToolName);
		cPane.Show();
	}
}

void my1Form::OnMouseClick(wxMouseEvent &event)
{
	// get event location
	//wxPoint pos = event.GetPosition();
	//if(event.LeftDown())
	//else if(event.RightDown())
}

void my1Form::OnClosePane(wxAuiManagerEvent &event)
{
	//DEBUG LINE! //wxMessageBox(wxT("Okay!"),wxT("Test"));
	//wxAuiPaneInfo *cPane = event.GetPane();
	//cPane->Hide();
	//mMainUI.Update();
	//event.Veto();
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

void my1Form::OnShowToolBar(wxCommandEvent &event)
{
	wxString cToolName = wxT("");
	switch(event.GetId())
	{
		case MY1ID_VIEW_FILETOOL:
			cToolName = wxT("fileTool");
			break;
		case MY1ID_VIEW_PROCTOOL:
			cToolName = wxT("fileTool");
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
	int cSelect = event.GetSelection();
	wxMenuBar *cMenuBar = this->GetMenuBar();
	wxWindow *cTarget = mNoteBook->GetPage(cSelect);
	wxString cToolName = wxT("procTool");
	wxAuiPaneInfo& cPane = mMainUI.GetPane(cToolName);
	if(cTarget->IsKindOf(CLASSINFO(my1CodeEdit)))
	{
		cMenuBar->EnableTop(cMenuBar->FindMenu(wxT("Tool")),true);
		cPane.Show().Position(0);
	}
	else
	{
		cMenuBar->EnableTop(cMenuBar->FindMenu(wxT("Tool")),false);
		cPane.Hide();
	}
	mMainUI.Update();
}

void my1Form::OnCodeChanged(wxStyledTextEvent &event)
{
	//DEBUG LINE!
	wxMessageBox(wxT("Okay!"),wxT("Test STC Event"));
}
