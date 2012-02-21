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

void my1LEDCtrl::DoUpdate(void* object)
{
	my1LED *anLED = (my1LED*) object;
	my1LEDCtrl *pLED = (my1LEDCtrl*) anLED->GetLink();
	if(!pLED) return;
	pLED->Light(anLED->GetData());
}
