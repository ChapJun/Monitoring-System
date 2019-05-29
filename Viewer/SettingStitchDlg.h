#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <iostream>
#include <string>

using namespace std;

static int feature_num = 300;
static double scale_x = 4;
static double scale_y = 4;
static double start_x = -1;
static double start_y = -1;

static string save_name = "panorama.jpg";

static int w_type = 0;
static int d_type = 0;
static int log_flag = 0;

// CSettingStitchDlg 대화 상자입니다.

class CSettingStitchDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSettingStitchDlg)

public:
	CSettingStitchDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CSettingStitchDlg();
	void OnClickedRadioDetector(UINT msg);
	void OnClickedRadioWarping(UINT msg);

	double GetScaleX();
	double GetScaleY();
	double GetStartX();
	double GetStartY();
	int GetdetectorType();
	int GetwarpingType();
	int GetLogFlag();
	int GetFeatureNum();
	string GetSaveName();

private:
	int dType;
	int wType;

	enum detector {
		SURF,
		ORB,
		AKAZE,
		BRISK
	};

	enum warping {
		AFFINE,
		PERSPECTIVE
	};

	int tmp;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTING_STITCH_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	int m_radio_warpingType;
	CSliderCtrl m_feature_num;
	CEdit m_EditScaleX;
	CEdit m_EditScaleY;
	CEdit m_EditStartScaleX;
	CEdit m_EditStartScaleY;
	CEdit m_EditSaveName;
	afx_msg void OnBnClickedOk();
	CButton m_check_log_flag;
	int m_radio_detector;
	afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	CEdit m_edit_feature;
};
