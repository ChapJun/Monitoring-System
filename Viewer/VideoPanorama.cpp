#include "stdafx.h"
#include "Viewer.h"
#include "VideoPanorama.h"
#include "afxdialogex.h"

bool loop;
bool firstdetection;
Mat detectionMat2;
bool cmp_Dmatch(DMatch a, DMatch b) {
	return a.distance < b.distance;
}

void VideoPanorama::set_Start_loc(double &height, double &width, Mat &img) {

	if (setDlg.GetStartX() < 0)
		width = img.cols * (setDlg.GetScaleX() / 2 - 0.5);
	else
		width = img.cols * (setDlg.GetStartX());

	if (setDlg.GetStartY() < 0)
		height = img.rows * (setDlg.GetScaleY() / 2 - 0.5);
	else
		height = img.rows * (setDlg.GetStartY());
}

void VideoPanorama::min_max_distance(vector <DMatch> &matches, double &min_dist, double &max_dist) {

	for (int i = 0; i < (int)matches.size(); i++)
	{
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}
}

void VideoPanorama::distance_good_matches(vector<DMatch> &matches, vector<DMatch> &good_matches) {

	double max_dist = 0; double min_dist = 100;

	min_max_distance(matches, min_dist, max_dist);

	for (int i = 0; i < matches.size() && i < 50; i++)
	{
		if (matches[i].distance <= 4 * min_dist)
		{
			good_matches.emplace_back(matches[i]);
		}
	}
}

void VideoPanorama::distance_good_matches(vector<DMatch> &matches, vector<DMatch> &good_matches, double min_dist) {

	sort(matches.begin(), matches.end(), cmp_Dmatch);
	for (int i = 0; i < (int)matches.size() && i < 50; i++)
	{
		if (matches[i].distance <= 4 * min_dist)
		{
			good_matches.emplace_back(matches[i]);
		}
	}
}

void VideoPanorama::norm_good_matches(vector<DMatch> &matches, vector<DMatch> &good_matches1, vector<KeyPoint> &last, vector<KeyPoint> &cur, int img_rows) {

	int counterx;
	double res;

	for (int i = 0; i < (int)matches.size(); i++) {

		counterx = 0;

		for (int j = 0; j < (int)matches.size(); j++) {
			if (i != j) {

				res = cv::norm(last[matches[i].queryIdx].pt - last[matches[j].queryIdx].pt) - cv::norm(cur[matches[i].trainIdx].pt - cur[matches[j].trainIdx].pt);

				if (abs(res) < (img_rows*0.03 + 3)) { //this value(0.03) has to be adjusted
					counterx++;
				}
			}
		}
		if (counterx > (matches.size() / 10)) {
			good_matches1.emplace_back(matches[i]);
		}
	}
}

Mat VideoPanorama::make_correlation_norm(Mat &H, Mat &H_old, double &now) {

	Mat correlation;

	H.convertTo(H, CV_32F);
	H_old.convertTo(H_old, CV_32F);

	matchTemplate(H_old, H, correlation, TM_CCOEFF_NORMED);
	correlation.convertTo(correlation, CV_64F);
	cout << "correlation : " << correlation << endl;

	now = norm(H - H_old, 2);
	H.convertTo(H, CV_64F);
	H_old.convertTo(H_old, CV_64F);

	return correlation;
}

void VideoPanorama::preparation_mask(Mat &img, Mat &H, vector<Point2f> &corners, vector<Point2f> &corner_trans, Mat &mask, int linetype) {

	corners[0] = Point2f(0, 0); // top left
	corners[1] = Point2f(0, (float)img.rows); // top right
	corners[2] = Point2f((float)img.cols, 0); // bottom left
	corners[3] = Point2f((float)img.cols, (float)img.rows); // bottom right
	perspectiveTransform(corners, corner_trans, H);

	vector<Point> line1;

	line1.push_back(corner_trans[0]);
	line1.push_back(corner_trans[2]);
	line1.push_back(corner_trans[3]);
	line1.push_back(corner_trans[1]);

	fillConvexPoly(mask, line1, Scalar::all(255), linetype);

	line(mask, corner_trans[0], corner_trans[2], Scalar::all(0), 4, linetype);
	line(mask, corner_trans[2], corner_trans[3], Scalar::all(0), 4, linetype);
	line(mask, corner_trans[3], corner_trans[1], Scalar::all(0), 4, linetype);
	line(mask, corner_trans[1], corner_trans[0], Scalar::all(0), 4, linetype);

}

