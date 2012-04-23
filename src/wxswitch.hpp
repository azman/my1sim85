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
#include "wxform.hpp"

#define SWI_SIZE_DEFAULT 21
#define SWI_SIZE_OFFSET 2
#define SWI_SIZE_SLIDER 4
#define SWI_SIZE_KNOB 6

class my1SWICtrl : public wxPanel, public my1BITCtrl
{
protected :
	wxWindow *mParent;
	wxString mLabel;
	int mSize;
	bool mSwitched;
	wxBitmap *mImageHI, *mImageLO;
	void DrawSWITCH(wxBitmap*,bool);
public :
	my1SWICtrl(wxWindow*,wxWindowID);
	~my1SWICtrl(){}
	void SetLabel(wxString&);
	virtual void LinkThis(my1BitIO*);
	bool GetState(void);
	bool Toggle(void);
	void Switch(bool aFlag=true);
	void OnPaint(wxPaintEvent&);
	void OnPopupClick(wxCommandEvent &event);
	void OnMouseClick(wxMouseEvent &event);
	void OnMouseOver(wxMouseEvent &event);
	// target for function pointer need to be static!
	static void DoDetect(void*);
};

#endif
