/**
*
* wxpanel.cpp
*
* - implementation for wx-based panel control
*
**/

#include "wxpanel.hpp"

#define WX_MEH wxMouseEventHandler
#define WX_SEH wxSizeEventHandler

my1Panel::my1Panel(wxWindow* parent, wxWindowID id, int aCheck,
	const wxString& aLabel, int aWidth, int aHeight, long style)
	: wxPanel(parent, id, wxDefaultPosition,
	(aWidth<0||aHeight<0)?wxDefaultSize:wxSize(aWidth,aHeight), style)
{
	mCheck = aCheck;
	wxBoxSizer *pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	mText = new wxStaticText(this,wxID_ANY,aLabel);
	pBoxSizer->Add(mText,1,wxALIGN_CENTER|wxEXPAND);
	this->SetSizerAndFit(pBoxSizer);
	// do that thing
	mText->Connect(wxEVT_LEFT_DOWN,WX_MEH(my1Panel::OnMouseClick),NULL,this);
	mText->Connect(wxEVT_LEFT_UP,WX_MEH(my1Panel::OnMouseClick),NULL,this);
	this->Connect(wxEVT_SIZE, WX_SEH(my1Panel::OnResize));
	this->Connect(wxEVT_LEFT_DOWN, WX_MEH(my1Panel::OnMouseClick));
	this->Connect(wxEVT_LEFT_UP, WX_MEH(my1Panel::OnMouseClick));
}

int my1Panel::Check(void)
{
	return mCheck;
}

const wxString& my1Panel::GetText(void)
{
	mBuffer = mText->GetLabelText();
	return mBuffer;
}

void my1Panel::SetText(const wxString& aLabel)
{
	mText->SetLabelText(aLabel);
}

void my1Panel::OnMouseClick(wxMouseEvent &event)
{
	event.Skip();
}

void my1Panel::OnResize(wxSizeEvent& event)
{
	int cWidth, cHeight, cTempX, cTempY;
	this->GetClientSize(&cWidth,&cHeight);
	mText->GetSize(&cTempX,&cTempY);
	mText->SetPosition(wxPoint((cWidth-cTempX)/2,(cHeight-cTempY)/2));
}
