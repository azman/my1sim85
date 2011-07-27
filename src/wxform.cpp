/**
*
* wxform.cpp
*
* - implementation for main wx-based form
*
**/

#include "wxform.hpp"
#include "wxcode.hpp"
//#include <wx/event.h>
//#include <wx/gbsizer.h>
//#include <wx/notebook.h>
//#include "wx/artprov.h"
//#include "wx/wxhtml.h"
//#include <wx/textfile.h>
//#include <wx/image.h>
//#include <wx/string.h>
//#include <wx/intl.h>
//#include <wx/bitmap.h>

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
	wxMenu *viewMenu = new wxMenu;
	viewMenu->Append(MY1ID_VIEW_INITPAGE, wxT("View Welcome Page"));
	viewMenu->Append(MY1ID_VIEW_INFOPANE, wxT("View Info Panel"));
	viewMenu->Append(MY1ID_VIEW_CONSPANE, wxT("View Console Panel"));
	viewMenu->Append(MY1ID_VIEW_LOGSPANE, wxT("View Log Panel"));
	wxMenu *procMenu = new wxMenu;
	procMenu->Append(MY1ID_ASSEMBLE, wxT("&Assemble\tF5"));
	procMenu->AppendSeparator();
	procMenu->Append(MY1ID_OPTIONS, wxT("&Options\tF6"));
	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(MY1ID_ABOUT, wxT("&About"), wxT("About This Program"));
	wxMenuBar *mainMenu = new wxMenuBar;
	mainMenu->Append(fileMenu, wxT("&File"));
	mainMenu->Append(viewMenu, wxT("&View"));
	mainMenu->Append(procMenu, wxT("&Tool"));
	mainMenu->Append(helpMenu, wxT("&Help"));
	this->SetMenuBar(mainMenu);

	// using AUI manager...
	mMainUI.SetManagedWindow(this);
	// using oncreate methods
	wxCommandEvent dummyEvent;
	// create initial pane for main view
	this->CreateInitPanel();
	// tool bar - file
	this->OnCreateFileTool(dummyEvent);
	// tool bar - proc
	this->OnCreateProcTool(dummyEvent);
	// info panel
	this->OnCreateInfoPanel(dummyEvent);
	// console panel
	this->OnCreateConsPanel(dummyEvent);
	// logs & alert panel
	this->OnCreateLogsPanel(dummyEvent);
	// commit changes!
	mMainUI.Update();

	// actions!
	this->Connect(MY1ID_EXIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnQuit));
	this->Connect(MY1ID_LOAD, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnLoad));
	this->Connect(MY1ID_SAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnSave));
	this->Connect(MY1ID_VIEW_INITPAGE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnCreateInitPage));
	this->Connect(MY1ID_VIEW_INFOPANE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnCreateInfoPanel));
	this->Connect(MY1ID_VIEW_CONSPANE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnCreateConsPanel));
	this->Connect(MY1ID_VIEW_LOGSPANE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnCreateLogsPanel));
	this->Connect(wxID_ANY, wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler(my1Form::OnClosePane));
	this->Connect(MY1ID_OPTIONS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnCheckOptions));

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
	wxAuiToolBar* fileTool = new wxAuiToolBar(this, MY1ID_FILETOOLBAR, wxDefaultPosition,
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

wxAuiToolBar* my1Form::CreateProcToolBar(void)
{
	wxIcon mIconAssemble(wxT(MY1BITMAP_BINARY));
	wxIcon mIconOptions(wxT(MY1BITMAP_OPTION));
	wxAuiToolBar* procTool = new wxAuiToolBar(this, MY1ID_PROCTOOLBAR, wxDefaultPosition,
		wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	procTool->SetToolBitmapSize(wxSize(16,16));
	procTool->AddTool(MY1ID_ASSEMBLE, wxT("Assemble"), mIconAssemble);
	procTool->AddTool(MY1ID_OPTIONS, wxT("Options"), mIconOptions);
	procTool->Realize();
	return procTool;
}

void my1Form::CreateInitPanel(void)
{
	// do this off-window? (freeze!)
	mNoteBook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE);
	//mNoteBook->Freeze();
	mNoteBook->AddPage(CreateMainPanel(mNoteBook), wxT("Welcome"), false);
	//mNoteBook->Thaw();
	mMainUI.AddPane(mNoteBook, wxAuiPaneInfo().Name(wxT("codeBook")).
		CenterPane().Layer(3).CloseButton(false).MaximizeButton(true).PaneBorder(false));
}

wxPanel* my1Form::CreateMainPanel(wxWindow *parent)
{
	wxPanel *cPanel = new wxPanel(parent, wxID_ANY);
	// welcome panel?
	return cPanel;
}

wxPanel* my1Form::CreateInfoPanel(void)
{
	wxPanel *cPanel = new wxPanel(this, MY1ID_INFOPANEL,
		wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	cPanel->SetMinSize(wxSize(INFO_PANEL_WIDTH,0));
	return cPanel;
}

wxPanel* my1Form::CreateConsPanel(void)
{
	wxPanel *cPanel = new wxPanel(this, MY1ID_CONSPANEL,
		wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    cPanel->SetMinSize(wxSize(0,CONS_PANEL_HEIGHT));

	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxVERTICAL);
	wxTextCtrl *cConsole = new wxTextCtrl(cPanel, wxID_ANY, wxT("Welcome to MY1Sim85"),
		wxDefaultPosition, wxDefaultSize, wxTE_AUTO_SCROLL|wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator);
	cBoxSizer->Add(cConsole, 1, wxEXPAND);
	wxBoxSizer *dBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	wxPanel *cComPanel = new wxPanel(cPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxTextCtrl *cCommandText = new wxTextCtrl(cComPanel, wxID_ANY, wxT(""),
		wxDefaultPosition, wxDefaultSize);
	wxButton *cButton = new wxButton(cComPanel, wxID_ANY, wxT("Execute"));
	dBoxSizer->Add(cCommandText, 1, wxEXPAND);
	dBoxSizer->Add(cButton, 0, wxALIGN_RIGHT);
	cComPanel->SetSizer(dBoxSizer);
	dBoxSizer->Fit(cComPanel);
	dBoxSizer->SetSizeHints(cComPanel);
	cBoxSizer->Add(cComPanel, 0, wxALIGN_BOTTOM|wxEXPAND);
	cPanel->SetSizer(cBoxSizer);
	cBoxSizer->Fit(cPanel);
	cBoxSizer->SetSizeHints(cPanel);

	return cPanel;
}

wxPanel* my1Form::CreateLogsPanel(void)
{
	wxPanel *cPanel = new wxPanel(this, MY1ID_INFOPANEL,
		wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	cPanel->SetMinSize(wxSize(INFO_PANEL_WIDTH,0));

	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	wxNotebook *cLogBook = new wxNotebook(cPanel, MY1ID_LOGBOOK, wxDefaultPosition, wxDefaultSize);
	wxPanel *cChkPanel = new wxPanel(cLogBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	cLogBook->AddPage(cChkPanel, wxT("Assembler Log"), false);
	cBoxSizer->Add(cLogBook, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
	cPanel->SetSizer(cBoxSizer);
    cBoxSizer->SetSizeHints(cPanel);

	return cPanel;
}

void my1Form::OpenEdit(wxString& cFileName)
{
	my1CodeEdit *cCodeEdit = new my1CodeEdit(mNoteBook, wxID_ANY, cFileName, this->mOptions);
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

void my1Form::OnMouseClick(wxMouseEvent &event)
{
	// get event location
	//wxPoint pos = event.GetPosition();
	//if(event.LeftDown())
	//else if(event.RightDown())
}

void my1Form::OnClosePane(wxAuiManagerEvent &event)
{
	//wxAuiPaneInfo *cPane = event.GetPane();
	//cPane->Hide();
	//mMainUI.Update();
	//event.Veto();
}

void my1Form::OnCreateInitPage(wxCommandEvent &event)
{
	bool cNotFound = true;
	int cCount = mNoteBook->GetPageCount();
	for(int cLoop=0;cLoop<cCount;cLoop++)
	{
		// set for all opened editor?
		wxWindow *cTarget = mNoteBook->GetPage(cLoop);
		if(!cTarget->IsKindOf(CLASSINFO(my1CodeEdit)))
		{
			cNotFound = false;
		}
	}
	if(cNotFound)
		mNoteBook->AddPage(CreateMainPanel(mNoteBook), wxT("Welcome"), false);
	return;
}

void my1Form::OnCreateInfoPanel(wxCommandEvent &event)
{
	wxString cPanelName = wxT("infoPanel");
	wxAuiPaneInfo cPane = mMainUI.GetPane(cPanelName);
	if(cPane.IsOk()) return;
	// have to destroy-on-close, hide/show doesn't work!
	mMainUI.AddPane(CreateInfoPanel(), wxAuiPaneInfo().Name(cPanelName).
		Caption(wxT("Information Panel")).DefaultPane().Layer(2).Left().
		TopDockable(false).BottomDockable(false).RightDockable(false).
		MinSize(wxSize(INFO_PANEL_WIDTH,0)).DestroyOnClose());
	if(event.GetEventType()!=wxEVT_NULL) mMainUI.Update();
	return;
}

void my1Form::OnCreateConsPanel(wxCommandEvent &event)
{
	wxString cPanelName = wxT("consPanel");
	wxAuiPaneInfo cPane = mMainUI.GetPane(cPanelName);
	if(cPane.IsOk()) return;
	// have to destroy-on-close, hide/show doesn't work!
	mMainUI.AddPane(CreateConsPanel(), wxAuiPaneInfo().Name(cPanelName).
		Caption(wxT("Console Panel")).CaptionVisible().Bottom().
		TopDockable(false).LeftDockable(false).RightDockable(false).
		MinSize(wxSize(0,CONS_PANEL_HEIGHT)).DestroyOnClose());
	if(event.GetEventType()!=wxEVT_NULL) mMainUI.Update();
	return;
}

void my1Form::OnCreateLogsPanel(wxCommandEvent &event)
{
	wxString cPanelName = wxT("logsPanel");
	wxAuiPaneInfo cPane = mMainUI.GetPane(cPanelName);
	if(cPane.IsOk()) return;
	// have to destroy-on-close, hide/show doesn't work!
	mMainUI.AddPane(CreateLogsPanel(), wxAuiPaneInfo().Name(wxT("logsPanel")).
		Caption(wxT("Logs Panel")).CaptionVisible().Bottom().Position(1).
		TopDockable(false).LeftDockable(false).RightDockable(false).
		MinSize(wxSize(0,CONS_PANEL_HEIGHT)).DestroyOnClose());
	if(event.GetEventType()!=wxEVT_NULL) mMainUI.Update();
	return;
}

void my1Form::OnCreateFileTool(wxCommandEvent &event)
{
	wxString cToolName = wxT("fileTool");
	wxAuiPaneInfo cPane = mMainUI.GetPane(cToolName);
	if(cPane.IsOk()) return;
	mMainUI.AddPane(CreateFileToolBar(), wxAuiPaneInfo().Name(cToolName).
		Caption(wxT("Main Toolbar")).ToolbarPane().Top().
		LeftDockable(false).RightDockable(false).DestroyOnClose());
	if(event.GetEventType()!=wxEVT_NULL) mMainUI.Update();
	return;
}

void my1Form::OnCreateProcTool(wxCommandEvent &event)
{
	wxString cToolName = wxT("procTool");
	wxAuiPaneInfo cPane = mMainUI.GetPane(cToolName);
	if(cPane.IsOk()) return;
	mMainUI.AddPane(CreateProcToolBar(), wxAuiPaneInfo().Name(wxT("procTool")).
		Caption(wxT("Process Toolbar")).ToolbarPane().Top().Position(1).
		LeftDockable(false).RightDockable(false).DestroyOnClose());
	if(event.GetEventType()!=wxEVT_NULL) mMainUI.Update();
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
		//DEBUG LINE! //wxMessageBox(wxT("Okay!"),wxT("Test"));
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

void my1Form::ShowStatus(wxString &aString)
{
	this->SetStatusText(aString,STATUS_MSG_INDEX);
	wxTimer* cTimer = new wxTimer(this, MY1ID_STAT_TIMER);
	cTimer->Start(STATUS_MSG_PERIOD,wxTIMER_ONE_SHOT); // will restart if already running!
	this->Connect(MY1ID_STAT_TIMER, wxEVT_TIMER, wxTimerEventHandler(my1Form::OnStatusTimer));
}

void my1Form::OnStatusTimer(wxTimerEvent& event)
{
	wxTimer* cTimer = &event.GetTimer();
	delete cTimer;
	this->SetStatusText(wxT(""),STATUS_MSG_INDEX);
}
