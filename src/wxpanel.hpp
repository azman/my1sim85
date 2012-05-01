/**
*
* wxpanel.hpp
*
* - header for wx-based panel control
*
**/

#ifndef __MY1PANEL_HPP__
#define __MY1PANEL_HPP__

#include <wx/wx.h>

class my1Panel : public wxPanel
{
protected:
	int mCheck;
	wxString mBuffer;
	wxStaticText *mText;
public:
	my1Panel(wxWindow*,wxWindowID,int,const wxString&,
		int aWidth=-1,int aHeight=-1,long style=wxTAB_TRAVERSAL);
	~my1Panel(){}
	int Check(void);
	const wxString& GetText(void);
	void SetText(const wxString&);
	virtual void OnResize(wxSizeEvent& event);
	virtual void OnMouseClick(wxMouseEvent& event);
};

#endif
