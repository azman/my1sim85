/**
*
* wxled.hpp
*
* - header for wx-based LED control
*
**/

#ifndef __MY1LED_HPP__
#define __MY1LED_HPP__

#include <wx/wx.h>
#include "wxdevbit.hpp"

#define LED_SIZE_DEFAULT 21
#define LED_SIZE_OFFSET 2

class my1LEDCtrl : public wxPanel
{
protected :
	wxWindow *mParent;
	int mSize;
	bool mLighted;
	wxBitmap *mImageHI, *mImageLO;
	my1BitSelect mLink;
	void DrawLED(wxBitmap*,const wxColor&);
public :
	my1LEDCtrl(wxWindow*,wxWindowID);
	~my1LEDCtrl(){}
	my1BitSelect& Link(void);
	void Link(my1BitSelect&);
	void Light(bool aFlag=true);
	void SetColor(wxColor&,bool aHIGH=true);
	void OnPaint(wxPaintEvent&);
	void OnMouseClick(wxMouseEvent &event);
	// target for function pointer need to be static!
	static void DoUpdate(void*);
};

#endif
