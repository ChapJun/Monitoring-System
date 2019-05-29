#pragma once
#include "afxcmn.h"
#include "SettingStitchDlg.h"
#include "util.h"
#include <iostream>
#include <stdio.h>
#include <opencv2\opencv.hpp>
#include <opencv2\xfeatures2d\nonfree.hpp>
#include "afxwin.h"

using namespace std;
using namespace cv;
using namespace xfeatures2d;
using namespace detail;

// CImageStitch 대화 상자입니다.

class CImageStitch : public CDialogEx
{
	DECLARE_DYNAMIC(CImageStitch)

public:
	CImageStitch(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CImageStitch();
	void ImageStitching();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IMAGE_STITCH };
#endif

private:
	int num;
	vector<string> img_name;
	int dType;

	bool try_gpu = false;
	
	float SURF_hessianThreshold;
	float AKAZE_Threshold;
	int BRISK_Threshold;

	double work_megapix = 0.6; 
	float match_conf = 0.3f;
	float conf_thresh = 1.1f;

	string ba_cost_func = "ray";
	string ba_refine_mask = "xxxxx";

	bool do_wave_correct = true;
	WaveCorrectKind wave_correct = WAVE_CORRECT_HORIZ;

	string warp_type = "spherical"; // (plane, cylindrical(*), spherical(*), transverseMercator, mercator(*))

	double seam_megapix = 0.1;
	string seam_find_type = "gc_color";

	double compose_megapix = -1;

	int expos_comp_type = ExposureCompensator::GAIN_BLOCKS;

	int blend_type = Blender::MULTI_BAND;
	float blend_strength = 5;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonAdd();
	

	CListBox m_fileList;
	afx_msg void OnBnDeleteClicked();
	afx_msg void OnBnClickedOk();
};
