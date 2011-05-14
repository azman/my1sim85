/**
*
* wxpanel.hpp
*
* - header for wx-based main panel
*
**/

#include <wx/wx.h>
//#include <wx/notebook.h>

#ifndef __MY1PANEL_HPP__
#define __MY1PANEL_HPP__

class my1Panel : public wxPanel
{
public:
	my1Panel(wxWindow *parent);
	void OnPaint(wxPaintEvent &event);
	void OnMouseClick(wxMouseEvent &event);
	void OnMouseMove(wxMouseEvent &event);
	void OnMouseLeave(wxMouseEvent &event);
	wxFrame *mParent;
	wxNotebook *mPanel;
	wxTextCtrl *mText;
};

#endif
