
// ViewerDlg.h : ��� ����
//

#pragma once

#include <opencv2\opencv.hpp>
#include <opencv2\xfeatures2d\nonfree.hpp>
#include "opencv2\core\ocl.hpp"
#include <opencv2\features2d.hpp>
#include <opencv2\xfeatures2d.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\calib3d\calib3d.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\video\tracking.hpp>
#include <opencv2\dnn.hpp>
#include <opencv2\dnn\shape_utils.hpp>
#include "SettingStitchDlg.h"
#include "ImageStitch.h"
#include "ObjectDetection.h"
#include "Video.h"
#include "VideoStitching.h"
#include "VideoPanorama.h"
#include <iostream>
#include <stdio.h>
#include "afxwin.h"


using namespace std;
using namespace cv;

// CViewerDlg ��ȭ ����
class CViewerDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CViewerDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.
	bool isImageOpened;
	string FileName;
	Mat Ori_Img;

	CSettingStitchDlg SetDlg;

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIEWER_DIALOG };
#endif

	enum { IMAGE = 1, VIDEO = 2 };
	int fileType;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileOpenClicked();
	CEdit m_fileName;
	afx_msg void OnStitchingSettingClicked();
	afx_msg void OnImageStitchClicked();
	afx_msg void OnObjectDetectionClicked();
	afx_msg void OnPanoDetectionClicked();
	afx_msg void OnVDstitchingClicked();
	afx_msg void OnVideoPanoramaClicked();

	void DisplayImage(Mat &img);

	enum METHOD {
		VIDEO_PANORAMA=1,
		DNN_DETECT_STITCH
	};
	CStatic thumnail;
};
