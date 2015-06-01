#include "LicensePlateReader.h"
#include <wx\dir.h>
#include <wx\filename.h>
#include <wx\file.h>

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"


#ifdef __BORLANDC__
#pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon (under Windows and OS/2 it is in resources and even
// though we could still include the XPM here it would be unused)
#ifndef wxHAS_IMAGES_IN_RESOURCES
#include "../sample.xpm"
#endif

// ----------------------------------------------------------------------------
// Button Sizes
// ----------------------------------------------------------------------------

#define SIZE_BUTTON_XSMALL wxSize(75,30)
#define SIZE_BUTTON_SMALL wxSize(100,30)
#define SIZE_BUTTON_MEDIUM wxSize(150,30)
#define SIZE_BUTTON_LARGE wxSize(200,30)
#define SIZE_BUTTON_XLARGE wxSize(250,30)
#define SIZE_BUTTON_XXLARGE wxSize(300,30)


// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
	// override base class virtuals
	// ----------------------------

	// this one is called on application startup and is a good place for the app
	// initialization (doing it here and not in the ctor allows to have an error
	// return: if OnInit() returns false, the application terminates)
	virtual bool OnInit();

};

using namespace cv;

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:
	// ctor(s)
	MyFrame(const wxString& title);

	// event handlers (these functions should _not_ be virtual)
	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnPaint(wxCommandEvent& WXUNUSED(event));
	void OnBtnSelectDirClick(wxCommandEvent& event);
	void OnPlateBoxSelect(wxCommandEvent& WXUNUSED(event));
	void OnCheckBoxSelect(wxCommandEvent& WXUNUSED(event));
	void OnBtnClearClick(wxCommandEvent& WXUNUSED(event));
	void checkAndSetOptions();

	////////////////////////
	///// FORM ELEMENTS ////
	////////////////////////
	// Buttons
	wxButton* btnOpenDir;
	wxButton* btnClearLists;
	// ListBoxes
	wxListBox *lbPictures;
	wxListBox *lbPlates;
	// Pictures
	wxStaticBitmap *btmMarked;
	wxStaticBitmap *btmLP;
	// Panels
	wxPanel *pnlTop;
	wxPanel *pnlBottom;

	wxCheckBox *cbShowBoundingRects;
	wxCheckBox *cbShowMarkedPicture;
	wxCheckBox *cbUseEqualizeHistogram;
	wxCheckBox *cbShowChSeg;
	wxCheckBox *cbShowCanny;
	wxCheckBox *cbShowPlateContours;


	// OPENCV FUNCS
	LicensePlateReader* lpr;
private:
	unsigned int options;
	// any class wishing to process wxWidgets events must use this macro
	DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
	// menu items
	Minimal_Quit = wxID_EXIT,
	Dialog_SelectDir = wxID_APPLY,
	// it is important for the id corresponding to the "About" command to have
	// this standard value as otherwise it won't be handled properly under Mac
	// (where it is special and put into the "Apple" menu)
	Minimal_About = wxID_ABOUT,
	PlateBox_Select = wxID_SELECTALL,
	Checkbox_Select = wxID_ANY,
	BtnClear_Click = wxID_CLEAR
};

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(Minimal_Quit, MyFrame::OnQuit)
EVT_MENU(Minimal_About, MyFrame::OnAbout)
EVT_MENU(Dialog_SelectDir, MyFrame::OnBtnSelectDirClick)
EVT_BUTTON(BtnClear_Click, MyFrame::OnBtnClearClick)
EVT_BUTTON(Dialog_SelectDir, MyFrame::OnBtnSelectDirClick)
EVT_LISTBOX(PlateBox_Select, MyFrame::OnPlateBoxSelect)
EVT_CHECKBOX(Checkbox_Select,MyFrame::OnCheckBoxSelect)

END_EVENT_TABLE()

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here

