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

my1LEDCtrl::my1LEDCtrl(wxWindow *parent, wxWindowID id,
	bool do_draw, int aWidth, int aHeight)
	: wxPanel(parent, id, wxDefaultPosition, wxSize(aWidth,aHeight))
{
	mParent = parent;
	// image size
	mSizeW = aWidth;
	mSizeH = aHeight;
	// optimum circle!
	mSizeX = mSizeW > mSizeH ? mSizeH : mSizeW;
	mSizeX /= 2; // in radius!
	mLighted = false;
	// prepare default light ON
	mImageDefHI = new wxBitmap(mSizeW,mSizeH);
	this->DrawLED(mImageDefHI,*wxGREEN);
	// prepare default light OFF
	mImageDefLO = new wxBitmap(mSizeW,mSizeH);
	this->DrawLED(mImageDefLO,*wxBLACK);
	// option to NOT draw (child classes)
	mImageHI = do_draw ? mImageDefHI : 0x0;
	mImageLO = do_draw ? mImageDefLO : 0x0;
	// everything else
	this->SetSize(mSizeW,mSizeH);
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
	if(aHIGH)
	{
		if(!mImageHI) mImageHI = new wxBitmap(mSizeW,mSizeH);
		this->DrawLED(mImageHI,aColor);
	}
	else
	{
		if(!mImageLO) mImageLO = new wxBitmap(mSizeW,mSizeH);
		this->DrawLED(mImageLO,aColor);
	}
}

void my1LEDCtrl::DrawLED(wxBitmap* aBitmap, const wxColor& aColor)
{
	// recreate LED image
	aBitmap->Create(mSizeW,mSizeH);
	// prepare device context
	wxMemoryDC cDC;
	cDC.SelectObject(*aBitmap);
	cDC.SetBackground(mParent->GetBackgroundColour());
	cDC.Clear();
	cDC.SetPen(aColor);
	cDC.SetBrush(aColor);
	// draw LED
	cDC.DrawCircle(mSizeW/2,mSizeH/2,mSizeX-LED_SIZE_SPACING);
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
	cDC.Blit(0,0,mSizeW,mSizeH,&tempDC,0,0);
	tempDC.SelectObject(wxNullBitmap);
}

void my1LEDCtrl::OnPopupClick(wxCommandEvent &event)
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
			if(pBit) pBit->Unlink();
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
			pBit->DoUpdate = my1LEDCtrl::DoUpdate;
			// unlink previous
			pBit = (my1BitIO*) mLink.mPointer;
			if(pBit) pBit->Unlink();
			// assign new link
			mLink = cSelect;
		}
	}
}

void my1LEDCtrl::OnMouseClick(wxMouseEvent &event)
{
	// get event location?
	//wxPoint pos = event.GetPosition();
	if(event.RightDown())
	{
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
			if(this!=(my1LEDCtrl*)pBit->GetLink())
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

my1LED7Seg::my1LED7Seg(wxWindow* parent, wxWindowID id, bool do_vertical,
	int aWidth, int aHeight)
	: my1LEDCtrl(parent, id, false,
		do_vertical ? aHeight : aWidth , do_vertical ? aWidth : aHeight)
{
	// prepare light ON
	mImageHI = new wxBitmap(mSizeW,mSizeH);
	this->DrawLED(mImageHI,*wxGREEN);
	// prepare light OFF
	mImageLO = new wxBitmap(mSizeW,mSizeH);
	this->DrawLED(mImageLO,*wxBLACK);
}

void my1LED7Seg::DrawLED(wxBitmap* aBitmap, const wxColor& aColor)
{
	//my1LEDCtrl::DrawLED(aBitmap,aColor);
	// recreate LED image
	aBitmap->Create(mSizeW,mSizeH);
	// prepare device context
	wxMemoryDC cDC;
	cDC.SelectObject(*aBitmap);
	cDC.SetBackground(mParent->GetBackgroundColour());
	cDC.Clear();
	cDC.SetPen(aColor);
	cDC.SetBrush(aColor);
	// draw LED
	cDC.DrawRoundedRectangle(LED_SIZE_SPACING,LED_SIZE_SPACING,
		mSizeW-LED_SIZE_SPACING*2,mSizeH-LED_SIZE_SPACING*2,SEG_SIZE_T);
	// release draw objects
	cDC.SetPen(wxNullPen);
	cDC.SetBrush(wxNullBrush);
	cDC.SelectObject(wxNullBitmap);
}
