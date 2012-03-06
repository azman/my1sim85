/**
*
* wxled.cpp
*
* - implementation for wx-based LED control
*
**/

#include "wxled.hpp"
#include "my1sim85.hpp"

typedef my1BitIO my1LED;

my1LEDCtrl::my1LEDCtrl(wxWindow *parent, wxWindowID id)
	: wxPanel(parent, id, wxDefaultPosition, wxSize(LED_SIZE_DEFAULT,LED_SIZE_DEFAULT))
{
	mParent = parent;
	mSize = LED_SIZE_DEFAULT;
	mLighted = false;
	// prepare light ON
	mImageHI = new wxBitmap(mSize,mSize);
	this->DrawLED(mImageHI,*wxGREEN);
	// prepare light OFF
	mImageLO = new wxBitmap(mSize,mSize);
	this->DrawLED(mImageLO,*wxBLACK);
	// everything else
	this->SetSize(mSize,mSize);
	this->Connect(wxEVT_PAINT,wxPaintEventHandler(my1LEDCtrl::OnPaint));
	this->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(my1LEDCtrl::OnMouseClick));
}

my1BitSelect& my1LEDCtrl::Link(void)
{
	return mLink;
}

void my1LEDCtrl::Link(my1BitSelect& aLink)
{
	mLink = aLink;
}

void my1LEDCtrl::Light(bool aFlag)
{
	mLighted = aFlag;
	this->Refresh(); // repaint!
}

void my1LEDCtrl::SetColor(wxColor& aColor, bool aHIGH)
{
	if(aHIGH) this->DrawLED(mImageHI,aColor);
	else this->DrawLED(mImageLO,aColor);
}

void my1LEDCtrl::DrawLED(wxBitmap* aBitmap, const wxColor& aColor)
{
	// recreate LED image
	aBitmap->Create(mSize,mSize);
	// prepare device context
	wxMemoryDC cDC;
	cDC.SelectObject(*aBitmap);
	cDC.SetBackground(mParent->GetBackgroundColour());
	cDC.Clear();
	cDC.SetPen(aColor);
	cDC.SetBrush(aColor);
	// draw LED
	cDC.DrawCircle(mSize/2,mSize/2,(mSize/2)-LED_SIZE_OFFSET);
	// release draw objects
	cDC.SetPen(wxNullPen);
	cDC.SetBrush(wxNullBrush);
	cDC.SelectObject(wxNullBitmap);
}

void my1LEDCtrl::OnPaint(wxPaintEvent& event)
{
	// prepare device context
	wxPaintDC pDC(this);
	wxDC &cDC = pDC;
	PrepareDC(cDC);
	// blit (overlay?) the image (faster!)
	wxMemoryDC tempDC;
	if(mLighted) tempDC.SelectObject(*mImageHI);
	else tempDC.SelectObject(*mImageLO);
	cDC.Blit(0,0,mSize,mSize,&tempDC,0,0);
	tempDC.SelectObject(wxNullBitmap);
}

void my1LEDCtrl::OnPopupClick(wxCommandEvent &event)
{
	my1BitSelect cSelect;
	int cCheck = event.GetId() - (MY1ID_DSEL_OFFSET+MY1ID_DBIT_OFFSET);
	cSelect.mDeviceBit = cCheck;
	cSelect.mDevicePort = cSelect.mDeviceBit/I8255_DATASIZE;
	cSelect.mDevice = cSelect.mDevicePort/(I8255_SIZE-1);
	cSelect.mPointer = 0x0;
	my1Form* pForm = (my1Form*) this->GetGrandParent();
	if(pForm->GetDeviceBit(cSelect))
	{
		my1BitIO* pBit = (my1BitIO*) cSelect.mPointer;
		if(pBit->GetLink())
		{
			if(wxMessageBox(wxT("Pin linked to another device! Force removal?"),
					wxT("Please confirm"),wxICON_QUESTION|wxYES_NO,this)==wxNO)
				return;
			pForm->UnlinkDeviceBit(pBit);
		}
		pBit->SetLink((void*)this);
		pBit->DoUpdate = my1LEDCtrl::DoUpdate;
		// copy link
		mLink.mDevice = cSelect.mDevice;
		mLink.mDevicePort = cSelect.mDevicePort;
		mLink.mDeviceBit = cSelect.mDeviceBit;
		mLink.mPointer = cSelect.mPointer;
	}
}

void my1LEDCtrl::OnMouseClick(wxMouseEvent &event)
{
	// get event location?
	//wxPoint pos = event.GetPosition();
	if(event.RightDown())
	{
		// port selector?
		my1Form *pForm = (my1Form*) this->GetGrandParent();
		wxMenu *cMenuPop = pForm->GetDevicePopupMenu();
		if(!cMenuPop) return;
		if(mLink.mPointer) // if linked!
		{
			int cCheck = MY1ID_DSEL_OFFSET+MY1ID_DBIT_OFFSET;
			int cIndex = mLink.mDevice*I8255_SIZE*I8255_DATASIZE;
			cIndex += mLink.mDevicePort*I8255_DATASIZE;
			cIndex += mLink.mDeviceBit;
			wxMenuItem *cItem = cMenuPop->FindItem(cIndex+cCheck);
			if(cItem) cItem->Check();
		}
		this->Connect(wxEVT_COMMAND_MENU_SELECTED,
			(wxObjectEventFunction)&my1LEDCtrl::OnPopupClick, NULL, this);
		this->PopupMenu(cMenuPop);
	}
}

void my1LEDCtrl::DoUpdate(void* object)
{
	my1LED *anLED = (my1LED*) object;
	my1LEDCtrl *pLED = (my1LEDCtrl*) anLED->GetLink();
	if(!pLED) return;
	pLED->Light(anLED->GetData());
}