bool MyApp::OnInit()
{
	// call the base class initialization method, currently it only parses a
	// few common command-line options but it could be do more in the future
	if (!wxApp::OnInit())
		return false;

	// create the main application window
	MyFrame *frame = new MyFrame("Smart License Plate Recognition System - SLPRS - v0.1a");

	// set initialsize
	frame->SetInitialSize(wxSize(800, 600));

	// and show it (the frames, unlike simple controls, are not shown when
	// created initially)
	frame->Show(true);

	// success: wxApp::OnRun() will be called which will enter the main message
	// loop and the application will run. If we returned false here, the
	// application would exit immediately.
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////
///////// FRAME CLASS - THIS CLASS IS VERY IMPORTANT! ALMOST EVERYTHING IS HERE //////
/////////////////////////////////////////////////////////////////////////////////////

// frame constructor
MyFrame::MyFrame(const wxString& title)
: wxFrame(NULL, wxID_ANY, title)
{
	// set the frame icon
	SetIcon(wxICON(sample));

#if wxUSE_MENUS
	
	/////////////////////////////////////////////////
	///////// MENU ELEMENTS SHOULD BE SET HERE //////
	/////////////////////////////////////////////////

	wxMenu *fileMenu = new wxMenu;
	wxMenu *helpMenu = new wxMenu;
	wxMenu *plateMenu = new wxMenu;

	helpMenu->Append(Minimal_About, "&Hakkýnda\tF1", "Hakkýnda Dialogunu Göster");
	plateMenu->Append(Dialog_SelectDir, "&Klasör Seç", "Plaka Resimlerini Ýçeren Klasörü Seçer");
	fileMenu->Append(Minimal_Quit, "Çý&kýþ\tAlt-X", "Programdan Çýk");

	// now append the freshly created menu to the menu bar...
	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "&Dosya");
	menuBar->Append(plateMenu, "&Plaka Ýþlemleri");
	menuBar->Append(helpMenu, "&Yardým");
	
	options = 0;

	// ... and attach this menu bar to the frame
	SetMenuBar(menuBar);
#endif // wxUSE_MENUS

	/////////////////////////////////////////////////
	///////// FORM ELEMENTS SHOULD BE SET HERE //////
	/////////////////////////////////////////////////

	btnOpenDir = new wxButton(this, wxID_APPLY, wxT("Klasör Seç"), wxPoint(10, 265), SIZE_BUTTON_SMALL, 0);
	btnClearLists = new wxButton(this, wxID_CLEAR, wxT("Temizle"), wxPoint(120, 265), SIZE_BUTTON_SMALL, 0);

	/* READ PIC */
	//Mat srcBGR = imread("C:/Users/MANGAKA/Desktop/plakalar/arac01.jpg");
	//srcBGR.create(Size(320, 240), CV_8UC3);
	Mat srcBGR = Mat::zeros(320, 240, CV_8UC3);
	Mat srcRGB = srcBGR.clone();
	convertBGR2RGB(srcBGR, srcRGB);
	wxImage* resim1 = new wxImage(srcRGB.cols, srcRGB.rows, srcRGB.data, true);
	wxImage resim = resim1->Scale(320, 240, wxIMAGE_QUALITY_HIGH);
	
	// PANELS
	pnlTop = new wxPanel(this, wxID_ANY, wxPoint(10, 10), wxSize(770, 250));
	pnlBottom = new wxPanel(this, wxID_ANY, wxPoint(10, 300), wxSize(770, 215));
	// PANELS



	btmMarked = new wxStaticBitmap(pnlTop, wxID_ANY, wxBitmap(resim), wxPoint(5, 5), wxSize(320, 240));
	/* LISTBOX */
	lbPictures = new wxListBox(pnlTop, wxID_SELECTALL, wxPoint(330, 5), wxSize(250, 240));
	lbPlates = new wxListBox(pnlTop, wxID_ANY, wxPoint(585, 50), wxSize(170, 195));

	
	/* Checboxes for Options*/
	wxStaticText* lblOptions = new wxStaticText(pnlBottom, wxID_ANY, wxT("SEÇENEKLER"), wxPoint(10, 10), SIZE_BUTTON_XLARGE, 0);
	cbShowCanny = new wxCheckBox(pnlBottom, Checkbox_Select, wxT("Canny Göster"), wxPoint(10, 30), SIZE_BUTTON_XLARGE, 0);
	cbShowBoundingRects = new wxCheckBox(pnlBottom, Checkbox_Select, wxT("Bounding Rect Göster"), wxPoint(10, 60), SIZE_BUTTON_XLARGE, 0);
	cbShowPlateContours = new wxCheckBox(pnlBottom, Checkbox_Select, wxT("Karakter Segmentasyonu Göster (Ýlk)"), wxPoint(10, 90), SIZE_BUTTON_XLARGE, 0);
	cbShowChSeg = new wxCheckBox(pnlBottom, Checkbox_Select, wxT("Karakter Segmentasyonu Göster (Son)"), wxPoint(10, 120), SIZE_BUTTON_XLARGE, 0);
	cbUseEqualizeHistogram = new wxCheckBox(pnlBottom, Checkbox_Select, wxT("Histogram Eþitle"), wxPoint(10, 150), SIZE_BUTTON_XLARGE, 0);
	cbShowMarkedPicture = new wxCheckBox(pnlBottom, Checkbox_Select, wxT("Ýþaretlenmiþ Resmi Göster"), wxPoint(10, 180), SIZE_BUTTON_XLARGE, 0);
	
	
	lpr = new LicensePlateReader();

	checkAndSetOptions();

	delete resim1, &resim;

	/* ILKER */

#if wxUSE_STATUSBAR
	// create a status bar just for fun (by default with 1 pane only)
	CreateStatusBar(2);
	SetStatusText("Welcome to SLPRS!");
#endif // wxUSE_STATUSBAR
}





