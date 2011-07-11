/**
*
* wxform.cpp
*
* - implementation for main wx-based form
*
**/

#include "wxform.hpp"
#include <wx/gbsizer.h>
#include <wx/notebook.h>
#include "wx/artprov.h"
#include "wx/wxhtml.h"
#include "wxcode.hpp"
//#include "wxpref.hpp"
//#include <wx/textfile.h>
//#include <wx/image.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define GRID_VGAP 4
#define GRID_HGAP 10

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
		wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE )
{
	mFaceMan.SetManagedWindow(this);

	// set up default notebook style
	mNoteStyle = wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER;
	mNoteTheme = 0;

	// setup image
	wxInitAllImageHandlers();
	this->SetIcon(wxIcon(wxT(MY1APP_ICON)));
	wxIcon mIconExit(wxT(MY1BITMAP_EXIT));
	wxIcon mIconClear(wxT(MY1BITMAP_NEW));
	wxIcon mIconLoad(wxT(MY1BITMAP_OPEN));
	wxIcon mIconSave(wxT(MY1BITMAP_SAVE));
	wxIcon mIconGenerate(wxT(MY1BITMAP_BINARY));
	wxIcon mIconOptions(wxT(MY1BITMAP_OPTION));

	// menu bar
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(MY1ID_LOAD, wxT("&Load\tF2"));
	fileMenu->Append(MY1ID_SAVE, wxT("&Save\tF3"));
	fileMenu->Append(MY1ID_CLEAR, wxT("&Clear\tF4"));
	fileMenu->AppendSeparator();
	fileMenu->Append(MY1ID_EXIT, wxT("E&xit"), wxT("Quit program"));
	wxMenu *procMenu = new wxMenu;
	procMenu->Append(MY1ID_GENERATE, wxT("&Generate\tF5"));
	procMenu->AppendSeparator();
	procMenu->Append(MY1ID_OPTIONS, wxT("&Options\tF6"));
	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(MY1ID_OPTIONS, wxT("&About"), wxT("About This Program"));
	wxMenuBar *mainMenu = new wxMenuBar;
	mainMenu->Append(fileMenu, wxT("&File"));
	mainMenu->Append(procMenu, wxT("&Tool"));
	mainMenu->Append(helpMenu, wxT("&Help"));
	this->SetMenuBar(mainMenu);

	// status bar
	this->CreateStatusBar(2);
	this->SetStatusText(wxT("Welcome to my1sim85!"));

	// hardcode min size? can we automate this?
	this->SetMinSize(wxSize(WIN_WIDTH,WIN_HEIGHT));

	// tool bar customizations?
	wxAuiToolBarItemArray prepItems;
	wxAuiToolBarItemArray moreItems;
	wxAuiToolBarItem testItem;
	testItem.SetKind(wxITEM_SEPARATOR);
	moreItems.Add(testItem);
	testItem.SetKind(wxITEM_NORMAL);
	testItem.SetId(MY1ID_TOOL_CUSTOMIZE);
	testItem.SetLabel(wxT("Customize..."));
	moreItems.Add(testItem);

	// tool bar - main
	wxAuiToolBar* mainTool = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition,
		wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_OVERFLOW);
	mainTool->SetToolBitmapSize(wxSize(16,16));
	mainTool->AddTool(MY1ID_EXIT, wxT("Exit"), mIconExit);
	mainTool->AddSeparator();
	mainTool->AddTool(MY1ID_CLEAR, wxT("Clear"), mIconClear);
	mainTool->AddTool(MY1ID_LOAD, wxT("Load"), mIconLoad);
	mainTool->AddTool(MY1ID_SAVE, wxT("Save"), mIconSave);
	mainTool->SetCustomOverflowItems(prepItems, moreItems);
	mainTool->Realize();
	mFaceMan.AddPane(mainTool, wxAuiPaneInfo().Name(wxT("mainTool")).
		Caption(wxT("Main Toolbar")).ToolbarPane().Top().
		LeftDockable(false).RightDockable(false));

	// tool bar - process
	wxAuiToolBar* procTool = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition,
		wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_OVERFLOW);
	procTool->SetToolBitmapSize(wxSize(16,16));
	procTool->AddTool(MY1ID_GENERATE, wxT("Generate"), mIconGenerate);
	procTool->AddTool(MY1ID_OPTIONS, wxT("Options"), mIconOptions);
	procTool->SetCustomOverflowItems(prepItems, moreItems);
	procTool->Realize();
	mFaceMan.AddPane(procTool, wxAuiPaneInfo().Name(wxT("procTool")).
		Caption(wxT("Process Toolbar")).ToolbarPane().Top().Position(1).
		LeftDockable(false).RightDockable(false));

	// create view
	wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
	wxPanel *leftPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition,
		wxDefaultSize, wxTAB_TRAVERSAL | wxSUNKEN_BORDER );
	wxGridBagSizer *regSizer = new wxGridBagSizer(GRID_VGAP,GRID_HGAP);
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxT("8085 Register Status")), wxGBPosition(0,0), wxGBSpan(2,5),
		wxALIGN_CENTER | wxALL);
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("RegB")),
				wxGBPosition(2,0));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("RegC")),
				wxGBPosition(2,3));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("RegD")),
				wxGBPosition(3,0));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("RegE")),
				wxGBPosition(3,3));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("RegH")),
				wxGBPosition(4,0));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("RegL")),
				wxGBPosition(4,3));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("RegA")),
				wxGBPosition(5,0));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("Flag")),
				wxGBPosition(5,3));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("Program Counter")),
				wxGBPosition(7,0), wxGBSpan(1,2), wxALIGN_LEFT);
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("Stack Pointer")),
				wxGBPosition(8,0), wxGBSpan(1,2), wxALIGN_LEFT);
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%02X H"), 0x55)), wxGBPosition(2,1));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%02X H"), 0xAA)), wxGBPosition(2,4));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%02X H"), 0x55)), wxGBPosition(3,1));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%02X H"), 0xAA)), wxGBPosition(3,4));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%02X H"), 0x55)), wxGBPosition(4,1));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%02X H"), 0xAA)), wxGBPosition(4,4));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%02X H"), 0x55)), wxGBPosition(5,1));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%02X H"), 0xAA)), wxGBPosition(5,4));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%04X H"), 0x55AA)), wxGBPosition(7,3),
		wxGBSpan(1,2), wxALIGN_LEFT);
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%04X H"), 0x55AA)), wxGBPosition(8,3),
		wxGBSpan(1,2), wxALIGN_LEFT);
	regSizer->Add(GRID_VGAP,GRID_HGAP*3/2, wxGBPosition(2,2));
	regSizer->Add(GRID_VGAP,GRID_HGAP*3/2, wxGBPosition(3,2));
	regSizer->Add(GRID_VGAP,GRID_HGAP*3/2, wxGBPosition(4,2));
	regSizer->Add(GRID_VGAP,GRID_HGAP*3/2, wxGBPosition(5,2));
	regSizer->Add(GRID_VGAP,GRID_HGAP*3/2, wxGBPosition(7,2));
	regSizer->Add(GRID_VGAP,GRID_HGAP*3/2, wxGBPosition(8,2));
	leftSizer->Add(regSizer, wxSizerFlags().Border(wxALL,5));
	//leftSizer->AddStretchSpacer();
	leftPanel->SetSizer(leftSizer);
    mFaceMan.AddPane(leftPanel, wxAuiPaneInfo().Name(wxT("leftPanel")).
		Caption(wxT("Status Panel")).Left());

	// do this off-window (freeze!)
	wxSize clientSize = this->GetClientSize();
	wxAuiNotebook* noteBook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, mNoteStyle);
	noteBook->Freeze();
	wxBitmap pageBMP = wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16,16));
	wxHtmlWindow* mainPage = new wxHtmlWindow(this, wxID_ANY);
	mainPage->SetPage(HTMLIntroduction());
	//mFaceMan.AddPane(mainPage, wxAuiPaneInfo().Caption(wxT("HTML Intro")).Float().FloatingPosition(GetStartPosition()).FloatingSize(320,240));
	noteBook->AddPage(mainPage, wxT("Welcome to wxAUI") , false, pageBMP);
	wxPanel *mainPanel = new wxPanel(noteBook, wxID_ANY);
	wxBoxSizer *testSizer = new wxBoxSizer(wxVERTICAL);
	testSizer->Add(new wxButton(mainPanel, wxID_ANY, wxT("DONE")),
		wxSizerFlags().Center());
	testSizer->Add(new wxButton(mainPanel, wxID_ANY, wxT("CANCEL")),
		wxSizerFlags().Center());
	mainPanel->SetSizer(testSizer);
	noteBook->AddPage(mainPanel, wxT("Welcome"), false);
	noteBook->Thaw();
	mFaceMan.AddPane(noteBook, wxAuiPaneInfo().Name(wxT("CodeBook")).
		CenterPane().PaneBorder(false));

