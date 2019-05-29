#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "SettingStitchDlg.h"
#include "util.h"
#include <opencv2\dnn.hpp>
#include <opencv2\dnn\shape_utils.hpp>
#include <fstream>
#include <thread>

class ObjectDetection {
public:
	dnn::Net create_Net(string model_Config, string model_Weight, int backend, int target);
	vector<string> read_Class_Name(string path);
	void object_Detection(VideoCapture &capture, float confidence_Threshold, dnn::Net &net, vector<string> &classNamesVec);

private:
	Util util;
};