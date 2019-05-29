#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <opencv2\opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;
// CVideoStitching 대화 상자입니다.

class CVideoStitching : public CDialogEx
{
	DECLARE_DYNAMIC(CVideoStitching)

public:
	CVideoStitching(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CVideoStitching();
	void Play(VideoCapture& _capture1, VideoCapture& _capture2, VideoCapture& _capture3);
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIDEO_STICHING_DIALOG };
#endif
private:
	string left_file;
	string right_file;
	string result_file;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLeftOpenClicked();
	afx_msg void OnRightOpenClicked();
	CEdit m_left_file_name;
	CEdit m_right_file_name;
	afx_msg void OnResultOpenClicked();
	CEdit m_result_file_name;
	afx_msg void OnBnClickedOk();
};
