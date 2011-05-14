/**
*
* wxform.cpp
*
* - implementation for main wx-based form
*
**/

#include "wxform.hpp"
//#include "wxpref.hpp"
//#include <wx/textfile.h>
#include <wx/statline.h>
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
	: wxFrame( NULL, wxID_ANY, title, wxDefaultPosition, wxSize(320, 240), wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX) )
{
	wxInitAllImageHandlers();
	SetIcon(wxIcon(wxT(MY1APP_ICON)));
	wxIcon mIconExit(wxT(MY1BITMAP_EXIT));
	wxIcon mIconClear(wxT(MY1BITMAP_NEW));
	wxIcon mIconLoad(wxT(MY1BITMAP_OPEN));
	wxIcon mIconSave(wxT(MY1BITMAP_SAVE));
	wxIcon mIconGenerate(wxT(MY1BITMAP_BINARY));
	wxIcon mIconOptions(wxT(MY1BITMAP_OPTION));

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

	this->CreateStatusBar(2);
	this->SetStatusText(wxT("Welcome to my1sim85!"));

	wxPanel* mainPanel = new wxPanel(this, wxID_ANY);
	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	wxStaticText *mainLabel = new wxStaticText(mainPanel, wxID_ANY,
		wxT("This is a label!"));
	wxTextCtrl *mainText = new wxTextCtrl(mainPanel, wxID_ANY,
		wxT("Edit Text!"),wxDefaultPosition, wxSize(100,60), wxTE_MULTILINE);
	topSizer->Add(mainLabel,
		wxSizerFlags().Align(wxALIGN_RIGHT).Border(wxALL & ~wxBOTTOM, 5));
	topSizer->Add(mainText,
		wxSizerFlags(1).Expand().Border(wxALL, 5));
	wxBoxSizer *statSizer = new wxStaticBoxSizer(
		new wxStaticBox(mainPanel, wxID_ANY, wxT("StaticBoxSizer")), wxVERTICAL);
	statSizer->Add(new wxStaticText(mainPanel, wxID_ANY, wxT("And some TEXT inside it")),
		wxSizerFlags().Center().Border(wxALL, 30));
	topSizer->Add(statSizer, wxSizerFlags(1).Expand().Border(wxALL, 10));
	wxGridSizer *gridSizer = new wxGridSizer(2, 5, 5);
	gridSizer->Add(new wxStaticText(mainPanel, wxID_ANY, wxT("Label")),
				   wxSizerFlags().Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL));
	gridSizer->Add(new wxTextCtrl(mainPanel, wxID_ANY, wxT("Grid sizer demo")),
				   wxSizerFlags(1).Align(wxGROW | wxALIGN_CENTER_VERTICAL));
	gridSizer->Add(new wxStaticText(mainPanel, wxID_ANY, wxT("Another label")),
				   wxSizerFlags().Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL));
	gridSizer->Add(new wxTextCtrl(mainPanel, wxID_ANY, wxT("More text")),
				   wxSizerFlags(1).Align(wxGROW | wxALIGN_CENTER_VERTICAL));
	gridSizer->Add(new wxStaticText(mainPanel, wxID_ANY, wxT("Final label")),
				   wxSizerFlags().Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL));
	gridSizer->Add(new wxTextCtrl(mainPanel, wxID_ANY, wxT("And yet more text")),
				   wxSizerFlags().Align(wxGROW | wxALIGN_CENTER_VERTICAL));
	topSizer->Add(gridSizer, wxSizerFlags().Proportion(1).Expand().Border(wxALL, 10));
	topSizer->Add(new wxStaticLine(mainPanel, wxID_ANY, wxDefaultPosition, wxSize(3,3), wxHORIZONTAL),
		wxSizerFlags().Expand());
	wxBoxSizer *buttSizer = new wxBoxSizer(wxHORIZONTAL);
	buttSizer->Add(new wxButton(mainPanel, wxID_ANY, wxT("Two buttons in a box")),
		wxSizerFlags().Border(wxALL, 7));
	buttSizer->Add(new wxButton(mainPanel, wxID_ANY, wxT("(wxCENTER)")),
		wxSizerFlags().Border(wxALL, 7));
	topSizer->Add(buttSizer, wxSizerFlags().Center());
	mainPanel->SetSizer(topSizer);
	topSizer->SetSizeHints( this );

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
