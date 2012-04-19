/**
*
* wxswitch.cpp
*
* - implementation for wx-based switching control
*
**/

#include "wxswitch.hpp"
#include "my1sim85.hpp"

typedef my1BitIO my1SWI;

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

my1BitSelect& my1SWICtrl::Link(void)
{
	return mLink;
}

void my1SWICtrl::Link(my1BitSelect& aLink)
{
	mLink = aLink;
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
	cDC.SetBackground(mParent->GetBackgroundColour());
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
		cDC.DrawRectangle(mSize/4,mSize-SWI_SIZE_KNOB-SWI_SIZE_OFFSET*3/2,mSize/2,SWI_SIZE_KNOB);
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
	my1BitSelect cSelect;
	int cCheck = event.GetId() - (MY1ID_DSEL_OFFSET+MY1ID_DBIT_OFFSET);
	if(cCheck<0) return;
	cSelect.UseIndex(cCheck);
	my1Form* pForm = (my1Form*) this->GetGrandParent();
	if(pForm->GetDeviceBit(cSelect))
	{
		my1BitIO* pBit = (my1BitIO*) cSelect.mPointer;
		if(cSelect.mPointer==mLink.mPointer)
		{
			pBit->Unlink();
			mLink.mPointer = 0x0; // unlink existing!
		}
		else
		{
			if(pBit->GetLink()) // should not happen... will delete
			{
				if(wxMessageBox(wxT("Pin used! Force removal?"),
					wxT("Please confirm"),wxICON_QUESTION|wxYES_NO,this)==wxNO)
					return;
				pBit->Unlink();
			}
			pBit->SetLink((void*)this);
			pBit->DoDetect = my1SWICtrl::DoDetect;
			// unlink previous
			pBit = (my1BitIO*) mLink.mPointer;
			if(pBit) pBit->Unlink();
			// assign new link
			mLink = cSelect;
		}
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
	else if(event.RightDown())
	{
		// not linked to i/o device! special condition for intr switch!
		if(mLink.mDevice<0) return;
		// port selector?
		wxWindow *cTarget = this->GetGrandParent();
		if(!cTarget->IsKindOf(CLASSINFO(my1Form)))
			return;
		my1Form *pForm = (my1Form*) cTarget;
		if(pForm->IsFloatingWindow(mParent))
		{
			wxMessageBox(wxT("Please dock this panel for that!"),
				wxT("Invalid Environment!"),wxOK|wxICON_EXCLAMATION,pForm);
			return;
		}
		wxMenu *cMenuPop = pForm->GetDevicePopupMenu();
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
		this->Connect(wxEVT_COMMAND_MENU_SELECTED,
			(wxObjectEventFunction)&my1SWICtrl::OnPopupClick, NULL, this);
		this->PopupMenu(cMenuPop);
	}
}

void my1SWICtrl::DoDetect(void* object)
{
	my1SWI *aSWI = (my1SWI*) object;
	my1SWICtrl *pSWI = (my1SWICtrl*) aSWI->GetLink();
	if(!pSWI) return;
	aSWI->SetState(pSWI->GetState());
}
