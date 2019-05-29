#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "SettingStitchDlg.h"
#include "util.h"
#include <iostream>
#include <opencv2\opencv.hpp>
#include <opencv2\xfeatures2d\nonfree.hpp>
#include <opencv2\opencv.hpp>
#include <opencv2\xfeatures2d\nonfree.hpp>

using namespace std;
using namespace cv;

class Video {
public:
	void Read_Video(VideoCapture &cap, Mat &img, string &file_name);
	void print_Info_Camera(VideoCapture cr);
	int device_num = -1;

private:
	Util util;
};