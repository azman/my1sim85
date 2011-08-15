/**
*
* wxswitch.cpp
*
* - implementation for wx-based switching control
*
**/

#include "wxswitch.hpp"

my1SWICtrl::my1SWICtrl(wxWindow *parent, wxWindowID id)
	: wxPanel(parent, id, wxDefaultPosition, wxSize(SWI_SIZE_DEFAULT,SWI_SIZE_DEFAULT))
{
	mParent = parent;
	mSize = SWI_SIZE_DEFAULT;
	mSwitched = false;
	// prepare switch ON
	mImageHI = new wxBitmap(mSize,mSize);
	this->DrawSWITCH(mImageHI,true);
	// prepare light OFF
	mImageLO = new wxBitmap(mSize,mSize);
	this->DrawSWITCH(mImageLO,false);
	// everything else
	this->SetSize(mSize,mSize);
	this->Connect(wxEVT_PAINT,wxPaintEventHandler(my1SWICtrl::OnPaint));
	this->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(my1SWICtrl::OnMouseClick));
	this->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(my1SWICtrl::OnMouseClick));
}

bool my1SWICtrl::GetState(void)
{
	return mSwitched;
}

void my1SWICtrl::Switch(bool aFlag)
{
	mSwitched = aFlag;
	this->Refresh(); // repaint!
}

void my1SWICtrl::DrawSWITCH(wxBitmap* aBitmap, bool aFlag)
{
	// recreate SWITCH image
	aBitmap->Create(mSize,mSize);
	// prepare device context
	wxMemoryDC cDC;
	cDC.SelectObject(*aBitmap);
	cDC.SetBackground(mParent->GetBackgroundColour());
	cDC.Clear();
	cDC.SetPen(*wxBLACK);
	// draw SWITCH
	cDC.SetBrush(*wxWHITE);
	cDC.DrawRectangle(mSize/4,SWI_SIZE_OFFSET,mSize/2,mSize-SWI_SIZE_OFFSET*2);
	cDC.SetBrush(*wxBLACK);
	cDC.DrawRectangle((mSize/2)-(SWI_SIZE_SLIDER/2),SWI_SIZE_OFFSET*2,
		SWI_SIZE_SLIDER,mSize-SWI_SIZE_OFFSET*4);
	cDC.SetBrush(*wxRED);
	if(!aFlag)
		cDC.DrawRectangle(mSize/4,SWI_SIZE_OFFSET*3/2,mSize/2,SWI_SIZE_KNOB);
	else
		cDC.DrawRectangle(mSize/4,mSize-SWI_SIZE_KNOB-SWI_SIZE_OFFSET*3/2,mSize/2,SWI_SIZE_KNOB);
	// release draw objects
	cDC.SetPen(wxNullPen);
	cDC.SetBrush(wxNullBrush);
	cDC.SelectObject(wxNullBitmap);
}

void my1SWICtrl::OnPaint(wxPaintEvent& event)
{
	// prepare device context
	wxPaintDC pDC(this);
	wxDC &cDC = pDC;
	PrepareDC(cDC);
	// blit (overlay?) the image (faster!)
	wxMemoryDC tempDC;
	if(mSwitched) tempDC.SelectObject(*mImageHI);
	else tempDC.SelectObject(*mImageLO);
	cDC.Blit(0,0,mSize,mSize,&tempDC,0,0);
	tempDC.SelectObject(wxNullBitmap);
}

void my1SWICtrl::OnMouseClick(wxMouseEvent &event)
{
	// get event location?
	//wxPoint pos = event.GetPosition();
	if(event.LeftDown())
	{
		if(!this->GetState()&&DoUpdate) (*DoUpdate)((void*)this);
		this->Switch();
	}
	else if(event.RightDown())
	{
		if(this->GetState()&&DoUpdate) (*DoUpdate)((void*)this);
		this->Switch(false);
	}
}