/*
	// create view
	wxBoxSizer *mainSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *codeSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *testSizer = new wxBoxSizer(wxVERTICAL);
	wxPanel *leftPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition,
		wxDefaultSize, wxTAB_TRAVERSAL | wxSUNKEN_BORDER );
	wxPanel *codePanel = new wxPanel(this, MY1ID_CODE_PANEL, wxDefaultPosition,
		wxDefaultSize, wxTAB_TRAVERSAL | wxSUNKEN_BORDER );
	wxPanel *testPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition,
		wxDefaultSize, wxTAB_TRAVERSAL | wxSUNKEN_BORDER );
	wxGridBagSizer *regSizer = new wxGridBagSizer(GRID_VGAP,GRID_HGAP);
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxT("8085 Register Status")), wxGBPosition(0,0), wxGBSpan(2,5),
		wxALIGN_CENTER | wxALL);
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("RegB")),
				wxGBPosition(2,0));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("RegC")),
				wxGBPosition(2,3));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("RegD")),
				wxGBPosition(3,0));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("RegE")),
				wxGBPosition(3,3));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("RegH")),
				wxGBPosition(4,0));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("RegL")),
				wxGBPosition(4,3));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("RegA")),
				wxGBPosition(5,0));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("Flag")),
				wxGBPosition(5,3));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("Program Counter")),
				wxGBPosition(7,0), wxGBSpan(1,2), wxALIGN_LEFT);
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY, wxT("Stack Pointer")),
				wxGBPosition(8,0), wxGBSpan(1,2), wxALIGN_LEFT);
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%02X H"), 0x55)), wxGBPosition(2,1));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%02X H"), 0xAA)), wxGBPosition(2,4));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%02X H"), 0x55)), wxGBPosition(3,1));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%02X H"), 0xAA)), wxGBPosition(3,4));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%02X H"), 0x55)), wxGBPosition(4,1));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%02X H"), 0xAA)), wxGBPosition(4,4));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%02X H"), 0x55)), wxGBPosition(5,1));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%02X H"), 0xAA)), wxGBPosition(5,4));
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%04X H"), 0x55AA)), wxGBPosition(7,3),
		wxGBSpan(1,2), wxALIGN_LEFT);
	regSizer->Add(new wxStaticText(leftPanel, wxID_ANY,
		wxString::Format(wxT("%04X H"), 0x55AA)), wxGBPosition(8,3),
		wxGBSpan(1,2), wxALIGN_LEFT);
	regSizer->Add(GRID_VGAP,GRID_HGAP*3/2, wxGBPosition(2,2));
	regSizer->Add(GRID_VGAP,GRID_HGAP*3/2, wxGBPosition(3,2));
	regSizer->Add(GRID_VGAP,GRID_HGAP*3/2, wxGBPosition(4,2));
	regSizer->Add(GRID_VGAP,GRID_HGAP*3/2, wxGBPosition(5,2));
	regSizer->Add(GRID_VGAP,GRID_HGAP*3/2, wxGBPosition(7,2));
	regSizer->Add(GRID_VGAP,GRID_HGAP*3/2, wxGBPosition(8,2));
	leftSizer->Add(regSizer, wxSizerFlags().Border(wxALL,5));
	//leftSizer->AddStretchSpacer();
	leftPanel->SetSizer(leftSizer);
	mainSizer->Add(leftPanel, wxSizerFlags().Align(wxALIGN_LEFT).Expand());
	// mid is text editor?
	wxNotebook *codeBook = new wxNotebook(codePanel, MY1ID_CODE_BOOK);
	codeSizer->Add(codeBook, 1, wxGROW);
	codePanel->SetSizer(codeSizer);
	mainSizer->Add(codePanel, wxSizerFlags(1).Expand().Border(wxALL,0));
	// right panel
	testSizer->Add(new wxButton(testPanel, wxID_ANY, wxT("DONE")),
		wxSizerFlags().Center());
	testSizer->Add(new wxButton(testPanel, wxID_ANY, wxT("CANCEL")),
		wxSizerFlags().Center());
	testPanel->SetSizer(testSizer);
	mainSizer->Add(testPanel, wxSizerFlags().Align(wxALIGN_RIGHT).Expand());
	this->SetSizer(mainSizer);
	this->Layout();
	mainSizer->Fit(this);
	mainSizer->SetSizeHints(this);
	this->SetSize(WIN_WIDTH,WIN_HEIGHT);
*/

	// actions!
	this->Connect(MY1ID_EXIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnQuit));
	this->Connect(MY1ID_CLEAR, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnClear));
	this->Connect(MY1ID_LOAD, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnLoad));
	this->Connect(MY1ID_SAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnSave));
	this->Connect(MY1ID_GENERATE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnGenerate));
	this->Connect(MY1ID_OPTIONS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnCheckOptions));
	//this->Connect(MY1ID_CODEPAGE_CHANGE, wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING, wxNotebookEventHandler(my1Form::OnCodePageChange));

		// gui events
	this->Connect(wxEVT_PAINT, wxPaintEventHandler(my1Form::OnPaint));
	this->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(my1Form::OnMouseClick));
	this->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(my1Form::OnMouseClick));
	this->Connect(wxEVT_MOTION, wxMouseEventHandler(my1Form::OnMouseMove));
	this->Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(my1Form::OnMouseLeave));

	// position this!
	this->Centre();
	mFaceMan.Update();
}

