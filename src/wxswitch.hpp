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
#include "wxbit.hpp"

#define SWI_SIZE_DEFAULT 21
#define SWI_SIZE_OFFSET 2
#define SWI_SIZE_SLIDER 4
#define SWI_SIZE_KNOB 6
#define KEY_SIZE_PANEL 19

class my1SWICtrl : public my1BITCtrl
{
protected:
	my1Form *myForm;
	int mSize;
	bool mSwitched;
	wxBitmap *mImageDefHI, *mImageDefLO;
	wxBitmap *mImageHI, *mImageLO;
	virtual void DrawSWITCH(wxBitmap*,bool);
public:
	my1SWICtrl(wxWindow*,wxWindowID,bool do_draw=true,
		int aWidth=SWI_SIZE_DEFAULT,int aHeight=SWI_SIZE_DEFAULT);
	~my1SWICtrl();
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

class my1ENCkPad : public my1SWICtrl
{
public:
	my1ENCkPad(wxWindow*, wxWindowID, bool do_dummy=false,
		int aWidth=KEY_SIZE_PANEL,int aHeight=KEY_SIZE_PANEL);
	virtual void DrawSWITCH(wxBitmap*,bool);
};

class my1KEYCtrl : public wxPanel
{
protected:
	bool mPushed;
	int mKeyID;
	wxStaticText *mText;
public:
	my1KEYCtrl(wxWindow*,wxWindowID,int,int,int,const wxString&);
	~my1KEYCtrl();
	int KeyID(void);
	wxWindow* GetNextCtrl(wxWindowList::Node**);
	void OnPaint(wxPaintEvent& event);
	void OnResize(wxSizeEvent& event);
	void OnMouseClick(wxMouseEvent& event);
};

#endif
