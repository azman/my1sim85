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
	// setup image
	wxInitAllImageHandlers();
	SetIcon(wxIcon(wxT(MY1APP_ICON)));
	wxIcon mIconExit(wxT(MY1BITMAP_EXIT));
	wxIcon mIconClear(wxT(MY1BITMAP_NEW));
	wxIcon mIconLoad(wxT(MY1BITMAP_OPEN));
	wxIcon mIconSave(wxT(MY1BITMAP_SAVE));
	wxIcon mIconGenerate(wxT(MY1BITMAP_BINARY));
	wxIcon mIconOptions(wxT(MY1BITMAP_OPTION));

	// tool bar
	wxToolBar *mainTool = this->CreateToolBar();
	// our icon is 16x16, windows defaults to 24x24
	mainTool->SetToolBitmapSize(wxSize(16,16));
	mainTool->AddTool(MY1ID_EXIT, wxT("Exit"), mIconExit);
	mainTool->AddSeparator();
	mainTool->AddTool(MY1ID_CLEAR, wxT("Clear"), mIconClear);
	mainTool->AddTool(MY1ID_LOAD, wxT("Load"), mIconLoad);
	mainTool->AddTool(MY1ID_SAVE, wxT("Save"), mIconSave);
	mainTool->AddSeparator();
	mainTool->AddTool(MY1ID_GENERATE, wxT("Generate"), mIconGenerate);
	mainTool->AddTool(MY1ID_OPTIONS, wxT("Options"), mIconOptions);
	mainTool->Realize();

	// menu bar
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(MY1ID_LOAD, wxT("&Load\tF2"));
	fileMenu->Append(MY1ID_SAVE, wxT("&Save\tF3"));
	fileMenu->Append(MY1ID_CLEAR, wxT("&Clear\tF4"));
	fileMenu->AppendSeparator();
	fileMenu->Append(MY1ID_GENERATE, wxT("&Generate\tF5"));
	fileMenu->Append(MY1ID_OPTIONS, wxT("&Options\tF6"));
	fileMenu->AppendSeparator();
	fileMenu->Append(MY1ID_EXIT, wxT("E&xit"), wxT("Quit program"));
	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(MY1ID_OPTIONS, wxT("&About"), wxT("About This Program"));
	wxMenuBar *mainMenu = new wxMenuBar;
	mainMenu->Append(fileMenu, _T("&File"));
	mainMenu->Append(helpMenu, _T("&Help"));
	this->SetMenuBar(mainMenu);

	// status bar
	this->CreateStatusBar(2);
	this->SetStatusText(wxT("Welcome to my1sim85!"));

	// create view
	wxBoxSizer *mainSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *codeSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *testSizer = new wxBoxSizer(wxVERTICAL);
    wxPanel *leftPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition,
		wxDefaultSize, wxTAB_TRAVERSAL | wxSUNKEN_BORDER );
    wxPanel *codePanel = new wxPanel(this, wxID_ANY, wxDefaultPosition,
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
	wxNotebook *codeBook = new wxNotebook(codePanel, wxID_ANY);
	codeSizer->Add(codeBook, 1, wxGROW);
	// example how to display code load from file
	wxTextCtrl *mainText = new wxTextCtrl(codeBook, wxID_ANY,
		wxT("Text Editor!"),wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE); // |wxTE_PROCESS_TAB
	codeBook->AddPage(mainText, wxT("Main Code"));
	wxPanel *notePanel = new wxPanel(codeBook, wxID_ANY);
	codeBook->AddPage(notePanel, wxT("Options Panel"));
	wxSizer *notePanelSizer = new wxBoxSizer(wxVERTICAL);
    wxTextCtrl *codeText = new wxTextCtrl(notePanel, wxID_ANY, wxT("TextLine 1."), wxDefaultPosition, wxSize(250,wxDefaultCoord));
    notePanelSizer->Add(codeText, 0, wxGROW|wxALL, 10);
    codeText = new wxTextCtrl(notePanel, wxID_ANY, wxT("TextLine 2."), wxDefaultPosition, wxSize(250,wxDefaultCoord));
    notePanelSizer->Add(codeText, 0, wxGROW|wxALL, 10);
    wxButton *codeButton = new wxButton(notePanel, wxID_ANY, wxT("Done"));
    notePanelSizer->Add(codeButton, 0, wxALIGN_RIGHT|wxLEFT|wxRIGHT|wxBOTTOM, 10);
    notePanel->SetAutoLayout(true);
    notePanel->SetSizer(notePanelSizer);
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

	// gui events
	this->Connect(wxEVT_PAINT, wxPaintEventHandler(my1Form::OnPaint));
	this->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(my1Form::OnMouseClick));
	this->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(my1Form::OnMouseClick));
	this->Connect(wxEVT_MOTION, wxMouseEventHandler(my1Form::OnMouseMove));
	this->Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(my1Form::OnMouseLeave));

	// actions!
	this->Connect(MY1ID_EXIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnQuit));
	this->Connect(MY1ID_CLEAR, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnClear));
	this->Connect(MY1ID_LOAD, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnLoad));
	this->Connect(MY1ID_SAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnSave));
	this->Connect(MY1ID_GENERATE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnGenerate));
	this->Connect(MY1ID_OPTIONS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnCheckOptions));

	// position this!
	this->Centre();
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
	//wxString cFileName = wxFileSelector(_T("Select code file"));
	//if(!cFileName)
	//	return;
}

void my1Form::OnSave(wxCommandEvent& WXUNUSED(event))
{
	// this
}

void my1Form::OnGenerate(wxCommandEvent& WXUNUSED(event))
{
	// this
}

void my1Form::OnCheckOptions(wxCommandEvent &event)
{
	// this
}

void my1Form::OnPaint(wxPaintEvent& event)
{
	// this
}

void my1Form::OnMouseClick(wxMouseEvent &event)
{
	// this
}

void my1Form::OnMouseMove(wxMouseEvent &event)
{
	// this
}

void my1Form::OnMouseLeave(wxMouseEvent &event)
{
	// this
}
