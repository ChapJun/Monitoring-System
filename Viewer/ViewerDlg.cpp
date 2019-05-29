
// ViewerDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "Viewer.h"
#include "ViewerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CViewerDlg 대화 상자



CViewerDlg::CViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_VIEWER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FILE_NAME, m_fileName);
	DDX_Control(pDX, IDC_thumnail, thumnail);
}

BEGIN_MESSAGE_MAP(CViewerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_32771, &CViewerDlg::OnFileOpenClicked)
	ON_COMMAND(ID_32774, &CViewerDlg::OnStitchingSettingClicked)
	ON_COMMAND(ID_32775, &CViewerDlg::OnImageStitchClicked)
	ON_COMMAND(ID_32777, &CViewerDlg::OnObjectDetectionClicked)
	ON_COMMAND(ID_32778, &CViewerDlg::OnPanoDetectionClicked)
	ON_COMMAND(ID_32779, &CViewerDlg::OnVDstitchingClicked)
	ON_COMMAND(ID_32776, &CViewerDlg::OnVideoPanoramaClicked)
END_MESSAGE_MAP()


// CViewerDlg 메시지 처리기

BOOL CViewerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CViewerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CViewerDlg::OnFileOpenClicked()
{
	CFileDialog dlg(TRUE);
	isImageOpened = false;

	VideoCapture capture;


	if (dlg.DoModal() == IDOK) {
		if (dlg.GetFileExt() == "mp4" || dlg.GetFileExt() == "MP4" || dlg.GetFileExt() == "MOV" || dlg.GetFileExt() == "mov" || dlg.GetFileExt() == "avi" || dlg.GetFileExt() == "mpg") {
			fileType = VIDEO;
			Ori_Img = imread((const char*)dlg.GetPathName());
			FileName = (const char*)dlg.GetPathName();
			isImageOpened = true;

			m_fileName.SetWindowTextA(dlg.GetPathName());

			capture.open(FileName);
			Mat frame;
			capture >> frame;

			rotate(frame, frame, ROTATE_180);
			flip(frame, frame, 1);
			
			DisplayImage(frame);
		}
		else if (dlg.GetFileExt() == "jpg" || dlg.GetFileExt() == "jpeg" || dlg.GetFileExt() == "JPG" || dlg.GetFileExt() == "JPEG"|| dlg.GetFileExt() == "png" || dlg.GetFileExt() == "PNG") {
			fileType = IMAGE;
			m_fileName.SetWindowTextA(dlg.GetFileName());
			Ori_Img = imread((const char*)dlg.GetPathName());
			FileName = (const char*)dlg.GetPathName();
			isImageOpened = true;

			rotate(Ori_Img, Ori_Img, ROTATE_180);
			flip(Ori_Img, Ori_Img, 1);

			DisplayImage(Ori_Img);
			
		}
		else {
			MessageBox(_T("올바른 파일 형식이 아닙니다."));
			isImageOpened = false;

			CDC *p = thumnail.GetWindowDC();
			OnEraseBkgnd(p);
			RedrawWindow();

			return;
		}
	}

}

void CViewerDlg::DisplayImage(Mat &img)
{
	CDC* pDC;
	CImage* mfcImg = nullptr;

	pDC = thumnail.GetDC();

	Mat tmp;
	resize(img, tmp, Size(img.cols * 2, img.rows * 2));

	BITMAPINFO bitmapinfo;
	bitmapinfo.bmiHeader.biYPelsPerMeter = 0;
	bitmapinfo.bmiHeader.biBitCount = 24;
	bitmapinfo.bmiHeader.biWidth = tmp.cols;
	bitmapinfo.bmiHeader.biHeight = tmp.rows;
	bitmapinfo.bmiHeader.biPlanes = 1;
	bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfo.bmiHeader.biCompression = BI_RGB;
	bitmapinfo.bmiHeader.biClrImportant = 0;
	bitmapinfo.bmiHeader.biClrUsed = 0;
	bitmapinfo.bmiHeader.biSizeImage = 0;
	bitmapinfo.bmiHeader.biXPelsPerMeter = 0;

	if (img.channels() == 3) {
		mfcImg = new CImage();
		mfcImg->Create(tmp.cols, tmp.rows, 24);
	}
	else if (img.channels() == 4) {

		bitmapinfo.bmiHeader.biBitCount = 32;
		mfcImg = new CImage();
		mfcImg->Create(tmp.cols, tmp.rows, 32);
	}
	
	CClientDC dc(GetDlgItem(IDC_thumnail));

	CRect rect;
	GetDlgItem(IDC_thumnail)->GetClientRect(&rect);

	SetStretchBltMode(dc.GetSafeHdc(), COLORONCOLOR);
	StretchDIBits(dc.GetSafeHdc(), 0, 0, rect.Width(), rect.Height(),
		0, 0, tmp.cols, tmp.rows, tmp.data, &bitmapinfo, DIB_RGB_COLORS, SRCCOPY);

	if (mfcImg) {
		mfcImg->ReleaseDC();
		delete mfcImg;
		mfcImg = nullptr;
	}

	tmp.release();
	ReleaseDC(pDC);
}


void CViewerDlg::OnStitchingSettingClicked()
{
	if (SetDlg.DoModal() == IDOK) {

	}
}


void CViewerDlg::OnImageStitchClicked()
{
	CImageStitch ImgDlg;

	if (ImgDlg.DoModal() == IDOK) {

	}
}


void CViewerDlg::OnObjectDetectionClicked()
{
	ObjectDetection OD;
	Video vd;
	Util util;

	//string model_Config = "yolov3.cfg";
	//string model_Weight = "yolov3.weights";

	string model_Config = "yolov3-tiny.cfg";
	string model_Weight = "yolov3-tiny.weights";

	dnn::Net net = OD.create_Net(model_Config, model_Weight, dnn::DNN_BACKEND_DEFAULT, dnn::DNN_TARGET_OPENCL);

	vector<string> classNamesVec = OD.read_Class_Name("coco.names");

	Mat img;
	VideoCapture capture;

	/*capture.open(FileName);
	CV_Assert(capture.isOpened());
	vd.print_Info_Camera(capture);

	capture >> img;

	util.erase_Noise(img); */

	vd.Read_Video(capture, img, FileName);
	OD.object_Detection(capture, 0.24f, net, classNamesVec);
	
}

void CViewerDlg::OnPanoDetectionClicked()
{
	if (FileName == "") {
		MessageBox(_T("현재 파일이 존재하지 않습니다."));
		return;
	}

	VideoPanorama vdpanoDlg;
	Video vd;

	Mat img;
	VideoCapture cap;

	vd.Read_Video(cap, img, FileName);
	vdpanoDlg.video_panorama(cap, img, DNN_DETECT_STITCH);

}


void CViewerDlg::OnVDstitchingClicked()
{
	CVideoStitching vdStitch;

	if (vdStitch.DoModal() == IDOK) {

	} 
}


void CViewerDlg::OnVideoPanoramaClicked()
{
	if (FileName == "") {
		MessageBox(_T("현재 파일이 존재하지 않습니다."));
		return;
	}

	VideoPanorama vdpanoDlg;
	Video vd;

	Mat img;
	VideoCapture cap;

	vd.Read_Video(cap, img, FileName);
	vdpanoDlg.video_panorama(cap, img, VIDEO_PANORAMA);
	
}
