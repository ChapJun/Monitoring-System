// SettingStitchDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Viewer.h"
#include "SettingStitchDlg.h"
#include "afxdialogex.h"


// CSettingStitchDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CSettingStitchDlg, CDialogEx)

CSettingStitchDlg::CSettingStitchDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SETTING_STITCH_DIALOG, pParent)
	, m_radio_warpingType(w_type)
	, m_radio_detector(d_type)
{
	dType = 0;
	wType = 0;
}

CSettingStitchDlg::~CSettingStitchDlg()
{
}

void CSettingStitchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_AFFINE, m_radio_warpingType);
	DDX_Control(pDX, IDC_SLIDER1, m_feature_num);
	DDX_Control(pDX, IDC_EDIT_SCALE_X, m_EditScaleX);
	DDX_Control(pDX, IDC_EDIT_SCALE_Y, m_EditScaleY);
	DDX_Control(pDX, IDC_EDIT_START_X, m_EditStartScaleX);
	DDX_Control(pDX, IDC_EDIT_START_Y, m_EditStartScaleY);
	DDX_Control(pDX, IDC_EDIT_SAVE_NAME, m_EditSaveName);
	DDX_Control(pDX, IDC_CHECK_LOG_FLAG, m_check_log_flag);
	DDX_Radio(pDX, IDC_SURF, m_radio_detector);
	DDX_Control(pDX, IDC_EDIT_FEATURE, m_edit_feature);
}


BEGIN_MESSAGE_MAP(CSettingStitchDlg, CDialogEx)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_AFFINE, IDC_RADIO_PERSPECTIVE, OnClickedRadioWarping)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_SURF, IDC_BRISK, OnClickedRadioDetector)
	ON_BN_CLICKED(IDOK, &CSettingStitchDlg::OnBnClickedOk)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CSettingStitchDlg::OnNMCustomdrawSlider1)
END_MESSAGE_MAP()

void CSettingStitchDlg::OnClickedRadioDetector(UINT msg) {
	UpdateData(true);
	switch (m_radio_detector) {
	case detector::SURF: // SURF
		dType = 0;
		break;
	case detector::ORB: // ORB
		dType = 1;
		break;
	case detector::AKAZE : // AKAZE
		dType = 2;
		break;
	case detector::BRISK : // BRISK
		dType = 3;
		break;
	}
} 

void CSettingStitchDlg::OnClickedRadioWarping(UINT msg) {
	UpdateData(true);
	switch (m_radio_warpingType) {
	case warping::AFFINE : // AFFINE
		wType = 0;
		break;
	case warping::PERSPECTIVE: // PERSPECTIVE
		wType = 1;
		break;
	}
}

