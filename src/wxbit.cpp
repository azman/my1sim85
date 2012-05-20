/**
*
* wxbit.cpp
*
* - implementation for wx-based bit control base class
*
**/

#include "wxbit.hpp"

wxIMPLEMENT_DYNAMIC_CLASS(my1BITCtrl, wxWindow);

my1BITCtrl::my1BITCtrl()
	: wxPanel(0x0, wxID_ANY)
{
	//this->Hide();
}

my1BITCtrl::my1BITCtrl(wxWindow *parent,wxWindowID id,
	const wxPoint& point,const wxSize& size, bool dummy) :
	wxPanel(parent,id,point,size)
{
	myForm = (my1Form*) parent->GetParent();
	mDummy = dummy;
	mInput = false;
	if(mDummy) this->Hide();
}

my1BITCtrl::~my1BITCtrl()
{
	// nothing to do?
}

bool my1BITCtrl::IsDummy(void)
{
	return mDummy;
}

void my1BITCtrl::SetLabel(const wxString& aLabel)
{
	mLabel = aLabel;
}

const wxString& my1BITCtrl::GetLabel(void)
{
	return mLabel;
}

my1BitSelect* my1BITCtrl::GetLink(void)
{
	return &mLink;
}

my1BitSelect& my1BITCtrl::Link(void)
{
	return mLink;
}

void my1BITCtrl::Link(my1BitSelect& aLink)
{
	mLink = aLink;
}

void my1BITCtrl::LinkThis(my1BitIO* aBitIO)
{
	aBitIO->SetLink(0x0);
	aBitIO->DoUpdate = 0x0;
	aBitIO->DoDetect = 0x0;
}

void my1BITCtrl::LinkCheck(my1BitSelect& aLink)
{
	if(mLink.mPointer&&mLink.mPointer==aLink.mPointer)
	{
		mLink.mPointer = 0x0;
		return;
	}
	this->LinkThis((my1BitIO*)aLink.mPointer);
	this->Link(aLink);
}

void my1BITCtrl::LinkBreak(void)
{
	my1BitIO* pBit = (my1BitIO*) mLink.mPointer;
	if(pBit) pBit->Unlink();
	mLink.mPointer = 0x0;
}
