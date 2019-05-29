#include "stdafx.h"
#include "Viewer.h"
#include "ObjectDetection.h"
#include "afxdialogex.h"

bool threadtest;
bool detectionstart;
Mat detectionMat;
mutex value_mutex;
dnn::Net ObjectDetection::create_Net(string model_Config, string model_Weight, int backend, int target)
{
	dnn::Net net = dnn::readNetFromDarknet(model_Config, model_Weight);

	if (net.empty())
	{
		cerr << "Can't load network by using the following files: " << endl;
		cerr << "cfg-file:     " << model_Config << endl;
		cerr << "weights-file: " << model_Weight << endl;
		cerr << "Models can be downloaded here : " << endl;
		cerr << "https://pjreddie.com/darknet/yolo/" << endl;
		exit(-1);
	}

	net.setPreferableBackend(backend);
	net.setPreferableTarget(target);

	return net;
}

vector<string> ObjectDetection::read_Class_Name(string path)
{
	vector<string> classNamesVec;

	ifstream classNamesFile("coco.names");  // .names

	if (classNamesFile.is_open()) {
		string className = "";
		while (getline(classNamesFile, className)) {
			classNamesVec.emplace_back(className);
		}
	}

	return classNamesVec;
}

void loop(dnn::Net & net, Mat & frame)
{
	while (threadtest)
	{
		Mat inputBlob = dnn::blobFromImage(frame, 1 / 255.f, Size(416, 416), Scalar(), true, false); // 416 * 416

		net.setInput(inputBlob, "data");

		vector<string> outNames = net.getUnconnectedOutLayersNames();

		//for (int i = 0; i < outNames.size(); i++)
		//	cout << outNames[i] << endl;

		Mat detectionMatCopy = net.forward(outNames[outNames.size() - 1]);
		value_mutex.lock();
		detectionMatCopy.copyTo(detectionMat);
		value_mutex.unlock();
		detectionstart = true;
	}
	return;
}
void ObjectDetection::object_Detection(VideoCapture & capture, float confidence_Threshold, dnn::Net & net, vector<string>& classNamesVec)
{
	int64 start_app;
	double time_ms;
	Mat frame;
	capture >> frame;
	if (frame.channels() == 4)
		cvtColor(frame, frame, COLOR_BGRA2BGR);
	threadtest = true;
	detectionstart = false;
	//object detection thread
	thread t1(loop, net, frame);

	for (;;) {
		start_app = getTickCount();
		Sleep(30);
		capture >> frame;

		if (frame.empty()) {
			break;
		}

		util.erase_Noise(frame);

		if (frame.channels() == 4)
			cvtColor(frame, frame, COLOR_BGRA2BGR);

		vector<double> layersTimings;
		//double tick_freq = getTickFrequency();
		//double time_ms = net.getPerfProfile(layersTimings) / tick_freq * 1000;
		time_ms = 1 / ((getTickCount() - start_app) / getTickFrequency());
		putText(frame, format("FPS: %.2f ;", time_ms),
			Point(20, 20), 0, 0.5, Scalar(0, 0, 255));
		if (detectionstart)
		{
			for (int i = 0; i < detectionMat.rows; i++) {

				const int probability_index = 5;
				const int probability_size = detectionMat.cols - probability_index;

				float *prob_array_ptr = &detectionMat.at<float>(i, probability_index);

				size_t objectClass = max_element(prob_array_ptr, prob_array_ptr + probability_size) - prob_array_ptr;
				float confidence = detectionMat.at<float>(i, (int)objectClass + probability_index);

				if (confidence > confidence_Threshold) {

					float x_center = detectionMat.at<float>(i, 0) * frame.cols;
					float y_center = detectionMat.at<float>(i, 1) * frame.rows;
					float width = detectionMat.at<float>(i, 2) * frame.cols;
					float height = detectionMat.at<float>(i, 3) * frame.rows;

					Point2f p1(cvRound(x_center - width / 2), cvRound(y_center - height / 2));
					Point2f p2(cvRound(x_center + width / 2), cvRound(y_center + height / 2));
					Rect object(p1, p2);

					Scalar object_roi_color(0, 255, 0);

					rectangle(frame, object, object_roi_color);

					String className = objectClass < classNamesVec.size() ? classNamesVec[objectClass] : cv::format("unknown(%d)", objectClass);
					String label = format("%s: %.2f", className.c_str(), confidence);
					int baseLine = 0;
					double fontSize = 0.3;
					Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, fontSize, 1, &baseLine);
					rectangle(frame, Rect(p1, Size(labelSize.width, labelSize.height + baseLine)), object_roi_color, FILLED);
					putText(frame, label, p1 + Point2f(0, labelSize.height), FONT_HERSHEY_SIMPLEX, fontSize, Scalar(0, 0, 0));
				}
			}
		}

		namedWindow("YOLO: Detections", WINDOW_NORMAL);
		imshow("YOLO: Detections", frame);
		if (waitKey(1) >= 0)
		{
			break;
		}
	}
	destroyAllWindows();
	threadtest = false;
	t1.join();
}