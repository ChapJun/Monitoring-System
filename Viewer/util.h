#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "SettingStitchDlg.h"
#include <iostream>
#include <opencv2\opencv.hpp>
#include <opencv2\xfeatures2d\nonfree.hpp>
#include "opencv2\core\ocl.hpp"

using namespace std;
using namespace cv;
using namespace xfeatures2d;
using namespace detail;

class Util{
public:
	void automatic_feature_detection_set(Mat img);
	void print_Info_Camera(VideoCapture cr);
	bool cmp_Dmatch(DMatch a, DMatch b);
	void sharpen2D(const cv::Mat &image, cv::Mat &result);
	void erase_Noise(Mat &img);
	float GetSURFthreshold();
	float GetAKAZEthreshold();
	int GetBRISKthreshold();

	ocl::Context getContext();

	void connect_gpu();

private:
	//string detector_type; // Type of features used for images matching. The default is surf.    (surf, orb, akaze, brisk)
	//int features_num;
	float SURF_hessianThreshold = 100.0f;
	float AKAZE_Threshold = 0.001f;
	int BRISK_Threshold = 30;
	
	ocl::Context context;
};