BOOL CSettingStitchDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	m_feature_num.SetRange(100, 3000); // 특징점 개수 범위
	m_feature_num.SetTicFreq(100); // 눈금 간격

	m_feature_num.SetLineSize(100);
	m_EditSaveName.SetWindowTextA(save_name.c_str());
	//CString strname = "panorama.jpg";

	CString cTemp1, cTemp2, cTemp3, cTemp4;

	cTemp1 = to_string((int)scale_x).c_str();
	cTemp2 = to_string((int)scale_y).c_str();
	cTemp3 = to_string((int)start_x).c_str();
	cTemp4 = to_string((int)start_y).c_str();
	m_EditScaleX.SetWindowTextA(cTemp1);
	m_EditScaleY.SetWindowTextA(cTemp2);
	m_EditStartScaleX.SetWindowTextA(cTemp3);
	m_EditStartScaleY.SetWindowTextA(cTemp4);

	m_feature_num.SetPos(feature_num);

	if (d_type == SURF) {
		CButton *sButton = (CButton*)GetDlgItem(IDC_SURF);
		CButton *oButton = (CButton*)GetDlgItem(IDC_ORB);
		CButton *aButton = (CButton*)GetDlgItem(IDC_AKAZE);
		CButton *bButton = (CButton*)GetDlgItem(IDC_BRISK);

		sButton->SetCheck(true);
		oButton->SetCheck(false);
		aButton->SetCheck(false);
		bButton->SetCheck(false);

	}
	else if (d_type == ORB) {
		CButton *sButton = (CButton*)GetDlgItem(IDC_SURF);
		CButton *oButton = (CButton*)GetDlgItem(IDC_ORB);
		CButton *aButton = (CButton*)GetDlgItem(IDC_AKAZE);
		CButton *bButton = (CButton*)GetDlgItem(IDC_BRISK);

		sButton->SetCheck(false);
		oButton->SetCheck(true);
		aButton->SetCheck(false);
		bButton->SetCheck(false);
	}
	else if (d_type == AKAZE) {
		CButton *sButton = (CButton*)GetDlgItem(IDC_SURF);
		CButton *oButton = (CButton*)GetDlgItem(IDC_ORB);
		CButton *aButton = (CButton*)GetDlgItem(IDC_AKAZE);
		CButton *bButton = (CButton*)GetDlgItem(IDC_BRISK);

		sButton->SetCheck(false);
		oButton->SetCheck(false);
		aButton->SetCheck(true);
		bButton->SetCheck(false);
	}
	else if (d_type == BRISK) {
		CButton *sButton = (CButton*)GetDlgItem(IDC_SURF);
		CButton *oButton = (CButton*)GetDlgItem(IDC_ORB);
		CButton *aButton = (CButton*)GetDlgItem(IDC_AKAZE);
		CButton *bButton = (CButton*)GetDlgItem(IDC_BRISK);

		sButton->SetCheck(false);
		oButton->SetCheck(false);
		aButton->SetCheck(false);
		bButton->SetCheck(true);

	}

	if (w_type == AFFINE) {
		CButton *aButton = (CButton*)GetDlgItem(IDC_RADIO_AFFINE);
		CButton *pButton = (CButton*)GetDlgItem(IDC_RADIO_PERSPECTIVE);


		aButton->SetCheck(true);
		pButton->SetCheck(false);
	}
	else if (w_type == PERSPECTIVE) {
		CButton *aButton = (CButton*)GetDlgItem(IDC_RADIO_AFFINE);
		CButton *pButton = (CButton*)GetDlgItem(IDC_RADIO_PERSPECTIVE);

		aButton->SetCheck(false);
		pButton->SetCheck(true);

	}
	
	return true;
}

double CSettingStitchDlg::GetScaleX() {
	return scale_x;
}

double CSettingStitchDlg::GetScaleY() {
	return scale_y;
}

double CSettingStitchDlg::GetStartX() {
	return start_x;
}

double CSettingStitchDlg::GetStartY() {
	return start_y;
}

int CSettingStitchDlg::GetdetectorType() {
	return d_type;
}

int CSettingStitchDlg::GetwarpingType() {
	return w_type;
}

int CSettingStitchDlg::GetLogFlag() {
	return log_flag;
}

int CSettingStitchDlg::GetFeatureNum() {
	return feature_num;
}

string CSettingStitchDlg::GetSaveName() {
	return save_name;
}

void CSettingStitchDlg::OnBnClickedOk()
{ 
	feature_num = m_feature_num.GetPos();

	CString cTemp1, cTemp2, cTemp3, cTemp4;

	m_EditScaleX.GetWindowTextA(cTemp1);
	m_EditScaleY.GetWindowTextA(cTemp2);
	m_EditStartScaleX.GetWindowTextA(cTemp3);
	m_EditStartScaleY.GetWindowTextA(cTemp4);
	scale_x = atof(cTemp1);
	scale_y = atof(cTemp2);
	start_x = atof(cTemp3);
	start_y = atof(cTemp4);

	CString tmp;
	m_EditSaveName.GetWindowTextA(tmp);
	save_name = tmp;

	//save_name = m_EditSaveName.GetWindowTextA();

	w_type = wType;
	d_type = dType;
	log_flag = m_check_log_flag.GetCheck(); // 1이면 체크

	CDialogEx::OnOK();
}


void CSettingStitchDlg::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	int nPos;
	CString strPos;

	// get current position of slider
	nPos = m_feature_num.GetPos();
	// convert position int to string
	strPos.Format(_T("%d"), nPos);
	// set edit-box position
	m_edit_feature.SetWindowTextA(strPos);
	*pResult = 0;
}
