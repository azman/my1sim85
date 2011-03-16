/**
*
* wxform.cpp
*
* - implementation for main wx-based form
*
**/

#include "wxform.hpp"
#include "wxpref.hpp"
#include <wx/textfile.h>
#include <wx/image.h>

#if USE_XPM_BITMAPS
	#define MY1BITMAP_EXIT   "bitmaps/exit.xpm"
	#define MY1BITMAP_NEW    "bitmaps/new.xpm"
	#define MY1BITMAP_OPEN   "bitmaps/open.xpm"
	#define MY1BITMAP_SAVE   "bitmaps/save.xpm"
	#define MY1BITMAP_BINARY "bitmaps/binary.xpm"
	#define MY1BITMAP_OPTION "bitmaps/option.xpm"
#else
	#define MY1BITMAP_EXIT   "exit"
	#define MY1BITMAP_NEW    "new"
	#define MY1BITMAP_OPEN   "open"
	#define MY1BITMAP_SAVE   "save"
	#define MY1BITMAP_BINARY "binary"
	#define MY1BITMAP_OPTION "option"
#endif

my1Form::my1Form(const wxString &title)
	: wxFrame( NULL, wxID_ANY, title, wxDefaultPosition, wxSize(320, 240), wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX) )
{
	wxInitAllImageHandlers();
	SetIcon(wxIcon(wxT(MY1APP_ICON)));
	wxIcon mIconExit(wxT(MY1BITMAP_EXIT));
	wxIcon mIconClear(wxT(MY1BITMAP_NEW));
	wxIcon mIconLoad(wxT(MY1BITMAP_OPEN));
	wxIcon mIconSave(wxT(MY1BITMAP_SAVE));
	wxIcon mIconGenerate(wxT(MY1BITMAP_BINARY));
	wxIcon mIconOptions(wxT(MY1BITMAP_OPTION));

	wxToolBar *mainTool = CreateToolBar();
	// our icon is 16x16, windows defaults to 24x24
	mainTool->SetToolBitmapSize(wxSize(16,16));
	mainTool->AddTool(MY1ID_EXIT, wxT("Exit"), mIconExit);
	mainTool->AddSeparator();
	mainTool->AddTool(MY1ID_CLEAR, wxT("Clear"), mIconClear);
	mainTool->AddTool(MY1ID_LOAD, wxT("Load"), mIconLoad);
	mainTool->AddTool(MY1ID_SAVE, wxT("Save"), mIconSave);
	mainTool->AddSeparator();
	mainTool->AddTool(MY1ID_GENERATE, wxT("Generate"), mIconGenerate);
	mainTool->AddTool(MY1ID_OPTIONS, wxT("Options"), mIconOptions);
	mainTool->Realize();

	CreateStatusBar(2);
	SetStatusText(_T("Welcome to my1GoLCD!"));

	mEditor = new my1Editor(this);
	SetClientSize(mCanvas->mImageWidth*mCanvas->mImageGridSize,
		mCanvas->mImageHeight*mCanvas->mImageGridSize);
	// SendSizeEvent(); // just in case??

	// actions!
	this->Connect(MY1ID_EXIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnQuit));
	this->Connect(MY1ID_CLEAR, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnClear));
	this->Connect(MY1ID_LOAD, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnLoad));
	this->Connect(MY1ID_SAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnSave));
	this->Connect(MY1ID_GENERATE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnGenerate));
	this->Connect(MY1ID_OPTIONS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(my1Form::OnCheckOptions));

	Centre();
}

void my1Form::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

void my1Form::OnClear(wxCommandEvent& WXUNUSED(event))
{
	mCanvas->ResetImage();
	mCanvas->Refresh();
	mCanvas->mImageChanged = false;
}

void my1Form::OnLoad(wxCommandEvent& WXUNUSED(event))
{
	if(mCanvas->mImageChanged)
	{
		if(wxMessageBox(wxT("Current content has not been saved! Proceed?"), wxT("Please confirm"), wxICON_QUESTION | wxYES_NO, this) == wxNO)
			return;
	}

	wxString cFileName = wxFileSelector(_T("Select image file"));
	if(!cFileName)
		return;

	wxImage cTestImage;
	if( !cTestImage.LoadFile(cFileName) )
	{
		wxLogError(_T("Couldn't load image from '%s'."), cFileName.c_str());
		return;
	}

	while(cTestImage.GetWidth()!=mCanvas->mImageWidth||
			cTestImage.GetHeight()!=mCanvas->mImageHeight)
	{
		if(wxMessageBox(wxT("Try to rescale image to fit?"), wxT("Incompatible Image Size"), wxICON_QUESTION | wxYES_NO, this) == wxNO)
		{
			wxLogError(_T("Incompatible image size '%d x %d' (%dx%d)."), cTestImage.GetWidth(), cTestImage.GetHeight(), mCanvas->mImageWidth, mCanvas->mImageHeight);
			return;
		}
		cTestImage.Rescale(mCanvas->mImageWidth, mCanvas->mImageHeight);
	}

	cTestImage.ConvertToMono(255,255,255);
	mCanvas->mCurrentImage->Paste(cTestImage,0,0);
	mCanvas->Refresh();
}

