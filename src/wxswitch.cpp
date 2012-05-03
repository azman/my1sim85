/**
*
* wxswitch.cpp
*
* - implementation for wx-based switching control
*
**/

#include "wxswitch.hpp"

#define WX_MEH wxMouseEventHandler
#define WX_SEH wxSizeEventHandler
#define WX_PEH wxPaintEventHandler

typedef my1BitIO my1SWI;

my1SWICtrl::my1SWICtrl(wxWindow *parent, wxWindowID id,
	bool do_draw, int aWidth, int aHeight)
	: my1BITCtrl(parent, id, wxDefaultPosition, wxSize(aWidth,aHeight))
{
	mLabel = wxT("SWITCH");
	mSize = aWidth>aHeight? aWidth : aHeight;
	mSwitched = false;
	// prepare switch ON
	mImageDefHI = new wxBitmap(mSize,mSize);
	this->DrawSWITCH(mImageDefHI,true);
	// prepare switch OFF
	mImageDefLO = new wxBitmap(mSize,mSize);
	this->DrawSWITCH(mImageDefLO,false);
	// option to NOT draw (child classes)
	mImageHI = do_draw ? mImageDefHI : 0x0;
	mImageLO = do_draw ? mImageDefLO : 0x0;
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
	my1BitSelect cSelect;
	int cCheck = event.GetId();
	if(cCheck<MY1ID_8085_OFFSET)
	{
		cCheck -= MY1ID_CBIT_OFFSET;
		if(cCheck<0) return;
		cSelect.UseIndex(cCheck);
	}
	else
	{
		cCheck -= MY1ID_8085_OFFSET;
		cSelect.UseSystem(cCheck,0x0);
	}
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
			wxT("Enter new label"), wxT("Changing Label - ")+mLabel);
		if(cDialog->ShowModal()!=wxID_OK)
			return;
		wxString cTestValue = cDialog->GetValue();
		if(cTestValue.Length()) mLabel = cTestValue;
	}
	else if(event.RightDown())
	{
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
				int cCheck;
				if(mLink.mDevice<0)
					cCheck = mLink.mDeviceBit + MY1ID_8085_OFFSET;
				else
					cCheck = mLink.GetIndex() + MY1ID_CBIT_OFFSET;
				wxMenuItem *cItem = cMenuPop->FindItem(cCheck);
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

my1ENCkPad::my1ENCkPad(wxWindow* parent, wxWindowID id, bool do_dummy,
	int aWidth, int aHeight)
	: my1SWICtrl(parent, id, false, aWidth, aHeight)
{
	// create dummy ctrl
	if(do_dummy) 
	{
		mDummy = true;
		this->Hide();
		return;
	}
	// prepare switch ON
	mImageHI = new wxBitmap(mSize,mSize);
	this->DrawSWITCH(mImageHI,true);
	// prepare switch OFF
	mImageLO = new wxBitmap(mSize,mSize);
	this->DrawSWITCH(mImageLO,false);
	// disconnect click-switching mechanism!
	this->Disconnect(wxEVT_LEFT_DOWN, WX_MEH(my1SWICtrl::OnMouseClick));
}

void my1ENCkPad::DrawSWITCH(wxBitmap* aBitmap, bool aFlag)
{
	// recreate LED image
	aBitmap->Create(mSize,mSize);
	// prepare device context
	wxMemoryDC cDC;
	cDC.SelectObject(*aBitmap);
	cDC.SetBackground(this->GetParent()->GetBackgroundColour());
	cDC.Clear();
	cDC.SetPen(*wxBLACK);
	// draw switch outline
	if(!aFlag) cDC.SetBrush(*wxWHITE);
	else cDC.SetBrush(*wxBLACK);
	cDC.DrawCircle(mSize/2,mSize/2,(mSize/2)-SWI_SIZE_OFFSET);
	// draw switch indicator
	if(aFlag) cDC.SetBrush(*wxWHITE);
	else cDC.SetBrush(*wxBLACK);
	cDC.DrawCircle(mSize/2,mSize/2,(mSize/2)-2*SWI_SIZE_OFFSET);
	// release draw objects
	cDC.SetPen(wxNullPen);
	cDC.SetBrush(wxNullBrush);
	cDC.SelectObject(wxNullBitmap);
}

my1KEYCtrl::my1KEYCtrl(wxWindow* parent, wxWindowID id, int aWidth, int aHeight,
		int aKeyID, const wxString& aLabel)
	: wxPanel(parent, id, wxDefaultPosition, wxSize(aWidth,aHeight),
		wxTAB_TRAVERSAL)
{
	mPushed = false;
	mKeyID = aKeyID;
	mText = new wxStaticText(this,wxID_ANY,aLabel);
	mText->Connect(wxEVT_LEFT_DOWN,WX_MEH(my1KEYCtrl::OnMouseClick),NULL,this);
	mText->Connect(wxEVT_LEFT_UP,WX_MEH(my1KEYCtrl::OnMouseClick),NULL,this);
	this->Connect(wxEVT_PAINT, WX_PEH(my1KEYCtrl::OnPaint));
	this->Connect(wxEVT_SIZE, WX_SEH(my1KEYCtrl::OnResize));
	this->Connect(wxEVT_LEFT_DOWN, WX_MEH(my1KEYCtrl::OnMouseClick));
	this->Connect(wxEVT_LEFT_UP, WX_MEH(my1KEYCtrl::OnMouseClick));
}

my1KEYCtrl::~my1KEYCtrl()
{
	// nothing to do
}

int my1KEYCtrl::KeyID(void)
{
	return mKeyID;
}

wxWindow* my1KEYCtrl::GetNextCtrl(wxWindowList::Node **pNode)
{
	wxWindow *pTarget;
	do
	{
		if(!*pNode) return 0x0;
		pTarget = (wxWindow*) (*pNode)->GetData();
		(*pNode) = (*pNode)->GetNext();
	}
	while(!pTarget->IsKindOf(CLASSINFO(my1BITCtrl)));
	return pTarget;
}

void my1KEYCtrl::OnPaint(wxPaintEvent& event)
{
	int cPX, cPY;
	this->GetClientSize(&cPX,&cPY);
	// prepare device context
	wxPaintDC cDC(this);
	wxColor cColorW = wxColor(0x90,0x90,0x90);
	wxColor cColorB = wxColor(0x50,0x50,0x50);
	cDC.SetBackground(this->GetParent()->GetBackgroundColour());
	cDC.Clear();
	// draw top and left border outline
	if(mPushed) { cDC.SetPen(cColorB); cDC.SetBrush(cColorB); }
	else { cDC.SetPen(cColorW); cDC.SetBrush(cColorW); }
	cDC.DrawRectangle(0,0,cPX-1,2);
	cDC.DrawRectangle(0,0,2,cPY-1);
	// draw bottom and right border outline
	if(mPushed) { cDC.SetPen(cColorW); cDC.SetBrush(cColorW); }
	else { cDC.SetPen(cColorB); cDC.SetBrush(cColorB); }
	cDC.DrawRectangle(0,cPY-2,cPX-1,cPY-1);
	cDC.DrawRectangle(cPX-2,0,cPX-1,cPY-1);
	// release draw objects
	cDC.SetPen(wxNullPen);
	cDC.SetBrush(wxNullBrush);
}

void my1KEYCtrl::OnResize(wxSizeEvent& event)
{
	int cCX, cCY;
	int cPX, cPY;
	mText->GetSize(&cCX,&cCY);
	this->GetClientSize(&cPX,&cPY);
	mText->SetPosition(wxPoint((cPX-cCX)/2,(cPY-cCY)/2));
}

void my1KEYCtrl::OnMouseClick(wxMouseEvent& event)
{
	if(event.LeftDown())
	{
		wxWindowList& cList = this->GetParent()->GetChildren();
		if((int)cList.GetCount()<=0) return;
		wxWindowList::Node *pNode = cList.GetFirst();
		// look for DA bit
		my1ENCkPad *pCtrlDA = 0x0;
		// skip dummy controls
		do
		{
			pCtrlDA = (my1ENCkPad*) this->GetNextCtrl(&pNode);
			if(!pCtrlDA) return;
		}
		while(pCtrlDA->IsDummy());
		// get the four bits!
		unsigned int cMask = 0x08;
		for(int cLoop=0;cLoop<4;cLoop++)
		{
			wxWindow *pTarget = this->GetNextCtrl(&pNode);
			if(!pTarget) return;
			my1ENCkPad *pCtrl = (my1ENCkPad*) pTarget;
			if(mKeyID&cMask) pCtrl->Switch();
			else pCtrl->Switch(false);
			cMask >>= 1;
		}
		// switch after all bits are set
		pCtrlDA->Switch();
		mPushed = true;
		this->Refresh();
		this->Update();
	}
	else if(event.LeftUp())
	{
		wxWindowList& cList = this->GetParent()->GetChildren();
		if((int)cList.GetCount()<=0) return;
		wxWindowList::Node *pNode = cList.GetFirst();
		// look for DA bit
		my1ENCkPad *pCtrlDA = 0x0;
		// skip dummy controls
		do
		{
			pCtrlDA = (my1ENCkPad*) this->GetNextCtrl(&pNode);
			if(!pCtrlDA) return;
		}
		while(pCtrlDA->IsDummy());
		pCtrlDA->Switch(false);
		mPushed = false;
		this->Refresh();
		this->Update();
	}
}