my1Form::~my1Form()
{
	mFaceMan.UnInit();
}

wxWindow* my1Form::GetCodeBook(void)
{
	wxWindow *cTemp = this->GetChildID(this,MY1ID_CODE_PANEL);
	if(!cTemp)
		return cTemp;
	return this->GetChildID(cTemp,MY1ID_CODE_BOOK);
}

wxWindow* my1Form::GetChildID(wxWindow* aParent, int anID)
{
	wxWindow *cTemp = 0x0;
	wxWindowList& cList = aParent->GetChildren();
	wxWindowList::iterator cWindow;
	for(cWindow=cList.begin();cWindow!=cList.end();cWindow++)
	{
		wxWindow *cTest = (wxWindow*) *cWindow;
		if(cTest->GetId()==anID)
		{
			cTemp = cTest;
			break;
		}
	}
	return cTemp;
}

void my1Form::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

void my1Form::OnClear(wxCommandEvent& WXUNUSED(event))
{
	// this
}

void my1Form::OnLoad(wxCommandEvent& WXUNUSED(event))
{
	wxNotebook *cCodeBook = (wxNotebook*) this->GetCodeBook();
	wxFileDialog *cSelect = new wxFileDialog(this,wxT("Select code file"),
		wxT(""),wxT(""),wxT("Any file (*.*)|*.*"),
		wxFD_DEFAULT_STYLE|wxFD_FILE_MUST_EXIST|wxFD_CHANGE_DIR);
	cSelect->SetWildcard("ASM files (*.asm)|*.asm|8085 ASM files (*.8085)|*.8085|Any file (*.*)|*.*");
	if(cSelect->ShowModal()!=wxID_OK)
		return;
	my1CodeEdit *cCodeEdit = new my1CodeEdit(cCodeBook, wxID_ANY, cSelect->GetPath());
	cCodeBook->AddPage(cCodeEdit, cCodeEdit->GetFileName());
}