void my1Form::OnSave(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog saveFileDialog(this, wxT("Save image file"), wxT(""), wxT(""), wxT("BMP (*.bmp)|*.bmp"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

	if(saveFileDialog.ShowModal() == wxID_CANCEL)
		return;

	mCanvas->mCurrentImage->SaveFile(saveFileDialog.GetPath(),wxBITMAP_TYPE_BMP);
	mCanvas->mImageChanged = false;
}

void my1Form::OnGenerate(wxCommandEvent& WXUNUSED(event))
{
	wxArrayString cList;
	wxString cBuffer, cConvert;
	bool cUseASM = false;
	wxFileDialog genFileDialog(this, wxT("Generated Filename"), wxT(""), wxT(""),
		wxT("ASM File (*.asm)|*.asm|C/C++ File (*.c;*.cpp)|*.c;*.cpp"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

	if(genFileDialog.ShowModal()==wxID_OK)
	{
		if(genFileDialog.GetFilterIndex()==0)
		{
			cBuffer = wxT("\tdfb ");
			cUseASM = true;
		}
		else
		{
			cBuffer = wxT("\t0x");
			cList.Add(wxT("unsigned char cData[]={"));
		}
		unsigned char *pPixel = mCanvas->mCurrentImage->GetData();
		int cCount = 0;
		for(int cLoop=0;cLoop<mCanvas->mImageBankSize;cLoop++)
		{
			if(cUseASM)
			{
				cConvert.Printf(wxT("; Start Bank %d"),cLoop);
			}
			else
			{
				cConvert.Printf(wxT("/* Start Bank %d"),cLoop);
				cConvert << wxT(" */");
			}
			cList.Add(cConvert);
			for(int cLoop1=0;cLoop1<mCanvas->mImageWidth;cLoop1++)
			{
				int cY = cLoop*8;
				unsigned char cByte = 0x00, cMask = 0x01;
				for(int cLoop2=0;cLoop2<8;cLoop2++)
				{
					if(pPixel[((cY+cLoop2) * mCanvas->mImageWidth + cLoop1) * 3]==PIXEL_GRAY_BLACK)
					{
						cByte |= cMask;
					}
					cMask <<=1;
				}
				if(cByte>0x9f&&cUseASM)
					cConvert.Printf(wxT("%03X"),cByte);
				else
					cConvert.Printf(wxT("%02X"),cByte);
				cBuffer << cConvert;
				cCount++;
				if(cUseASM)
				{
					if(cCount==mCanvas->mImageBankSize)
					{
						cBuffer << wxT("h");
						cList.Add(cBuffer);
						cBuffer = wxT("\tdfb ");
						cCount = 0;
					}
					else
					{
						cBuffer << wxT("h, ");
					}
				}
				else
				{
					if(cCount==mCanvas->mImageBankSize)
					{
						cBuffer << wxT(",");
						cList.Add(cBuffer);
						cBuffer = wxT("\t0x");
						cCount = 0;
					}
					else
					{
						cBuffer << wxT(", 0x");
					}
				}
			}
		}
		if(!cUseASM)
		{
			cBuffer = cList[cList.GetCount()-1];
			cList.RemoveAt(cList.GetCount()-1);
			cBuffer.RemoveLast(); // remove trailing comma
			cList.Add(cBuffer);
			cList.Add(wxT("};"));
		}
		wxTextFile genFile(genFileDialog.GetPath());
		if(genFile.Exists())
		{
			genFile.Open(genFileDialog.GetPath());
			genFile.Clear();
		}
		else
		{
			genFile.Create(genFileDialog.GetPath());
		}
		for(int cIndex=0;cIndex<(int)(cList.GetCount());cIndex++)
		{
			genFile.AddLine(cList[cIndex]);
		}
		genFile.Write();
		genFile.Close();
		cBuffer.Printf(wxT("LCD Data Table Created in %s."),genFileDialog.GetPath().c_str());
		wxMessageBox(cBuffer,wxT("Just FYI"),wxOK|wxICON_INFORMATION,this);
	}
}

void my1Form::OnCheckOptions(wxCommandEvent &event)
{
	my1Options currOptions = { 0, mCanvas->mImageWidth, mCanvas->mImageHeight,
	   	mCanvas->mImageGridSize, mCanvas->mImageBankSize };
	my1OptionDialog *prefDialog = new my1OptionDialog(this, wxT("Options"), currOptions);
	prefDialog->ShowModal();
	//prefDialog->Destroy(); // double deletion if done here!
	delete prefDialog;

	if(currOptions.mChanged)
	{
		mCanvas->mImageWidth = currOptions.mWidth;
		mCanvas->mImageHeight = currOptions.mHeight;
		mCanvas->mImageGridSize = currOptions.mGridSize;
		mCanvas->mImageBankSize = currOptions.mBankSize;
		//wxMessageBox(wxT("Changing Display!"),wxT("Just FYI"),wxOK|wxICON_INFORMATION,this);

		//Freeze();
		SetClientSize(mCanvas->mImageWidth*mCanvas->mImageGridSize,
			mCanvas->mImageHeight*mCanvas->mImageGridSize);
		mCanvas->RedrawGrid();
		// try to rescale instead of clearing!
		mCanvas->RescaleImage();
		//mCanvas->ResetImage();
		SendSizeEvent();
		//Thaw();
		Refresh();
	}
}
