/**
*
* wxform.cpp
*
* - implementation for main wx-based form
*
**/

#include "wxform.hpp"
#include <wx/gbsizer.h>
//#include "wxpref.hpp"
//#include <wx/textfile.h>
//#include <wx/image.h>

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
	wxGridBagSizer *regSizer = new wxGridBagSizer();
	regSizer->Add(new wxStaticText(this, wxID_ANY, wxT("8085 Register Status")),
				wxGBPosition(0,0), wxGBSpan(1,4),
				wxALIGN_CENTER | wxALL);
	regSizer->Add(wxT("Reg B"), wxGBPosition(1,0));
	regSizer->Add(new wxTextCtrl(this, wxID_ANY,
			wxString::Format(wxT("%08d"), 0x55)), wxGBPosition(1,1));
	regSizer->Add(new wxTextCtrl(this, wxID_ANY,
			wxT("Reg C")), wxGBPosition(1,2));
	regSizer->Add(new wxTextCtrl(this, wxID_ANY,
			wxString::Format(wxT("%02xH"), 0xAA)), wxGBPosition(1,3));
	regSizer->Add(new wxTextCtrl(this, wxID_ANY,
			wxT("Reg A")), wxGBPosition(2,0));
	regSizer->Add(new wxTextCtrl(this, wxID_ANY,
			wxString::Format(wxT("%08d"), 0x55)), wxGBPosition(2,1));
	regSizer->Add(new wxTextCtrl(this, wxID_ANY,
			wxT("Reg F")), wxGBPosition(2,2));
	regSizer->Add(new wxTextCtrl(this, wxID_ANY,
			wxString::Format(wxT("%02xH"), 0xAA)), wxGBPosition(2,3));
	leftSizer->Add(regSizer, wxSizerFlags(1).Expand().Border(wxALL));
	mainSizer->Add(leftSizer, wxSizerFlags().Align(wxALIGN_LEFT).Border(wxALL & ~wxRIGHT, 5));
	// mid is text editor?
	//wxPanel *leftPanel = new wxPanel(this,-1,wxDefaultPosition,wxDefaultSize,
	//	wxTAB_TRAVERSAL | wxSUNKEN_BORDER);
	wxTextCtrl *mainText = new wxTextCtrl(this, wxID_ANY,
		wxT("Text Editor!"),wxDefaultPosition, wxSize(100,60), wxTE_MULTILINE);
	mainSizer->Add(mainText, wxSizerFlags(1).Expand().Border(wxALL, 5));
	// right panel
	wxBoxSizer *buttSizer = new wxBoxSizer(wxVERTICAL);
	buttSizer->Add(new wxButton(this, wxID_ANY, wxT("Two buttons in a box")),
		wxSizerFlags().Border(wxALL, 7));
	buttSizer->Add(new wxButton(this, wxID_ANY, wxT("(wxCENTER)")),
		wxSizerFlags().Border(wxALL, 7));
	mainSizer->Add(buttSizer, wxSizerFlags().Align(wxALIGN_RIGHT).Border(wxALL & ~wxLEFT, 5));
	this->SetSizer(mainSizer);
	this->Layout();
	mainSizer->Fit(this);
	mainSizer->SetSizeHints(this);

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
	// this
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
}

void my1Form::OnMouseClick(wxMouseEvent &event)
{
}

void my1Form::OnMouseMove(wxMouseEvent &event)
{
}

void my1Form::OnMouseLeave(wxMouseEvent &event)
{
}
