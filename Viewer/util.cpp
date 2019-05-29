#include "stdafx.h"
#include "Viewer.h"
#include "util.h"
#include "afxdialogex.h"

void Util::automatic_feature_detection_set(Mat img) {
	CSettingStitchDlg setDlg;

	int last_progress = 0;

	if (setDlg.GetdetectorType() == 4) { // SURF

		Ptr<SurfFeatureDetector> detectors = SurfFeatureDetector::create(SURF_hessianThreshold);

		vector<KeyPoint> keypoints_list;
		detectors->detect(img, keypoints_list);

		int max = (int)keypoints_list.size();
		while (keypoints_list.size() > setDlg.GetFeatureNum() + 50) {

			SURF_hessianThreshold += 100;
			detectors->setHessianThreshold(SURF_hessianThreshold);

			detectors->detect(img, keypoints_list);

			int progress = 100 - (((int)keypoints_list.size() - (setDlg.GetFeatureNum() + 48)) * 100 / max);
			if (progress >= 100 || progress < 0)
				progress = 100;

			if (last_progress != progress)
				std::cout << "Please wait:  It might take some minutes for configuration. " << progress << "%" << endl;

			last_progress = progress;
		}
		std::cout << "\nConfigured successfully. The parameter for Hessian Threshold is " << SURF_hessianThreshold << endl;

	}
	else if (setDlg.GetdetectorType() == 3) { // ORB
		std::cout << "ORB's Features_Num is " << setDlg.GetFeatureNum() << endl;
	}
	else if (setDlg.GetdetectorType() == 2) { // AKAZE

		Ptr<AKAZE> detectors = AKAZE::create(AKAZE::DescriptorType::DESCRIPTOR_MLDB, 0, 3, AKAZE_Threshold);

		vector<KeyPoint> keypoints_list;
		detectors->detect(img, keypoints_list);

		int max = (int)keypoints_list.size();
		while (keypoints_list.size() > setDlg.GetFeatureNum() + 50) {

			AKAZE_Threshold += 0.0002f; // 수정 필요.
			detectors->setThreshold(AKAZE_Threshold);

			detectors->detect(img, keypoints_list);
			int progress = 100 - (((int)keypoints_list.size() - (setDlg.GetFeatureNum() + 48)) * 100 / max);
			if (progress >= 100 || progress < 0)
				progress = 100;

			if (last_progress != progress)
				std::cout << "Please wait:  It might take some minutes for configuration. " << progress << "%" << endl;

			last_progress = progress;
		}
		std::cout << "\nConfigured successfully. The parameter for Akaze Threshold is " << AKAZE_Threshold << endl;

	}
	else if (setDlg.GetdetectorType() == 1) { // BRISK
		Ptr<BRISK> detectors = BRISK::create(BRISK_Threshold);

		vector<KeyPoint> keypoints_list;
		detectors->detect(img, keypoints_list);

		int max = (int)keypoints_list.size();
		while (keypoints_list.size() > setDlg.GetFeatureNum() + 50) {

			BRISK_Threshold += 5; // --?
			detectors = BRISK::create(BRISK_Threshold); // Fast -> Brisk도 고민을..

			detectors->detect(img, keypoints_list);
			int progress = 100 - (((int)keypoints_list.size() - (setDlg.GetFeatureNum() + 48)) * 100 / max);
			if (progress >= 100 || progress < 0)
				progress = 100;

			if (last_progress != progress)
				std::cout << "Please wait:  It might take some minutes for configuration. " << progress << "%" << endl;

			last_progress = progress;
		}
		std::cout << "\nConfigured successfully. The parameter for Brisk Threshold is " << BRISK_Threshold << endl;
	}
	else {
		std::cout << "Not exist detector type" << endl;
	}

}

void Util::print_Info_Camera(VideoCapture cr) {

	std::cout << "\nWidth : " << cr.get(CAP_PROP_FRAME_WIDTH) << endl;
	std::cout << "Height : " << cr.get(CAP_PROP_FRAME_HEIGHT) << endl;

	std::cout << "Exposure : " << cr.get(CAP_PROP_EXPOSURE) << endl;
	std::cout << "Brightness : " << cr.get(CAP_PROP_BRIGHTNESS) << endl;

	std::cout << "FPS : " << cr.get(CAP_PROP_FPS) << endl;
	std::cout << "Total Frame Count : " << cr.get(CAP_PROP_FRAME_COUNT) << endl << endl;
}

bool Util::cmp_Dmatch(DMatch a, DMatch b) {
	return a.distance < b.distance; // 오름차순
}

void Util::sharpen2D(const cv::Mat &image, cv::Mat &result) {

	// Construct kernel (all entries initialized to 0)
	cv::Mat kernel(3, 3, CV_32F, cv::Scalar(0));
	// assigns kernel values
	kernel.at<float>(1, 1) = 5.0;
	kernel.at<float>(0, 1) = -1.0;
	kernel.at<float>(2, 1) = -1.0;
	kernel.at<float>(1, 0) = -1.0;
	kernel.at<float>(1, 2) = -1.0;

	//filter the image
	cv::filter2D(image, result, image.depth(), kernel);

}

void Util::erase_Noise(Mat &img) {

	GaussianBlur(img, img, Size(5, 5), 1.0);
	sharpen2D(img, img);
}
float Util::GetSURFthreshold() {
	return SURF_hessianThreshold;
}

float Util::GetAKAZEthreshold() {
	return AKAZE_Threshold;
}

int Util::GetBRISKthreshold() {
	return BRISK_Threshold;
}

void Util::connect_gpu() {

	if (!ocl::haveOpenCL()) {
		cout << "에러 : OpenCL을 사용할 수 없는 시스템입니다." << endl;
		return;
	}

	if (!context.create(ocl::Device::TYPE_GPU)) {
		cout << " 에러 : 컨텍스트를 생성할 수 없습니다." << endl;
		return;
	}

	cout << endl << context.ndevices() << " GPU device (s) detected " << endl;

	for (size_t i = 0; i < context.ndevices(); i++) {
		ocl::Device device = context.device(i);
		cout << " - Device " << i << " --- " << endl;
		cout << " Name : " << device.name() << endl;
		cout << " Availability : " << device.available() << endl;
		cout << " Image Support : " << device.imageSupport() << endl;
		cout << " OpenCL C version : " << device.OpenCL_C_Version() << endl;
	}

	ocl::Device(context.device(0));
	ocl::setUseOpenCL(true);
	
}

ocl::Context Util::getContext() {
	return context;
}