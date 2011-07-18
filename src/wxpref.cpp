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
	// should i consider wxPropertySheetDialog?!

	// why do this? filter!
	mCurrentOptions = mParentOptions;

	wxStaticBoxSizer* sbox_editOpt = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Edit Options"));
	wxCheckBox *cBox_ViewWS = new wxCheckBox(this,MY1ID_PREF_VIEWWS, wxT("View White Space"));
	wxCheckBox *cBox_ViewEOL = new wxCheckBox(this,MY1ID_PREF_VIEWEOL, wxT("View End-Of-Line"));
	sbox_editOpt->Add(cBox_ViewWS, 0);
	sbox_editOpt->Add(cBox_ViewEOL, 0);
	sbox_editOpt->AddStretchSpacer();

	wxStaticBoxSizer* sbox_dummyOpt = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Dummy Options"));
	sbox_dummyOpt->AddStretchSpacer();

	wxBoxSizer *hbox_select = new wxBoxSizer(wxHORIZONTAL);
	hbox_select->Add(sbox_editOpt, 1, wxEXPAND | wxTOP | wxLEFT, 10);
	hbox_select->AddSpacer(5);
	hbox_select->Add(sbox_dummyOpt, 1, wxEXPAND | wxTOP | wxRIGHT, 10);

	wxBoxSizer *hbox_decider = new wxBoxSizer(wxHORIZONTAL);
	wxButton *buttOK = new wxButton(this, MY1ID_PREF_SAVE, wxT("Save"));
	wxButton *buttKO = new wxButton(this, MY1ID_PREF_CANCEL, wxT("Cancel"));
	hbox_decider->Add(buttOK, 0, wxALL, 10);
	hbox_decider->Add(buttKO, 0, wxALL, 10);

	wxBoxSizer *vbox_main = new wxBoxSizer(wxVERTICAL);
	vbox_main->Add(hbox_select, 1, wxEXPAND | wxALL);
	vbox_main->Add(hbox_decider, 0, wxALIGN_CENTER);

	this->SetSizer(vbox_main);
	this->Fit();
	this->Centre();

	this->Connect(MY1ID_PREF_SAVE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1OptionDialog::OnOptSave));
	this->Connect(MY1ID_PREF_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(my1OptionDialog::OnOptClose));
	this->Connect(MY1ID_PREF_DUMMY, wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(my1OptionDialog::OnOptCheck));
	this->Connect(MY1ID_PREF_VIEWWS, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(my1OptionDialog::OnOptCheck));
	this->Connect(MY1ID_PREF_VIEWEOL, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(my1OptionDialog::OnOptCheck));

	// update with current options
	cBox_ViewWS->SetValue(mCurrentOptions.mEdit_ViewWS);
	cBox_ViewEOL->SetValue(mCurrentOptions.mEdit_ViewEOL);
}

void my1OptionDialog::OnOptCheck(wxCommandEvent &event)
{
	wxObject *cObject = event.GetEventObject();
	wxCheckBox *cCheckBox;
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
		case MY1ID_PREF_DUMMY:
			break;
	}
}

void my1OptionDialog::OnOptSave(wxCommandEvent &event)
{
	mParentOptions = mCurrentOptions;
	mParentOptions.mChanged = true;
	this->EndModal(0);
}

void my1OptionDialog::OnOptClose(wxCommandEvent &event)
{
	this->EndModal(0);
}
