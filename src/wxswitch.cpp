/**
*
* wxswitch.cpp
*
* - implementation for wx-based switching control
*
**/

#include "wxswitch.hpp"
#include "my1sim85.hpp"

#define WX_MEH wxMouseEventHandler

typedef my1BitIO my1SWI;

my1SWICtrl::my1SWICtrl(wxWindow *parent, wxWindowID id, int aWidth, int aHeight)
	: my1BITCtrl(parent, id, wxDefaultPosition, wxSize(aWidth,aHeight))
{
	myForm = (my1Form*) parent->GetParent();
	mLabel = wxT("SWITCH");
	mSize = aWidth>aHeight? aWidth : aHeight;
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
	this->Connect(wxEVT_MIDDLE_DOWN, WX_MEH(my1SWICtrl::OnMouseClick));
	this->Connect(wxEVT_LEFT_DOWN, WX_MEH(my1SWICtrl::OnMouseClick));
	this->Connect(wxEVT_RIGHT_DOWN, WX_MEH(my1SWICtrl::OnMouseClick));
	this->Connect(wxEVT_ENTER_WINDOW, WX_MEH(my1SWICtrl::OnMouseOver));
	this->Connect(wxEVT_LEAVE_WINDOW, WX_MEH(my1SWICtrl::OnMouseOver));
}

my1SWICtrl::~my1SWICtrl()
{
	my1BitIO* pBit = (my1BitIO*) mLink.mPointer;
	if(pBit&&this==(my1SWICtrl*)pBit->GetLink())
		pBit->Unlink();
}

void my1SWICtrl::SetLabel(wxString& aString)
{
	mLabel = aString;
}

void my1SWICtrl::LinkThis(my1BitIO* aBitIO)
{
	aBitIO->SetLink((void*)this);
	aBitIO->DoDetect = &my1SWICtrl::DoDetect;
	aBitIO->DoUpdate = 0x0; // just in case!
}

bool my1SWICtrl::GetState(void)
{
	return mSwitched;
}

bool my1SWICtrl::Toggle(void)
{
	this->Switch(!mSwitched);
	return mSwitched;
}

void my1SWICtrl::Switch(bool aFlag)
{
	bool cUpdate = false;
	if(mSwitched!=aFlag)
		cUpdate = true;
	mSwitched = aFlag;
	if(cUpdate)
	{
		this->Refresh(); // repaint!
		this->Update(); // ...immediately!
	}
}

void my1SWICtrl::DrawSWITCH(wxBitmap* aBitmap, bool aFlag)
{
	// recreate SWITCH image
	aBitmap->Create(mSize,mSize);
	// prepare device context
	wxMemoryDC cDC;
	cDC.SelectObject(*aBitmap);
	cDC.SetBackground(this->GetParent()->GetBackgroundColour());
	cDC.Clear();
	cDC.SetPen(*wxBLACK);
	// draw SWITCH
	cDC.SetBrush(*wxWHITE);
	cDC.DrawRectangle(mSize/4,SWI_SIZE_OFFSET,mSize/2,mSize-SWI_SIZE_OFFSET*2);
	cDC.SetBrush(*wxBLACK);
	cDC.DrawRectangle((mSize/2)-(SWI_SIZE_SLIDER/2),SWI_SIZE_OFFSET*2,
		SWI_SIZE_SLIDER,mSize-SWI_SIZE_OFFSET*4);
	if(aFlag)
	{
		cDC.SetBrush(*wxBLUE);
		cDC.DrawRectangle(mSize/4,mSize-SWI_SIZE_KNOB-SWI_SIZE_OFFSET*3/2,
			mSize/2,SWI_SIZE_KNOB);
	}
	else
	{
		cDC.SetBrush(*wxRED);
		cDC.DrawRectangle(mSize/4,SWI_SIZE_OFFSET*3/2,mSize/2,SWI_SIZE_KNOB);
	}
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

void my1SWICtrl::OnPopupClick(wxCommandEvent &event)
{
	int cCheck = event.GetId() - MY1ID_CBIT_OFFSET;
	if(cCheck<0) return;
	my1BitSelect cSelect(cCheck);
	if(myForm->GetDeviceBit(cSelect))
	{
		// unlink previous
		my1BitIO* pBit = (my1BitIO*) mLink.mPointer;
		if(pBit) pBit->Unlink();
		// assign new link
		this->LinkCheck(cSelect);
	}
}

void my1SWICtrl::OnMouseClick(wxMouseEvent &event)
{
	// get event location?
	//wxPoint pos = event.GetPosition();
	if(event.LeftDown())
	{
		this->Toggle();
	}
	else if(event.MiddleDown())
	{
		wxTextEntryDialog* cDialog = new wxTextEntryDialog(this,
			wxT("Enter new label for Switch"), wxT("Changing Label"));
		if(cDialog->ShowModal()!=wxID_OK)
			return;
		wxString cTestValue = cDialog->GetValue();
		if(cTestValue.Length()) mLabel = cTestValue;
	}
	else if(event.RightDown())
	{
		// not linked to i/o device! special condition for intr switch!
		if(mLink.mDevice<0) return;
		// port selector?
		wxMenu *cMenuPop = myForm->GetDevicePopupMenu();
		if(!cMenuPop) return;
		if(mLink.mPointer) // if linked!
		{
			my1BitIO* pBit = (my1BitIO*) mLink.mPointer;
			if(this!=(my1SWICtrl*)pBit->GetLink())
				mLink.mPointer = 0x0; // invalid link!
			else
			{
				int cCheck = MY1ID_DSEL_OFFSET+MY1ID_DBIT_OFFSET;
				int cIndex = mLink.GetIndex();
				wxMenuItem *cItem = cMenuPop->FindItem(cIndex+cCheck);
				if(cItem) { cItem->Check(); cItem->Enable(); }
			}
		}
		this->Bind(wxEVT_COMMAND_MENU_SELECTED,&my1SWICtrl::OnPopupClick,this);
		this->PopupMenu(cMenuPop);
	}
}

void my1SWICtrl::OnMouseOver(wxMouseEvent &event)
{
	if(event.Entering())
	{
		this->SetToolTip(mLabel);
	}
	else if(event.Leaving())
	{
		this->UnsetToolTip();
	}
}

void my1SWICtrl::DoDetect(void* object)
{
	my1SWI *aSWI = (my1SWI*) object;
	my1SWICtrl *pSWI = (my1SWICtrl*) aSWI->GetLink();
	if(!pSWI) return;
	aSWI->SetState(pSWI->GetState());
}
