#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "SettingStitchDlg.h"
#include "util.h"
#include "ObjectDetection.h"
#include "CL\cl.h"

#include <opencv2\features2d.hpp>
#include <opencv2\xfeatures2d.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\calib3d\calib3d.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\video\tracking.hpp>

class VideoPanorama {
public:
	void video_panorama(VideoCapture &cap, Mat img, int type);

	void set_Start_loc(double &height, double &width, Mat &img);
	
	void min_max_distance(vector <DMatch> &matches, double &min_dist, double &max_dist);
	void distance_good_matches(vector<DMatch> &matches, vector<DMatch> &good_matches);
	void distance_good_matches(vector<DMatch> &matches, vector<DMatch> &good_matches, double min_dist);
	void norm_good_matches(vector<DMatch> &matches, vector<DMatch> &good_matches1, vector<KeyPoint> &last, vector<KeyPoint> &cur, int img_rows);

	Mat make_correlation_norm(Mat &H, Mat &H_old, double &now);
	void preparation_mask(Mat &img, Mat &H, vector<Point2f> &corners, vector<Point2f> &corner_trans, Mat &mask, int linetype);

private: 
	//double XScale;
	//double YScale;
	//double x_start_scale;
	//double y_start_scale;

	bool gpu_flag = true;
	int match_filter = 1;

	double work_megapix = 0.6;

	CSettingStitchDlg setDlg;
	ObjectDetection odDlg;
	Util util;
};