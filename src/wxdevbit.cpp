/**
*
* wxdevbit.cpp
*
* - implementation for wx-based device bit select dialog
*
**/

#include "wxdevbit.hpp"
#include "wxform.hpp"

#include <iostream>
// CHANGE THIS TO POP-UP MENU!

my1BitSelectDialog::my1BitSelectDialog(wxWindow *parent, const wxString &title,
	my1BitSelect &select) : wxDialog( parent, wxID_ANY, title ),
	mParentCopy(select)
{
	my1Form* pForm = (my1Form*) parent;
	// reset
	mCurrentCopy = mParentCopy;
	// try small fonts
	wxFont cTestFont(8,wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	this->SetFont(cTestFont);
	// create device selection boxes
	wxStaticBoxSizer* cDeviceSizer = new wxStaticBoxSizer(wxVERTICAL,this,wxT("Select Device"));
	int cIndexID = MY1ID_DEVBIT_DUMMY+MY1ID_DEVC_OFFSET;
	int cGoStyle = wxRB_GROUP;
	int cCount = 0;
	my1Device *pDevice = (*pForm).Processor().Device(0);
	while(pDevice)
	{
		wxString cText = wxT("Device @") +
			wxString::Format(wxT("%02X"),pDevice->GetStart());
		wxRadioButton *cRadio = new wxRadioButton(this, cIndexID, cText,
			wxDefaultPosition, wxDefaultSize, cGoStyle);
		cDeviceSizer->Add(cRadio, 0, wxALIGN_TOP);
		this->Connect(cIndexID, wxEVT_COMMAND_RADIOBUTTON_SELECTED,
			wxCommandEventHandler(my1BitSelectDialog::OnSelectClick));
		if(cCount==mCurrentCopy.mDevice)
			cRadio->SetValue(true);
		cCount++; cIndexID++; cGoStyle = 0;
		pDevice = (my1Device*) pDevice->Next();
	}
	cDeviceSizer->AddStretchSpacer();
	// create port selection boxes
	wxStaticBoxSizer* cDevPortSizer = new wxStaticBoxSizer(wxVERTICAL,this,wxT("Select Device Port"));
	cIndexID = MY1ID_DEVBIT_DUMMY+MY1ID_PORT_OFFSET;
	cGoStyle = wxRB_GROUP;
	cCount = 0;
	for(int cLoop=0;cLoop<I8255_SIZE-1;cLoop++)
	{
		wxString cText = wxT("Port ") +
			wxString::Format(wxT("%c"),(char)(cLoop+(int)'A'));
		wxRadioButton *cRadio = new wxRadioButton(this, cIndexID, cText,
			wxDefaultPosition, wxDefaultSize, cGoStyle);
		cDevPortSizer->Add(cRadio, 0, wxALIGN_TOP);
		this->Connect(cIndexID, wxEVT_COMMAND_RADIOBUTTON_SELECTED,
			wxCommandEventHandler(my1BitSelectDialog::OnSelectClick));
		if(cCount==mCurrentCopy.mDevicePort)
			cRadio->SetValue(true);
		cCount++; cIndexID++; cGoStyle = 0;
	}
	cDevPortSizer->AddStretchSpacer();
	// create bit selection boxes
	wxStaticBoxSizer* cDevBitSizer = new wxStaticBoxSizer(wxVERTICAL,this,wxT("Select Bit/Pin"));
	cIndexID = MY1ID_DEVBIT_DUMMY+MY1ID_DBIT_OFFSET;
	cGoStyle = wxRB_GROUP;
	cCount = 0;
	for(int cLoop=0;cLoop<I8255_DATASIZE;cLoop++)
	{
		wxString cText = wxT("Bit ") +
			wxString::Format(wxT("%01X"),cLoop);
		wxRadioButton *cRadio = new wxRadioButton(this, cIndexID, cText,
			wxDefaultPosition, wxDefaultSize, cGoStyle);
		cDevBitSizer->Add(cRadio, 0, wxALIGN_TOP);
		this->Connect(cIndexID, wxEVT_COMMAND_RADIOBUTTON_SELECTED,
			wxCommandEventHandler(my1BitSelectDialog::OnSelectClick));
		if(cCount==mCurrentCopy.mDeviceBit)
			cRadio->SetValue(true);
		cCount++; cIndexID++; cGoStyle = 0;
	}
	cDevBitSizer->AddStretchSpacer();
	// create selection sizer
	wxBoxSizer *cPickSizer = new wxBoxSizer(wxHORIZONTAL);
	cPickSizer->Add(cDeviceSizer, 1, wxEXPAND | wxTOP | wxLEFT, 10);
	cPickSizer->AddSpacer(DEVICE_PICK_SPACER);
	cPickSizer->Add(cDevPortSizer, 1, wxEXPAND | wxTOP | wxRIGHT, 10);
	cPickSizer->AddSpacer(DEVICE_PICK_SPACER);
	cPickSizer->Add(cDevBitSizer, 1, wxEXPAND | wxTOP | wxRIGHT, 10);
	// create button sizer
	wxBoxSizer *cButtSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *buttOK = new wxButton(this, MY1ID_DEVBIT_SELECT, wxT("Select"));
	wxButton *buttKO = new wxButton(this, MY1ID_DEVBIT_CANCEL, wxT("Cancel"));
	cButtSizer->Add(buttOK, 0, wxALL, 10);
	cButtSizer->Add(buttKO, 0, wxALL, 10);
	// create main sizer
	wxBoxSizer *cMainSizer = new wxBoxSizer(wxVERTICAL);
	cMainSizer->Add(cPickSizer, 1, wxEXPAND | wxALL);
	cMainSizer->Add(cButtSizer, 0, wxALIGN_CENTER);
	// assign to dialog
	this->SetSizer(cMainSizer);
	this->Fit();
	this->Centre();
	// events & actions
	this->Connect(MY1ID_DEVBIT_SELECT, wxEVT_COMMAND_BUTTON_CLICKED,
		wxCommandEventHandler(my1BitSelectDialog::OnDeviceSelect));
	this->Connect(MY1ID_DEVBIT_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED,
		wxCommandEventHandler(my1BitSelectDialog::OnDeviceCancel));
}

void my1BitSelectDialog::OnSelectClick(wxCommandEvent &event)
{
	int cCheck = event.GetId() - MY1ID_DEVBIT_DUMMY;
	if(cCheck<MY1ID_PORT_OFFSET)
	{
		// device select
		mCurrentCopy.mDevice = cCheck-MY1ID_DEVC_OFFSET;
	}
	else if(cCheck<MY1ID_DBIT_OFFSET)
	{
		// port select
		mCurrentCopy.mDevicePort = cCheck-MY1ID_PORT_OFFSET;
	}
	else
	{
		// bit select
		mCurrentCopy.mDeviceBit = cCheck-MY1ID_DBIT_OFFSET;
	}
	my1BitSelect &aSelect = mCurrentCopy;
	std::cout << "Test: " << cCheck << " :: "<< aSelect.mDevice << ", "
		<< aSelect.mDevicePort << ", " << aSelect.mDeviceBit << ", "
		<< aSelect.mPointer << "\n";
}

void my1BitSelectDialog::OnDeviceSelect(wxCommandEvent &event)
{
	mParentCopy.mDevice = mCurrentCopy.mDevice;
	mParentCopy.mDevicePort = mCurrentCopy.mDevicePort;
	mParentCopy.mDeviceBit = mCurrentCopy.mDeviceBit;
	mParentCopy.mPointer = mCurrentCopy.mPointer;
	this->EndModal(1);
}

void my1BitSelectDialog::OnDeviceCancel(wxCommandEvent &event)
{
	this->EndModal(0);
}
