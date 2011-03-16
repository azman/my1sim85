/**
*
* wxeditor.hpp
*
* - header for wx-based text editor panel
*
**/

#include <wx/wx.h>
//#include <wx/notebook.h>

#ifndef __MY1EDITOR_HPP__
#define __MY1EDITOR_HPP__

class my1Editor : public wxPanel
{
public:
	my1Editor(wxWindow *parent);
	void OnPaint(wxPaintEvent &event);
	void OnMouseClick(wxMouseEvent &event);
	void OnMouseMove(wxMouseEvent &event);
	void OnMouseLeave(wxMouseEvent &event);
	wxFrame *mParent;
	wxNotebook *mPanel;
	wxTextCtrl *mText;
};

#endif
