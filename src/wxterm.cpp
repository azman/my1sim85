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
	mCurrentOptions = mParentOptions;

	wxStaticBoxSizer* sbox_imagesize = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Image Size"));
	rbut_set1 = new wxRadioButton(this, MY1ID_RBCHECK_1, wxT("84 x 48"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	rbut_set2 = new wxRadioButton(this, MY1ID_RBCHECK_2, wxT("128 x 64"), wxDefaultPosition);
	rbut_cust = new wxRadioButton(this, MY1ID_RBCHECK_3, wxT("Custom"), wxDefaultPosition);
	tc_custw = new wxTextCtrl(this, MY1ID_TCCHECK_W, wxT(""), wxDefaultPosition);
	tc_custh = new wxTextCtrl(this, MY1ID_TCCHECK_H, wxT(""), wxDefaultPosition);
	tc_custw->Disable();
	tc_custh->Disable();
	sbox_imagesize->Add(rbut_set1, 0);
	sbox_imagesize->Add(rbut_set2, 0);
	sbox_imagesize->Add(rbut_cust, 0);
	sbox_imagesize->Add(tc_custw, 0);
	sbox_imagesize->AddSpacer(5);
	sbox_imagesize->Add(tc_custh, 0);
	sbox_imagesize->AddStretchSpacer();

	wxStaticBoxSizer* sbox_dispsize = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Display Size"));
	stext_gridsize =  new wxStaticText(this, wxID_ANY, wxT("Grid Size"));
	tc_gridsize = new wxTextCtrl(this, wxID_ANY, wxT(""));
	stext_banksize =  new wxStaticText(this, wxID_ANY, wxT("Bank Size"));
	tc_banksize = new wxTextCtrl(this, wxID_ANY, wxT(""));
	// disable these for now!
	tc_gridsize->Disable();
	tc_banksize->Disable();
	sbox_dispsize->Add(stext_gridsize, 0);
	sbox_dispsize->Add(tc_gridsize, 0);
	sbox_dispsize->Add(stext_banksize, 0);
	sbox_dispsize->Add(tc_banksize, 0);
	sbox_dispsize->AddStretchSpacer();

	wxBoxSizer *hbox_select = new wxBoxSizer(wxHORIZONTAL);
	hbox_select->Add(sbox_imagesize, 1, wxEXPAND | wxTOP | wxLEFT, 10);
	hbox_select->AddSpacer(5);
	hbox_select->Add(sbox_dispsize, 1, wxEXPAND | wxTOP | wxRIGHT, 10);

	wxBoxSizer *hbox_decider = new wxBoxSizer(wxHORIZONTAL);
	buttOK = new wxButton(this, MY1ID_PREF_SAVE, wxT("Save"));
	buttKO = new wxButton(this, MY1ID_PREF_CANCEL, wxT("Cancel"));
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
	this->Connect(MY1ID_RBCHECK_1, wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(my1OptionDialog::OnRBSizeCheck));
	this->Connect(MY1ID_RBCHECK_2, wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(my1OptionDialog::OnRBSizeCheck));
	this->Connect(MY1ID_RBCHECK_3, wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler(my1OptionDialog::OnRBSizeCheck));

	// update imagesize info
	*tc_custw << mCurrentOptions.mWidth;
	*tc_custh << mCurrentOptions.mHeight;
	if (mCurrentOptions.mWidth==84&&mCurrentOptions.mHeight==48)
		rbut_set1->SetValue(true);
	else if(mCurrentOptions.mWidth==128&&mCurrentOptions.mHeight==64)
		rbut_set2->SetValue(true);
	else
	{
		rbut_cust->SetValue(true);
		tc_custw->Enable(true);
		tc_custh->Enable(true);
	}

	// update display info
	*tc_gridsize << mCurrentOptions.mGridSize;
	*tc_banksize << mCurrentOptions.mBankSize;
}

void my1OptionDialog::OnRBSizeCheck(wxCommandEvent &event)
{
	switch(event.GetId())
	{
		case MY1ID_RBCHECK_1:
			tc_custw->Disable();
			tc_custh->Disable();
			mCurrentOptions.mWidth = 84;
			mCurrentOptions.mHeight = 48;
			break;
		case MY1ID_RBCHECK_2:
			tc_custw->Disable();
			tc_custh->Disable();
			mCurrentOptions.mWidth = 128;
			mCurrentOptions.mHeight = 64;
			break;
		case MY1ID_RBCHECK_3:
			tc_custw->Clear();
			tc_custh->Clear();
			*tc_custw << mCurrentOptions.mWidth;
			*tc_custh << mCurrentOptions.mHeight;
			tc_custw->Enable(true);
			tc_custh->Enable(true);
			break;
	}
}

void my1OptionDialog::OnOptSave(wxCommandEvent &event)
{
	long cWidth, cHeight;

	//check custom sizes
	if(rbut_cust->GetValue())
	{
		tc_custw->GetValue().ToLong(&cWidth);
		tc_custh->GetValue().ToLong(&cHeight);
		if(cHeight%8)
		{
			wxMessageBox(wxT("Height MUST BE divisible by 8."),wxT("INVALID VALUE!"),wxOK|wxICON_ERROR,this);
			tc_custh->Clear();
			*tc_custh << mCurrentOptions.mHeight;
			return;
		}
		mCurrentOptions.mWidth = (int) cWidth;
		mCurrentOptions.mHeight = (int) cHeight;
	}
	//save options! only if changed?
	if(mCurrentOptions.mWidth!=mParentOptions.mWidth||
		mCurrentOptions.mHeight!=mParentOptions.mHeight)
	{
		mParentOptions.mChanged = 1;
		mParentOptions.mWidth = mCurrentOptions.mWidth;
		mParentOptions.mHeight = mCurrentOptions.mHeight;
	}
	this->EndModal(0);
}

void my1OptionDialog::OnOptClose(wxCommandEvent &event)
{
	this->EndModal(0);
}
