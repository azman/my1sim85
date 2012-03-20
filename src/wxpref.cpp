/**
*
* wxpref.cpp
*
* - implementation for wx-based options dialog
*
**/

#include "wxpref.hpp"

my1OptionDialog::my1OptionDialog(wxWindow *parent, const wxString &title, my1Options &options)
	: wxDialog( parent, wxID_ANY, title ), mParentOptions(options)
{
	// why do this? filter!
	mCurrentOptions = mParentOptions;
	// try small fonts
	wxFont cTestFont(8,wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	this->SetFont(cTestFont);
	// main view - preference book
	mPrefBook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	// add all panels to book
	mPrefBook->AddPage(CreateEditPanel(), wxT("Editing"), false);
	mPrefBook->AddPage(CreateSimsPanel(), wxT("Simulation"), false);
	// main box-sizer
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->Add(mPrefBook, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
	this->SetSizer(cBoxSizer);
	this->Fit();
	this->Centre();
	// events & actions
	this->Connect(MY1ID_PREF_SAVE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1OptionDialog::OnOptSave));
	this->Connect(MY1ID_PREF_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1OptionDialog::OnOptClose));
	this->Connect(MY1ID_PREF_DUMMY, wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(my1OptionDialog::OnOptCheck));
	this->Connect(MY1ID_PREF_VIEWWS, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(my1OptionDialog::OnOptCheck));
	this->Connect(MY1ID_PREF_VIEWEOL, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(my1OptionDialog::OnOptCheck));
	this->Connect(MY1ID_PREF_UNIXEOL, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(my1OptionDialog::OnOptCheck));
	this->Connect(MY1ID_PREF_FREERUN, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(my1OptionDialog::OnOptCheck));
	this->Connect(MY1ID_PREF_RUNINFO, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(my1OptionDialog::OnOptCheck));
	this->Connect(MY1ID_PREF_STARTADDR, wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(my1OptionDialog::OnOptCheck));
	// update with current options
	wxCheckBox *cBoxViewWS = (wxCheckBox*) this->FindWindow(MY1ID_PREF_VIEWWS);
	cBoxViewWS->SetValue(mCurrentOptions.mEdit_ViewWS);
	wxCheckBox *cBoxViewEOL = (wxCheckBox*) this->FindWindow(MY1ID_PREF_VIEWEOL);
	cBoxViewEOL->SetValue(mCurrentOptions.mEdit_ViewEOL);
	wxCheckBox *cBoxSimFreeRun = (wxCheckBox*) this->FindWindow(MY1ID_PREF_FREERUN);
	cBoxSimFreeRun->SetValue(mCurrentOptions.mSims_FreeRunning);
	wxCheckBox *cBoxSimRunInfo = (wxCheckBox*) this->FindWindow(MY1ID_PREF_RUNINFO);
	cBoxSimRunInfo->SetValue(mCurrentOptions.mSims_ShowRunInfo);
	wxTextCtrl *cTextStartADDR = (wxTextCtrl*) this->FindWindow(MY1ID_PREF_STARTADDR);
	cTextStartADDR->ChangeValue(wxString::Format("%04X",mCurrentOptions.mSims_StartADDR));
}

wxPanel* my1OptionDialog::CreateEditPanel(void)
{
	wxPanel *cPanel = new wxPanel(mPrefBook);
	wxStaticBoxSizer* cEditSizer = new wxStaticBoxSizer(wxVERTICAL,cPanel,wxT("Edit Options"));
	wxCheckBox *cBoxViewWS = new wxCheckBox(cPanel,MY1ID_PREF_VIEWWS,wxT("View White Space"));
	wxCheckBox *cBoxViewEOL = new wxCheckBox(cPanel,MY1ID_PREF_VIEWEOL,wxT("View End-Of-Line"));
	cEditSizer->Add(cBoxViewWS,0);
	cEditSizer->Add(cBoxViewEOL,0);
	cEditSizer->AddStretchSpacer();
	wxStaticBoxSizer* cDummySizer = new wxStaticBoxSizer(wxVERTICAL,cPanel,wxT("Dummy Options"));
	wxCheckBox *cBoxUnixEOL = new wxCheckBox(cPanel,MY1ID_PREF_UNIXEOL,wxT("Force UNIX EOL"));
	cDummySizer->Add(cBoxUnixEOL,0);
	cDummySizer->AddStretchSpacer();
	wxBoxSizer *cTopSizer = new wxBoxSizer(wxHORIZONTAL);
	cTopSizer->Add(cEditSizer,1,wxEXPAND|wxTOP|wxLEFT);
	//cTopSizer->AddSpacer(5);
	cTopSizer->Add(cDummySizer,1,wxEXPAND|wxTOP|wxRIGHT);
	wxButton *cButtOK = new wxButton(cPanel,MY1ID_PREF_SAVE,wxT("Save"));
	wxButton *cButtKO = new wxButton(cPanel,MY1ID_PREF_CANCEL,wxT("Cancel"));
	wxBoxSizer *cButtSizer = new wxBoxSizer(wxHORIZONTAL);
	cButtSizer->Add(cButtOK,0,wxALIGN_RIGHT,10);
	cButtSizer->Add(cButtKO,0,wxALIGN_RIGHT,10);
	wxBoxSizer *cMainSizer = new wxBoxSizer(wxVERTICAL);
	cMainSizer->Add(cTopSizer,1,wxEXPAND|wxALL);
	cMainSizer->Add(cButtSizer,0,wxALIGN_BOTTOM|wxALIGN_RIGHT,10);
	cPanel->SetSizer(cMainSizer);
	cPanel->Fit();
	cMainSizer->SetSizeHints(cPanel);
	return cPanel;
}

