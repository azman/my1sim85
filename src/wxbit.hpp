/**
*
* wxbit.hpp
*
* - header for wx-based bit control base class
*
**/

#ifndef __MY1BIT_HPP__
#define __MY1BIT_HPP__

#include <wx/wx.h>
#include "wxform.hpp"

class my1BITCtrl : public wxPanel
{
	wxDECLARE_DYNAMIC_CLASS(my1BITCtrl);
protected:
	bool mDummy, mInput;
	wxString mLabel;
	my1BitSelect mLink;
	my1Form *myForm;
public:
	my1BITCtrl(wxWindow*,wxWindowID,const wxPoint& point = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,bool dummy=false);
	my1BITCtrl();
	~my1BITCtrl();
	bool IsDummy(void);
	bool IsInput(void);
	void SetLabel(const wxString&);
	const wxString& GetLabel(void);
	my1BitSelect* GetLink(void);
	my1BitSelect& Link(void);
	void Link(my1BitSelect&);
	virtual void LinkThis(my1BitIO* aBitIO);
	void LinkCheck(my1BitSelect& aLink);
	void LinkBreak(void);
};

#endif
