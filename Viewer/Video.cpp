#include "stdafx.h"
#include "Viewer.h"
#include "Video.h"
#include "afxdialogex.h"

void Video::Read_Video(VideoCapture &cap, Mat &img, string &file_name) {

	if (device_num == 0) {  // 0번 노트북 카메라

		cout << "Opening camera device number " << device_num << ". Please wait..." << endl;
		cap.open(device_num);
	}
	else if (device_num >= 1) { // IP 카메라

		string videoStreamAddress = "";  // IP address

		cout << "Opening Ip camera." << endl;
		cap.open(videoStreamAddress);

		cap.set(CAP_PROP_FRAME_WIDTH, 640);
		cap.set(CAP_PROP_FRAME_HEIGHT, 480);

	}
	else { // file

		/*string path;

		cout << "Enter Path : (.mp4 .mov) ";
		cin >> path;
		*/
		if (file_name == "") {
			return;
		}
		else {
			cap.open(file_name);
		}
		//cap.open(path);
	}

	CV_Assert(cap.isOpened());
	print_Info_Camera(cap);

	if (device_num >= 0) {

		for (int k = 0; k < 10; k++)
			cap >> img;
	}
	else
		cap >> img;

	util.erase_Noise(img);
}

void Video::print_Info_Camera(VideoCapture cr) {

	cout << "\nWidth : " << cr.get(CAP_PROP_FRAME_WIDTH) << endl;
	cout << "Height : " << cr.get(CAP_PROP_FRAME_HEIGHT) << endl;

	cout << "Exposure : " << cr.get(CAP_PROP_EXPOSURE) << endl;
	cout << "Brightness : " << cr.get(CAP_PROP_BRIGHTNESS) << endl;

	cout << "FPS : " << cr.get(CAP_PROP_FPS) << endl;
	cout << "Total Frame Count : " << cr.get(CAP_PROP_FRAME_COUNT) << endl << endl;
}