wxPanel* my1OptionDialog::CreateSimsPanel(void)
{
	wxPanel *cPanel = new wxPanel(mPrefBook);
	wxStaticText *cLabS = new wxStaticText(cPanel,wxID_ANY,wxT("Starting Address"));
	wxTextCtrl *cValS = new wxTextCtrl(cPanel,MY1ID_PREF_STARTADDR,
		wxString::Format("%04X",0),
		wxDefaultPosition,wxDefaultSize);
	wxBoxSizer *cBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	cBoxSizer->Add(cLabS,1,wxALIGN_CENTER);
	cBoxSizer->Add(cValS,0);
	wxCheckBox *cBoxSimFreeRun = new wxCheckBox(cPanel,MY1ID_PREF_FREERUN,wxT("Free Running Simulation"));
	wxCheckBox *cBoxSimRunInfo = new wxCheckBox(cPanel,MY1ID_PREF_RUNINFO,wxT("Auto-Print Execution Info"));
	wxStaticBoxSizer* cTopSizer = new wxStaticBoxSizer(wxVERTICAL,cPanel,wxT("Simulation Options"));
	cTopSizer->Add(cBoxSizer,0,wxALIGN_TOP);
	cTopSizer->Add(cBoxSimFreeRun,0,wxALIGN_TOP);
	cTopSizer->Add(cBoxSimRunInfo,0,wxALIGN_TOP);
	cTopSizer->AddStretchSpacer();
	wxButton *cButtOK = new wxButton(cPanel,MY1ID_PREF_SAVE,wxT("Save"));
	wxButton *cButtKO = new wxButton(cPanel,MY1ID_PREF_CANCEL,wxT("Cancel"));
	wxBoxSizer *cButtSizer = new wxBoxSizer(wxHORIZONTAL);
	cButtSizer->Add(cButtOK,0,wxALIGN_RIGHT,10);
	cButtSizer->Add(cButtKO,0,wxALIGN_RIGHT,10);
	wxBoxSizer *cMainSizer = new wxBoxSizer(wxVERTICAL);
	cMainSizer->Add(cTopSizer,1,wxEXPAND|wxALL);
	cMainSizer->Add(cButtSizer,0,wxALIGN_BOTTOM|wxALIGN_RIGHT,10);
	cPanel->SetSizer(cMainSizer);
	cPanel->Fit();
	cMainSizer->SetSizeHints(cPanel);
	return cPanel;
}

void my1OptionDialog::OnOptCheck(wxCommandEvent &event)
{
	wxObject *cObject = event.GetEventObject();
	wxCheckBox *cCheckBox;
	wxTextCtrl *cTextCtrl;
	wxString cCheckText;
	unsigned long cStart;
	switch(event.GetId())
	{
		case MY1ID_PREF_VIEWWS:
			cCheckBox = (wxCheckBox*) cObject;
			mCurrentOptions.mEdit_ViewWS = cCheckBox->GetValue();
			break;
		case MY1ID_PREF_VIEWEOL:
			cCheckBox = (wxCheckBox*) cObject;
			mCurrentOptions.mEdit_ViewEOL = cCheckBox->GetValue();
			break;
		case MY1ID_PREF_UNIXEOL:
			cCheckBox = (wxCheckBox*) cObject;
			mCurrentOptions.mConv_UnixEOL = cCheckBox->GetValue();
			break;
		case MY1ID_PREF_FREERUN:
			cCheckBox = (wxCheckBox*) cObject;
			mCurrentOptions.mSims_FreeRunning = cCheckBox->GetValue();
			break;
		case MY1ID_PREF_RUNINFO:
			cCheckBox = (wxCheckBox*) cObject;
			mCurrentOptions.mSims_ShowRunInfo = cCheckBox->GetValue();
			break;
		case MY1ID_PREF_STARTADDR:
			cTextCtrl = (wxTextCtrl*) cObject;
			cCheckText = cTextCtrl->GetLineText(0);
			if(cCheckText.ToULong(&cStart,16)&&cStart<0xFFFF)
				mCurrentOptions.mSims_StartADDR = cStart;
			else
				cTextCtrl->ChangeValue(wxString::Format("%04X",mCurrentOptions.mSims_StartADDR));
			break;
		case MY1ID_PREF_DUMMY:
			break;
	}
	mCurrentOptions.mChanged = true;
}

void my1OptionDialog::OnOptSave(wxCommandEvent &event)
{
	mParentOptions = mCurrentOptions;
	mParentOptions.mChanged = true;
	this->EndModal(0);
}

void my1OptionDialog::OnOptClose(wxCommandEvent &event)
{
	bool cGoClose = true;
	mCurrentOptions.mChanged = mParentOptions.mChanged;
	//if(mCurrentOptions.mChanged)
	if(mCurrentOptions!=mParentOptions)
	{
		int cGoSave = wxMessageBox(wxT("Save Before Closing?"),
			wxT("Changes Made!"),wxYES_NO|wxCANCEL,this);
		if(cGoSave==wxYES)
		{
			mParentOptions = mCurrentOptions;
			mParentOptions.mChanged = true;
		}
		else if(cGoSave==wxCANCEL)
			cGoClose = false;
	}
	if(cGoClose)
		this->EndModal(0);
}
