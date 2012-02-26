/**
*
* wxdevbit.hpp
*
* - implementation for wx-based device bit select dialog
*
**/

#ifndef __MY1DEVBIT_HPP__
#define __MY1DEVBIT_HPP__

#include <wx/wx.h>
#include <wx/notebook.h>

enum {
	MY1ID_DEVBIT_SELECT = wxID_HIGHEST+501,
	MY1ID_DEVBIT_CANCEL,
	MY1ID_DEVBIT_DUMMY
};

#define MY1ID_DEVC_OFFSET 1
#define MY1ID_PORT_OFFSET 51
#define MY1ID_DBIT_OFFSET 101
#define DEVICE_PICK_SPACER 5

struct my1BitSelect
{
	int mDevice;
	int mDevicePort;
	int mDeviceBit;
	void* mPointer;
	my1BitSelect():mDevice(0),mDevicePort(0),mDeviceBit(0),mPointer(0x0){}
};

class my1BitSelectDialog : public wxDialog
{
private:
	wxWindow* mParent;
	my1BitSelect &mParentCopy;
	my1BitSelect mCurrentCopy;
public:
	my1BitSelectDialog(wxWindow *parent, const wxString &title, my1BitSelect &select);
	void OnSelectClick(wxCommandEvent &event);
	void OnDeviceSelect(wxCommandEvent &event);
	void OnDeviceCancel(wxCommandEvent &event);
};

#endif