///////////////////////////////////////////////////////////////////
///////// EVENT METHODS WILL BE INSIDE OF FRAME CLASS /////////////
///////////////////////////////////////////////////////////////////

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	// true is to force the frame to close
	Close(true);
}

void MyFrame::OnPlateBoxSelect(wxCommandEvent& WXUNUSED(event))
{
	if (lbPictures->GetSelection() != wxNOT_FOUND)
	{
		wxString fullpath(lbPictures->GetString(lbPictures->GetSelection()));
		Mat markedRGB, lpRGB;
		checkAndSetOptions();
		lpr->setOptions(options);
		wxString licensePlate(lpr->readLicensePlates(fullpath, markedRGB, lpRGB));
		
		if (strcmp(licensePlate, ""))
		{
			wxImage* imgMarked_Raw = new wxImage(markedRGB.cols, markedRGB.rows, markedRGB.data, true);
			wxImage* imgLP_Raw = new wxImage(lpRGB.cols, lpRGB.rows, lpRGB.data, true);
			wxImage imgMarked_Scaled = imgMarked_Raw->Scale(320, 240, wxIMAGE_QUALITY_HIGH);
			wxImage imgLP_Scaled = imgLP_Raw->Scale(DIM_LP_TR_WIDTH*3, DIM_LP_TR_HEIGHT*3, wxIMAGE_QUALITY_HIGH);
			if (btmMarked!=NULL)
			delete btmMarked;
			btmMarked = new wxStaticBitmap(pnlTop, wxID_ANY, wxBitmap(imgMarked_Scaled), wxPoint(5, 5), wxSize(320, 240));
			btmLP = new wxStaticBitmap(pnlTop, wxID_ANY, wxBitmap(imgLP_Scaled), wxPoint(585, 10), wxSize(DIM_LP_TR_WIDTH * 3, DIM_LP_TR_HEIGHT * 3));
			btmMarked->Refresh();
			btmLP->Refresh();
			if (strlen(licensePlate)>6)
			lbPlates->AppendString(licensePlate);
			delete imgMarked_Raw, imgMarked_Scaled, imgLP_Raw, imgLP_Scaled;

		}
	}
}

void MyFrame::OnPaint(wxCommandEvent& WXUNUSED(event))
{
	
}

void MyFrame::OnBtnSelectDirClick(wxCommandEvent& WXUNUSED(event))
{
	wxDirDialog ofdialog = new wxDirDialog(this, wxString("Lütfen resimlerin bululduðu dizini seçiniz"),
		wxEmptyString, wxDD_CHANGE_DIR);
	if (ofdialog.ShowModal() == wxID_OK)
	{
		wxString path = ofdialog.GetPath();
		wxArrayString filearray;
		wxDir dir(path);
		wxString filename;
		wxString filespec = wxT("*.jpg");
		int flags = wxDIR_FILES | wxDIR_DIRS;
		bool fileExists; 
		if(fileExists = dir.GetFirst(&filename, filespec, flags))
			lbPictures->Clear();
		while (fileExists)
		{
			filearray.Add(wxFileName(path, filename).GetFullPath());
			fileExists = dir.GetNext(&filename);
		}
		lbPictures->Append(filearray);
	}
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox(wxString::Format
		(
		"Welcome to %s!\n"
		"\n"
		"This is the minimal wxWidgets sample\n"
		"running under %s.",
		wxVERSION_STRING,
		wxGetOsDescription()
		),
		"About wxWidgets minimal sample",
		wxOK | wxICON_INFORMATION,
		this);
}

void MyFrame::OnCheckBoxSelect(wxCommandEvent& WXUNUSED(event))
{
	checkAndSetOptions();
}

void MyFrame::checkAndSetOptions()
{
	options = 0;
	if (cbShowCanny->IsChecked())
		options |= SHOW_CANNY_RESULT;
	else
		options &= ~SHOW_CANNY_RESULT;

	if (cbShowBoundingRects->IsChecked())
		options |= SHOW_BOUNDING_RECTS;
	else
		options &= ~SHOW_BOUNDING_RECTS;

	if (cbShowChSeg->IsChecked())
		options |= SHOW_CHARACTER_SEG;
	else
		options &= ~SHOW_CHARACTER_SEG;

	if (cbUseEqualizeHistogram->IsChecked())
		options |= USE_EQUALIZE_HISTOGRAM;
	else
		options &= ~USE_EQUALIZE_HISTOGRAM;

	if (cbShowMarkedPicture->IsChecked())
		options |= SHOW_MARKED_PICTURE;
	else
		options &= ~SHOW_MARKED_PICTURE;

	if (cbShowPlateContours->IsChecked())
		options |= SHOW_PLATE_CONTOURS;
	else
		options &= ~SHOW_PLATE_CONTOURS;


}

void MyFrame::OnBtnClearClick(wxCommandEvent& WXUNUSED(event))
{
	lbPictures->Clear();
	lbPlates->Clear();
}