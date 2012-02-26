/**
*
* wxled.cpp
*
* - implementation for wx-based LED control
*
**/

#include "wxled.hpp"
#include "my1sim85.hpp"
#include "wxform.hpp"

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
	wxMenu* pMenu = static_cast<wxMenu*>(event.GetEventObject())->GetClientData();
}

void my1LEDCtrl::OnMouseClick(wxMouseEvent &event)
{
	// get event location?
	//wxPoint pos = event.GetPosition();
	if(event.RightDown())
	{
		// port selector?
		wxMenu *cMenuPop = new wxMenu;
		my1Form* pForm = (my1Form*) this->GetGrandParent();
		int cIndexID = MY1ID_DUMMY+MY1ID_DEVC_OFFSET;
		int cCount = 0;
		my1Device *pDevice = (*pForm).Processor().Device(0);
		while(pDevice)
		{
			wxString cText = wxT("Device @") +
				wxString::Format(wxT("%02X"),pDevice->GetStart());
			cMenuPop->Append(cIndexID, cText);
			this->Connect(cIndexID, wxEVT_COMMAND_RADIOBUTTON_SELECTED,
				wxCommandEventHandler(my1BitSelectDialog::OnSelectClick));
			this->Connect(wxEVT_COMMAND_MENU_SELECTED,my1LEDCtrl::OnPopupClick, NULL, this);
			if(cCount==mCurrentCopy.mDevice)
				cRadio->SetValue(true);
			cCount++; cIndexID++; cGoStyle = 0;
			pDevice = (my1Device*) pDevice->Next();
		}
		cMenuSelect.Connect(wxEVT_COMMAND_MENU_SELECTED,my1LEDCtrl::OnPopupClick, NULL, this);
		PopupMenu(&mnu);
		
if (!m_menu)
    {
        m_menu = new wxMenu;
        m_menu->Append(wxID_OPEN, wxT("&Open"));
        m_menu->AppendSeparator();
        m_menu->Append(wxID_EXIT, wxT("E&xit"));
    }

    PopupMenu(m_menu, event.GetPosition());
}
/*
		my1Form* pForm = (my1Form*) this->GetGrandParent();
		if(pForm->SelectBitIO(mLink))
		{
			my1BitIO* pBit = (my1BitIO*) mLink.mPointer;
			pBit->SetLink((void*)this);
			pBit->DoUpdate = my1LEDCtrl::DoUpdate;
			pBit->DoDetect = 0x0; // just in case!
		}
*/
	}
}

void my1LEDCtrl::DoUpdate(void* object)
{
	my1LED *anLED = (my1LED*) object;
	my1LEDCtrl *pLED = (my1LEDCtrl*) anLED->GetLink();
	if(!pLED) return;
	pLED->Light(anLED->GetData());
}
