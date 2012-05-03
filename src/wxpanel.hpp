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
#include "wxform.hpp"

class my1Panel : public wxPanel
{
protected:
	int mCheck;
	wxString mBuffer;
	wxStaticText *mText;
public:
	my1Panel(wxWindow*,wxWindowID id=wxID_ANY,int aCheck=-1,
		const wxString& aText=wxEmptyString,
		int aWidth=-1,int aHeight=-1,long style=wxTAB_TRAVERSAL);
	~my1Panel(){}
	int Check(void);
	const wxString& GetText(void);
	void SetText(const wxString&);
	virtual void OnResize(wxSizeEvent& event);
	virtual void OnMouseClick(wxMouseEvent& event);
};

class my1DEVPanel : public my1Panel
{
protected:
	my1Form *myForm;
	// redefine access
	const wxString& GetText(void);
	void SetText(const wxString&);
public:
	my1DEVPanel(wxWindow*,wxWindowID id=wxID_ANY,int aCheck=-1,
		int aWidth=-1,int aHeight=-1,long style=wxTAB_TRAVERSAL);
	~my1DEVPanel();
};

#endif
