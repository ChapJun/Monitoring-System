// VideoStitching.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Viewer.h"
#include "VideoStitching.h"
#include "afxdialogex.h"


// CVideoStitching 대화 상자입니다.

IMPLEMENT_DYNAMIC(CVideoStitching, CDialogEx)

CVideoStitching::CVideoStitching(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_VIDEO_STICHING_DIALOG, pParent)
{

}

CVideoStitching::~CVideoStitching()
{
}

void CVideoStitching::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_left_file_name);
	DDX_Control(pDX, IDC_EDIT2, m_right_file_name);
	DDX_Control(pDX, IDC_EDIT3, m_result_file_name);
}


BEGIN_MESSAGE_MAP(CVideoStitching, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CVideoStitching::OnLeftOpenClicked)
	ON_BN_CLICKED(IDC_BUTTON2, &CVideoStitching::OnRightOpenClicked)
	ON_BN_CLICKED(IDC_BUTTON3, &CVideoStitching::OnResultOpenClicked)
	ON_BN_CLICKED(IDOK, &CVideoStitching::OnBnClickedOk)
END_MESSAGE_MAP()


// CVideoStitching 메시지 처리기입니다.


void CVideoStitching::OnLeftOpenClicked()
{
	CFileDialog dlg(TRUE);

	if (dlg.DoModal() == IDOK) {
		if (dlg.GetFileExt() == "mp4" || dlg.GetFileExt() == "mov" || dlg.GetFileExt() == "avi") {
			m_left_file_name.SetWindowTextA(dlg.GetPathName());
			left_file = (const char*)dlg.GetPathName();
		}
		else {
			MessageBox(_T("올바른 파일 형식이 아닙니다."));
			return;
		}
	}
}


void CVideoStitching::OnRightOpenClicked()
{
	CFileDialog dlg(TRUE);
	if(dlg.DoModal() == IDOK) {
		if (dlg.GetFileExt() == "mp4" || dlg.GetFileExt() == "mov" || dlg.GetFileExt() == "avi") {
			m_right_file_name.SetWindowTextA(dlg.GetPathName());
			right_file = (const char*)dlg.GetPathName();
		}
		else {
			MessageBox(_T("올바른 파일 형식이 아닙니다."));
			return;
		}
	}
	
}


void CVideoStitching::OnResultOpenClicked()
{
	CFileDialog dlg(TRUE);
	if (dlg.DoModal() == IDOK) {
		if (dlg.GetFileExt() == "mp4" || dlg.GetFileExt() == "mov" || dlg.GetFileExt() == "avi") {
			m_result_file_name.SetWindowTextA(dlg.GetPathName());
			result_file = (const char*)dlg.GetPathName();
		}
		else {
			MessageBox(_T("올바른 파일 형식이 아닙니다."));
			return;
		}
	}
}

void CVideoStitching::Play(VideoCapture& _capture1, VideoCapture& _capture2, VideoCapture& _capture3) {
	
	while (char(waitKey(1)) != 'e') {

		Mat frame1, frame2, frame3;

		_capture1 >> frame1;
		_capture2 >> frame2;
		_capture3 >> frame3;

		//Check if the video is over
		if (frame1.empty() || frame2.empty() || frame3.empty())
			break;

		namedWindow("Left", WINDOW_NORMAL);
		imshow("Left", frame1);

		namedWindow("Right", WINDOW_NORMAL);
		imshow("Right", frame2);

		namedWindow("Result", WINDOW_NORMAL);
		imshow("Result", frame3);

	}

	destroyAllWindows();
}

void CVideoStitching::OnBnClickedOk()
{
	VideoCapture capture_left;
	capture_left.open(left_file);

	VideoCapture capture_right;
	capture_right.open(right_file);

	VideoCapture capture_result;
	capture_result.open(result_file);

	CV_Assert(capture_left.isOpened());
	CV_Assert(capture_right.isOpened());
	CV_Assert(capture_result.isOpened());

	Play(capture_left, capture_right, capture_result);

	ReleaseCapture();
	CDialogEx::OnOK();
}