void VideoPanorama::video_panorama(VideoCapture &cap, Mat img, int type) {
	double time_algorithm = 0;
	double time_complete = 0;

	double norms = 0;

	Mat gray_lastimage, gray_curimage;
	Mat img_last, img_cur;

	Mat mask;
	Mat img_last_scaled, img_cur_scaled;

	double fps = 0;

	Mat offset = Mat::eye(3, 3, CV_64F);
	int counter = 0;

	//print_Info_Pano();

	double start_width;
	double start_height;

	set_Start_loc(start_height, start_width, img);

	Mat final_img(Size(int(img.cols * setDlg.GetScaleX()), int(img.rows * setDlg.GetScaleY())), CV_8UC3);
	Mat f_roi(final_img, Rect((int)start_width, (int)start_height, img.cols, img.rows));
	img.copyTo(f_roi);
	img.copyTo(img_last);

	Size size_wrap(final_img.cols, final_img.rows);
	char key;
	Mat rImg;

	double work_scale = min(1.0, sqrt(work_megapix * 1e6 / img.size().area()));
	resize(img_last, img_last_scaled, Size(), work_scale, work_scale, INTER_CUBIC);
	cvtColor(img_last_scaled, gray_lastimage, COLOR_BGR2GRAY);

	util.automatic_feature_detection_set(gray_lastimage);

	Ptr<FeatureDetector> detector;

	vector<KeyPoint> keypoints_last, keypoints_cur;
	Mat descriptors_last, descriptors_cur;

	if (setDlg.GetdetectorType() == 0) {
		detector = SURF::create(util.GetSURFthreshold());
	}
	else if (setDlg.GetdetectorType() == 1) {
		detector = ORB::create(setDlg.GetFeatureNum());
	}
	else if (setDlg.GetdetectorType() == 2) {
		detector = AKAZE::create(AKAZE::DescriptorType::DESCRIPTOR_MLDB, 0, 3, util.GetAKAZEthreshold());
	}
	else if (setDlg.GetdetectorType() == 3) {
		detector = BRISK::create(util.GetBRISKthreshold());
	}
	else {
		//cout << "Unknown 2D features type : '" << detector_type << "'." << endl;
		return;
	}

	detector->detectAndCompute(gray_lastimage, noArray(), keypoints_last, descriptors_last);

	double gui_time;
	int64 t, start_app, start_algorithm;
	Mat panorama_temp;
	int64 start_app_main = getTickCount();
	bool last_err = false;

	bool first_time_4pointfound = false;
	bool second_time_4pointfound = false;

	// 항등행렬
	Mat H_kol = Mat::eye(3, 3, CV_32F);
	Mat H = cv::Mat::eye(3, 3, CV_64F);
	Mat H2 = Mat::eye(2, 3, CV_32F);
	Mat H_old = Mat::eye(3, 3, CV_32F);

	util.connect_gpu();

	//added
	//object detction 초기설정
	dnn::Net net;
	vector<string> classNamesVec;
	float confidence_Threshold;

	//backgruond subtraction 초기설정
	/*
	//bgsLibrary background subtraciton
	auto bgs = bgslibrary::algorithms::DPPratiMediod();
	*/
	//auto bgs = createBackgroundSubtractorMOG2(500, 16.0, true);
	//end

	thread t1;
	if (type == 2) {
		string model_Config = "yolov3-tiny.cfg";
		string model_Weight = "yolov3-tiny.weights";

		net = odDlg.create_Net(model_Config, model_Weight, dnn::DNN_BACKEND_DEFAULT, dnn::DNN_TARGET_OPENCL);

		classNamesVec = odDlg.read_Class_Name("coco.names");

		confidence_Threshold = 0.24f;

		cap >> img_cur;

		loop = true;
		firstdetection = false;
		t1 = thread([](dnn::Net & net, Mat & img_cur) {
			Mat frame;
			img_cur.copyTo(frame);
			if (frame.channels() == 4)
				cvtColor(frame, frame, COLOR_BGRA2BGR);
			while (loop)
			{
				Mat inputBlob = dnn::blobFromImage(img_cur, 1 / 255.f, Size(416, 416), Scalar(), true, false); // 416 * 416

				net.setInput(inputBlob, "data");

				vector<string> outNames = net.getUnconnectedOutLayersNames();

				//for (int i = 0; i < outNames.size(); i++)
				//	cout << outNames[i] << endl;

				Mat detectionMatCopy = net.forward(outNames[outNames.size() - 1]);
				//value_mutex.lock();
				detectionMatCopy.copyTo(detectionMat2);
				//value_mutex.unlock();
				firstdetection = true;
			}
			return;
		}, net, img_cur);
	}

	while (true) {

		start_app = getTickCount();
		counter++;
		cout << "------------------------------------------------------------------------------------" << endl;
		cout << "frame count : " << counter << endl;
		cap >> img_cur;

		start_algorithm = getTickCount();
		if (img_cur.empty())
			break;

		util.erase_Noise(img_cur);

		namedWindow("current video", WINDOW_NORMAL);
		imshow("current video", img_cur);
		waitKey(1);

		resize(img_cur, img_cur_scaled, Size(), work_scale, work_scale, INTER_LINEAR);
		if (img_cur_scaled.empty())
			break;

		cvtColor(img_cur_scaled, gray_curimage, COLOR_BGR2GRAY);

		t = getTickCount();

		detector->detectAndCompute(gray_curimage, noArray(), keypoints_cur, descriptors_cur);

		cout << "features = " << keypoints_cur.size() << endl;
		cout << "Finding features, time : " << ((getTickCount() - t) / getTickFrequency()) << " sec" << endl;

		if (descriptors_last.type() != CV_32F) {
			descriptors_last.convertTo(descriptors_last, CV_32F);
		}
		if (descriptors_cur.type() != CV_32F) {
			descriptors_cur.convertTo(descriptors_cur, CV_32F);
		}

		t = getTickCount();

		vector<DMatch> matches;
		Ptr<BFMatcher> matcher = BFMatcher::create(NORM_L2, true);
		matcher->match(descriptors_last, descriptors_cur, matches);

		if (matches.empty()) {
			last_err = false;
			continue;
		}

		vector<DMatch> good_matches1;
		vector<DMatch> good_matches2;

		sort(matches.begin(), matches.end(), cmp_Dmatch);

		// 최대 거리보다 4배 적은 거리의 처음 50개까지를 고려
		if (match_filter == 0)
		{
			distance_good_matches(matches, good_matches2);
		}
		// 거리의 표준을 계산.
		else if (match_filter == 1) {

			norm_good_matches(matches, good_matches1, keypoints_last, keypoints_cur, img.rows);

			// same match filter 0.
			double max_dist = 0; double min_dist = 100;
			min_max_distance(matches, min_dist, max_dist);

			cout << "max_dist : " << max_dist << endl;
			cout << "min_dist : " << min_dist << endl;

			//take just the good points
			if ((max_dist == 0) && (min_dist == 0))
			{
				last_err = false;
				continue;
			}

			distance_good_matches(good_matches1, good_matches2, min_dist);
		}
		//no filter
		else if (match_filter == 2)
			good_matches2 = matches;

		cout << "Good matches features = " << good_matches2.size() << endl;

		vector< Point2f > obj_last;
		vector< Point2f > scene_cur;

		for (int i = 0; i < good_matches2.size(); i++)
		{
			obj_last.emplace_back(keypoints_last[good_matches2[i].queryIdx].pt);
			scene_cur.emplace_back(keypoints_cur[good_matches2[i].trainIdx].pt);
		}

		cout << "match time : " << ((getTickCount() - t) / getTickFrequency()) << endl;

		t = getTickCount();
		Mat mat_match;

		if (scene_cur.size() >= 4)
		{
			first_time_4pointfound = true;

			//drawing some features and matches
			drawMatches(img_last, keypoints_last, img_cur, keypoints_cur, good_matches2, mat_match);
			namedWindow("match", WINDOW_NORMAL);
			imshow("match", mat_match);

			if (counter == 1)
				waitKey(0);

			if (setDlg.GetwarpingType() == 0) {

				H2 = estimateAffinePartial2D(scene_cur, obj_last, noArray(), RANSAC);

				if (H2.data == NULL) {
					last_err = false;
					good_matches1.clear();
					good_matches2.clear();
					scene_cur.clear();
					obj_last.clear();
					continue;
				}

				H.at<double>(0, 0) = H2.at<double>(0, 0);
				H.at<double>(0, 1) = H2.at<double>(0, 1);
				H.at<double>(0, 2) = H2.at<double>(0, 2);
				H.at<double>(1, 0) = H2.at<double>(1, 0);
				H.at<double>(1, 1) = H2.at<double>(1, 1);
				H.at<double>(1, 2) = H2.at<double>(1, 2);

				cout << "H = " << H2 << endl;
			}
			else if (setDlg.GetwarpingType() == 1) {

				H = findHomography(scene_cur, obj_last, RANSAC, 3); // 3 x 3

				if (H.empty()) {
					good_matches1.clear();
					good_matches2.clear();
					scene_cur.clear();
					obj_last.clear();
					continue;
				}
				cout << "H = " << H << endl;
			}

			// using correlations and norms to find the errors and skip that frame.
			Mat correlation;
			double nownorms;

			correlation = make_correlation_norm(H, H_old, nownorms);

			cout << "now norm : " << nownorms << endl;
			cout << "norm : " << norms << endl;

			if (norms == 0)
				norms = nownorms;


			if ((nownorms > 2 * norms) && (abs(correlation.at<float>(0)) < 0.8) || (nownorms > 10 * norms))
			{
				
				if (!last_err && (counter != 1))
				{
					if (log_flag) {
					cout << "Error!!" << endl;
						string name = "Error_frame_" + to_string(counter) + ".jpg";
						imwrite(name, panorama_temp);
					}

					last_err = true;
					good_matches1.clear();
					good_matches2.clear();
					scene_cur.clear();
					obj_last.clear();
					continue;
				}
				else
					last_err = false;

			}
			else {
				norms = (norms* (counter - 1) + nownorms) / counter;
				last_err = false;
			}

			offset.at<double>(0, 2) = start_width;
			offset.at<double>(1, 2) = start_height;

			if (first_time_4pointfound == true && second_time_4pointfound == false)
			{
				H_kol = offset*H;
				second_time_4pointfound = true;
			}
			else
			{
				H_kol = H_kol*H;
			}

			cout << "Homography time : " << ((getTickCount() - t) / getTickFrequency()) << endl;

			t = getTickCount();

			if (gpu_flag) {

				UMat rImg_gpu, img_cur_gpu;

				img_cur_gpu = img_cur.getUMat(ACCESS_READ);
				warpPerspective(img_cur_gpu, rImg_gpu, H_kol, size_wrap, INTER_LINEAR, BORDER_CONSTANT);
				rImg_gpu.copyTo(rImg);

			}
			else {

				Mat rImg_cpu, img_cur_cpu;

				img_cur.copyTo(img_cur_cpu);
				warpPerspective(img_cur_cpu, rImg_cpu, H_kol, size_wrap, INTER_LINEAR, BORDER_CONSTANT); // INTER_NEAREST, INTER_LINEAR, INTER_CUBIC
				rImg_cpu.copyTo(rImg);
			}

			cout << "warpPerspective time : " << ((getTickCount() - t) / getTickFrequency()) << endl;

			// Gain compensation: Normalise the brightness and contrast of all images.

			t = getTickCount();

			mask = cv::Mat::ones(final_img.size(), CV_8U) * 0;
			vector<Point2f> corners(4), corner_trans(4);
			preparation_mask(img, H_kol, corners, corner_trans, mask, LINE_AA);

			cout << "mask time: " << ((getTickCount() - t) / getTickFrequency()) << endl;

			// Seam Finding
			// Blending: Pixels along the stitch seam are blended to minimise visibility of stitch seams.

			rImg.copyTo(final_img, mask);

			t = getTickCount();

			final_img.copyTo(panorama_temp);

			line(final_img, corner_trans[0], corner_trans[2], Scalar(0, 0, 255), 4, 4);
			line(final_img, corner_trans[2], corner_trans[3], Scalar(0, 0, 255), 4, 4);
			line(final_img, corner_trans[3], corner_trans[1], Scalar(0, 0, 255), 4, 4);
			line(final_img, corner_trans[1], corner_trans[0], Scalar(0, 0, 255), 4, 4);

			//added
			//object detection call & get warping & draw line

			if (type == 2) {

				Mat frame;
				img_cur.copyTo(frame);

				//odDlg.object_detection_warping(final_img, frame, confidence_Threshold, net, classNamesVec, H_kol, setDlg.GetScaleX(), setDlg.GetScaleY());
				util.erase_Noise(frame);

				if (frame.channels() == 4)
					cvtColor(frame, frame, COLOR_BGRA2BGR);
				if (firstdetection)
				{
					for (int i = 0; i < detectionMat2.rows; i++) {

						const int probability_index = 5;
						const int probability_size = detectionMat2.cols - probability_index;

						float *prob_array_ptr = &detectionMat2.at<float>(i, probability_index);

						size_t objectClass = max_element(prob_array_ptr, prob_array_ptr + probability_size) - prob_array_ptr;
						float confidence = detectionMat2.at<float>(i, (int)objectClass + probability_index);

						if (confidence > confidence_Threshold) {
							float x_center = detectionMat2.at<float>(i, 0) * frame.cols;
							float y_center = detectionMat2.at<float>(i, 1) * frame.rows;
							float width = detectionMat2.at<float>(i, 2) * frame.cols;
							float height = detectionMat2.at<float>(i, 3) * frame.rows;

							Point2f p1(cvRound(x_center - width / 2), cvRound(y_center - height / 2));
							Point2f p2(cvRound(x_center + width / 2), cvRound(y_center + height / 2));

							vector<Point2f> coordinate;
							coordinate.push_back(p1);
							coordinate.push_back(p2);
							perspectiveTransform(coordinate, coordinate, H_kol);

							Rect object(coordinate[0], coordinate[1]);

							Scalar object_roi_color(0, 255, 0);

							rectangle(final_img, object, object_roi_color);

							String className = objectClass < classNamesVec.size() ? classNamesVec[objectClass] : cv::format("unknown(%d)", objectClass);
							String label = format("%s: %.2f", className.c_str(), confidence);
							int baseLine = 0;

							double fontSize = 0.3 + (setDlg.GetScaleX() + setDlg.GetScaleY()) / 10;

							Size labelSize = getTextSize(label, FONT_HERSHEY_DUPLEX, fontSize, 2, &baseLine);
							rectangle(final_img, Rect(coordinate[0], Size(labelSize.width, labelSize.height + baseLine)), object_roi_color, FILLED);
							putText(final_img, label, coordinate[0] + Point2f(0, labelSize.height), FONT_HERSHEY_DUPLEX, fontSize, Scalar(0, 0, 0), 2, LINE_AA);
						}
					}
				}
			}

			fps = 1 / ((getTickCount() - start_app) / getTickFrequency());

			putText(final_img, format("FPS: %.2f", fps),
				Point(100, 100), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0, 0, 255), 4, 8);
			putText(final_img, "Frame=" + std::to_string(counter), Point2f(100, 150), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0, 0, 255), 4, 8);

			namedWindow("Img", WINDOW_NORMAL);
			imshow("Img", final_img);

			gui_time = ((getTickCount() - t) / getTickFrequency());
			cout << "GUI time : " << gui_time << endl;

			t = getTickCount();

			gray_curimage.copyTo(gray_lastimage);
			img_cur.copyTo(img_last);
			keypoints_last = keypoints_cur;
			descriptors_last = descriptors_cur;
			H.copyTo(H_old);
			last_err = false;

			time_algorithm += (getTickCount() - start_algorithm) / getTickFrequency() - gui_time;
			time_complete += (getTickCount() - start_app) / getTickFrequency() - gui_time;

			cout << "loop prepration time : " << ((getTickCount() - t) / getTickFrequency()) << endl;
		}
		else {
			cout << "match point are less than 3 or 4 for homography" << endl;
		}

		good_matches1.clear();
		good_matches2.clear();
		key = waitKey(2);

		panorama_temp.copyTo(final_img);

		if (key == 'e') {
			destroyAllWindows();
			break;
		}

		if (key == 'r')
		{
			string name = "./result/reset_frame_" + to_string(counter) + ".jpg";
			imwrite(name, panorama_temp);
			panorama_temp = panorama_temp * 0;
			final_img = final_img * 0;

			H.at<double>(2, 0) = 0;
			H.at<double>(2, 1) = 0;
			H.at<double>(2, 2) = 1;

			H_kol = Mat::eye(3, 3, CV_64F);

			first_time_4pointfound = true;
			second_time_4pointfound = false;

			//copy current img in the middle of fov
			Mat f_roi(panorama_temp, Rect((int)start_width, (int)start_height, img_last.cols, img_last.rows));
			cap >> img_last;
			img_last.copyTo(f_roi);

		}

	}
	if (type == 2)
	{
		loop = false;
		t1.join();
	}
	cout << "Total time from start : " << ((getTickCount() - start_app_main) / getTickFrequency()) << endl;
	cout << "Total time with hard/webcam consideration : " << time_complete << endl;
	cout << "Total time algorithm : " << time_algorithm << endl;
	imwrite("./result/" + setDlg.GetSaveName(), panorama_temp);

	return;
}