void my1Form::OnSave(wxCommandEvent& WXUNUSED(event))
{
	wxNotebook *cCodeBook = (wxNotebook*) this->GetCodeBook();
	my1CodeEdit *cCodeEdit = (my1CodeEdit*) cCodeBook->GetCurrentPage();
	if(!cCodeEdit->GetModify())
		return;
	cCodeEdit->SaveFile(cCodeEdit->GetFullName());
}

void my1Form::OnGenerate(wxCommandEvent& WXUNUSED(event))
{
	// this
}

void my1Form::OnCheckOptions(wxCommandEvent &event)
{
	// this
}

/*
void my1Form::OnCodePageChange(wxNotebookEvent &event)
{
	wxNotebook *cCodeBook = (wxNotebook*) this->GetCodeBook();
	if(!cCodeBook)
	{
		wxMessageBox(wxT("Cannot get CodeBook!"), wxT("Error!"));
		return;
	}
	wxTextCtrl *cTest = (wxTextCtrl*) cCodeBook->GetCurrentPage();
	if(!cTest)
	{
		wxMessageBox(wxT("Cannot get current page?!"), wxT("Error!"));
		return;
	}
	if(cTest->IsModified())
	{
		int cAnswer = wxMessageBox(wxT("Save code?!"), wxT("Code modified!"), wxOK|wxCANCEL|wxICON_QUESTION);
		if(cAnswer==wxID_CANCEL)
		{
			event.Veto();
			return;
		}
		// how to save??? need to get full pathname!
	}
}
*/

void my1Form::OnPaint(wxPaintEvent& event)
{
	// this
}

void my1Form::OnMouseClick(wxMouseEvent &event)
{
/*
	// this
	def OnClick(evt):
	text = evt.GetEventObject()
	if text.GetInsertionPoint() <= text.GetLastPosition():
	start, end = text.GetSelection()
	input = text.GetValue()
	if start > 0:
	start = input.rfind('\n', 0, start)
	if start == -1:
	start = 0
	else:
	start = start + 1
	if end < text.GetLastPosition():
	end = input.find('\n', end)
	if end == -1:
	end = text.GetLastPosition()
	else:
	end = end + 1
	if start < end:
	text.SetSelection(start, end)
	*/
}

void my1Form::OnMouseMove(wxMouseEvent &event)
{
	// this
}

void my1Form::OnMouseLeave(wxMouseEvent &event)
{
	// this
}

wxString my1Form::HTMLIntroduction(void)
{
	const char* cHTMLText =
		"<html><body>"
		"<h1>Welcome to my1Sim85</h1>"
		"<br/><b>Overview</b><br/>"
		"<p>Whatever it is...</p>"
		"<p>See README.txt for more information.</p>"
		"</body></html>";
	return wxString::FromAscii(cHTMLText);
}
