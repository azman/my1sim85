/**
*
* wxswitch.hpp
*
* - header for wx-based switching control
*
**/

#ifndef __MY1SWITCH_HPP__
#define __MY1SWITCH_HPP__

#include <wx/wx.h>

#define SWI_SIZE_DEFAULT 21
#define SWI_SIZE_OFFSET 2
#define SWI_SIZE_SLIDER 4
#define SWI_SIZE_KNOB 6

class my1SWICtrl : public wxPanel
{
protected :
	wxWindow *mParent;
	int mSize;
	bool mSwitched;
	wxBitmap *mImageHI, *mImageLO;
	void DrawSWITCH(wxBitmap*,bool);
public :
	my1SWICtrl(wxWindow*,wxWindowID);
	~my1SWICtrl(){}
	void (*DoUpdate)(void*);
	bool GetState(void);
	void Switch(bool aFlag=true);
	void OnPaint(wxPaintEvent&);
	void OnMouseClick(wxMouseEvent &event);
} ;

#endif
