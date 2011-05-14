/**
*
* wxpanel.cpp
*
* - implementation for wx-based main panel
*
**/

#include "wxpanel.hpp"

my1Panel::my1Panel(wxWindow *parent)
	: wxPanel(parent)
{
	// setup main editor panel
	mPanel = new wxNotebook;

	// gui events
	this->Connect(wxEVT_PAINT, wxPaintEventHandler(my1Panel::OnPaint));
	this->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(my1Panel::OnMouseClick));
	this->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(my1Panel::OnMouseClick));
	this->Connect(wxEVT_MOTION, wxMouseEventHandler(my1Panel::OnMouseMove));
	this->Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(my1Panel::OnMouseLeave));
}

void my1Panel::OnPaint(wxPaintEvent& event)
{
	// prepare device context
	wxPaintDC pdc(this);
	wxDC &dc = pdc;
	PrepareDC(dc);
	dc.SetBackgroundMode(wxSOLID);  // wxTRANSPARENT @ wxSOLID;
	dc.SetBackground(wxBrush(wxColour(255,255,255),wxSOLID));
	dc.SetTextForeground(wxColour(0,0,0));
	dc.SetTextBackground(wxColour(255,255,255));
	//dc.Clear();
	dc.SetPen(wxPen(wxColour(0,0,0), 1, wxSOLID));

	// blit (overlay?) the grid image (faster!)
	wxMemoryDC temp_dc;
	temp_dc.SelectObject(*mGridImage);
	dc.Blit(0,0,mGridImage->GetWidth(),mGridImage->GetHeight(),&temp_dc,0,0);
	temp_dc.SelectObject(wxNullBitmap);

	// draw from image object?
	unsigned char *pPixel = mCurrentImage->GetData();
	for(int cRow=0;cRow<mImageHeight;cRow++)
	{
		for(int cCol=0;cCol<mImageWidth;cCol++)
		{
			// check bit and set color
			long index = ((cRow * mImageWidth + cCol) * 3);
			if(pPixel[index]==PIXEL_GRAY_BLACK)
			{
				int iRow = cRow * mImageGridSize;
				int iCol = cCol * mImageGridSize;
				for(int cLoop=0;cLoop<mImageGridSize-1;cLoop++)
				{
					for(int cLoop1=0;cLoop1<mImageGridSize-1;cLoop1++)
					{
						dc.DrawPoint(cLoop1+iCol,cLoop+iRow);
					}
				}
			}
		}
	}
	dc.SetPen(wxNullPen);
	//dc.SelectObject(wxNullBitmap);
}

void my1Panel::OnMouseClick(wxMouseEvent &event)
{
	// get event location
	wxPoint pos = event.GetPosition();
	int cCoordX = pos.x/mImageGridSize;
	int cCoordY = pos.y/mImageGridSize;
	unsigned char *pPixel = mCurrentImage->GetData();
	long index = ((cCoordY * mImageWidth + cCoordX) * 3);

	// set pixel status & update image object as well?
	wxColour pcolor;
	if(event.LeftDown())
	{
		pcolor = wxColour(0,0,0);
		pPixel[index] = pPixel[index+1] = pPixel[index+2] = PIXEL_GRAY_BLACK;
	}
	else if(event.RightDown())
	{
		pcolor = wxColour(255,255,255);
		pPixel[index] = pPixel[index+1] = pPixel[index+2] = PIXEL_GRAY_WHITE;
	}

	// update display
	wxPaintDC pdc(this);
	wxDC &dc = pdc;
	PrepareDC(dc);
	dc.SetPen(wxPen(pcolor, 1, wxSOLID));
	cCoordX *= mImageGridSize;
	cCoordY *= mImageGridSize;
	for(int cLoop=0;cLoop<mImageGridSize-1;cLoop++)
	{
		for(int cLoop1=0;cLoop1<mImageGridSize-1;cLoop1++)
		{
			dc.DrawPoint(cLoop1+cCoordX,cLoop+cCoordY);
		}
	}
	mImageChanged = true;
	//dc.SetPen(wxNullPen);
	//dc.SelectObject(wxNullBitmap);
	this->Refresh();
}

void my1Panel::OnMouseMove(wxMouseEvent &event)
{
	// get event location
	wxPoint pos = event.GetPosition();
	int posX = (int)(pos.x/mImageGridSize);
	int posY = (int)(pos.y/mImageGridSize);

	// convert to string
	wxString str;
	str.Printf(wxT("Pixel position: %d,%d"), posX, posY);

	// show on parent's status bar
	mParent->SetStatusText(str, 1);
}

void my1Panel::OnMouseLeave(wxMouseEvent &event)
{
	// show on parent's status bar
	mParent->SetStatusText(wxT(""), 1);
}
