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
#include "wx/colordlg.h"
#include "wxbit.hpp"

#define LED_SIZE_DEFAULT 21
#define LED_SIZE_SPACING 2
#define SEG_SIZE_W 30
#define SEG_SIZE_H 9
#define SEG_SIZE_T 4

class my1LEDCtrl : public my1BITCtrl
{
protected:
	int mSizeX, mSizeW, mSizeH;
	bool mLighted;
	wxColor mColorON, mColorOFF;
	wxColourData mColorData;
	wxBitmap *mImageDefHI, *mImageDefLO;
	wxBitmap *mImageHI, *mImageLO;
	virtual void DrawLED(wxBitmap*,const wxColor&);
public:
	my1LEDCtrl(wxWindow*,wxWindowID,bool do_draw=true,
		int aWidth=LED_SIZE_DEFAULT,int aHeight=LED_SIZE_DEFAULT);
	~my1LEDCtrl();
	virtual void LinkThis(my1BitIO*);
	void Light(bool aFlag=true);
	void SetColor(wxColor&,bool aHIGH=true);
	void OnPaint(wxPaintEvent&);
	void OnPopupClick(wxCommandEvent &event);
	void OnMouseClick(wxMouseEvent &event);
	void OnMouseOver(wxMouseEvent &event);
	// target for function pointer need to be static!
	static void DoUpdate(void*);
};

class my1LED7Seg : public my1LEDCtrl
{
public:
	my1LED7Seg(wxWindow*, wxWindowID,bool do_vertical=false,
		int aWidth=SEG_SIZE_W,int aHeight=SEG_SIZE_H);
	virtual void DrawLED(wxBitmap*,const wxColor&);
};

#endif
