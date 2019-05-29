// ImageStitch.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Viewer.h"
#include "ImageStitch.h"
#include "afxdialogex.h"
// CImageStitch 대화 상자입니다.

IMPLEMENT_DYNAMIC(CImageStitch, CDialogEx)

CImageStitch::CImageStitch(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IMAGE_STITCH, pParent)
{

}


CImageStitch::~CImageStitch()
{
}

void CImageStitch::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_FILE, m_fileList);
}


BEGIN_MESSAGE_MAP(CImageStitch, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CImageStitch::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &CImageStitch::OnBnDeleteClicked)
	ON_BN_CLICKED(IDOK, &CImageStitch::OnBnClickedOk)
END_MESSAGE_MAP()


void CImageStitch::OnBnClickedButtonAdd()
{
	// 초기 선택 폴더
	//CString strInitPath = _T(".//");

	// 폴더 선택 다이얼로그
	CFolderPickerDialog Picker(NULL, OFN_FILEMUSTEXIST, NULL, 0);

	if (Picker.DoModal() == IDOK)
	{
		// 선택된 폴더 경로얻음
		CString strFolderPath = Picker.GetPathName();

		CFileFind finder;
		BOOL bWorking = finder.FindFile(strFolderPath + "\\*.*");

		while (bWorking) {

			bWorking = finder.FindNextFileA();

			if (finder.IsArchived()) { // file

				CString _fileName = finder.GetFileName();

				if (_fileName == _T(".") ||
					_fileName == _T("..") ||
					_fileName == _T("Thumbs.db")) continue;

				m_fileList.AddString(_fileName);
				img_name.emplace_back(finder.GetFilePath());
			}
		}

	}
}

void CImageStitch::ImageStitching() {
	CSettingStitchDlg setDlg;
	Util util;
	double work_scale = 1, seam_scale = 1, compose_scale = 1;
	bool is_work_scale_set = false, is_seam_scale_set = false, is_compose_scale_set = false;

	Mat tmp = imread(img_name[0]);
	util.automatic_feature_detection_set(tmp);

	int64 t = getTickCount();
	Ptr<Feature2D> finder;

	if (setDlg.GetdetectorType() == 0) { // SURF
		SURF_hessianThreshold = util.GetSURFthreshold();
		finder = SURF::create(SURF_hessianThreshold);
		match_conf = 0.65f;
	}
	else if (setDlg.GetdetectorType() == 1) { // ORB
		finder = ORB::create(setDlg.GetFeatureNum());
	}
	else if (setDlg.GetdetectorType() == 2) { // AKAZE
		AKAZE_Threshold = util.GetAKAZEthreshold();
		finder = AKAZE::create(AKAZE::DescriptorType::DESCRIPTOR_MLDB, 0, 3, AKAZE_Threshold);
	}
	else if (setDlg.GetdetectorType() == 3) { // BRISK
		BRISK_Threshold = util.GetBRISKthreshold();
		finder = BRISK::create(BRISK_Threshold);
	}

	int num_images = img_name.size();
	Mat full_img, img;
	vector<ImageFeatures> features(num_images);

	vector<Mat> images(num_images);
	vector<Size> full_img_sizes(num_images);
	double seam_work_aspect = 1;

	for (int i = 0; i < num_images; i++) {

		full_img = imread(img_name[i]);
		full_img_sizes[i] = full_img.size();

		if (work_megapix < 0) {
			work_scale = 1;
			is_work_scale_set = true;
		}
		else {

			if (!is_work_scale_set)
			{
				work_scale = min(1.0, sqrt(work_megapix * 1e6 / full_img.size().area()));
				is_work_scale_set = true;
			}
			resize(full_img, img, Size(), work_scale, work_scale, INTER_LINEAR);
		}

		if (!is_seam_scale_set)
		{
			seam_scale = min(1.0, sqrt(seam_megapix * 1e6 / full_img.size().area()));
			seam_work_aspect = seam_scale / work_scale;
			is_seam_scale_set = true;
		}

		//finder->detectAndCompute(img, noArray(), features[i].keypoints, features[i].descriptors);
		computeImageFeatures(finder, img, features[i]);

		features[i].img_idx = i;
		cout << "Features in image #" << i + 1 << ": " << features[i].keypoints.size() << endl;
		resize(full_img, img, Size(), seam_scale, seam_scale, INTER_LINEAR);
		images[i] = img.clone();
	}

	finder->clear();
	full_img.release();
	img.release();

	cout << "Finding features, time : " << ((getTickCount() - t) / getTickFrequency()) << " sec" << endl;

	t = getTickCount();
	vector<MatchesInfo> pairwise_matches;
	BestOf2NearestMatcher matcher(try_gpu, match_conf);
	matcher(features, pairwise_matches);
	matcher.collectGarbage();

	cout << "Pairwise matching, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec" << endl;

	// matchesGraphAsString(img_names, pairwise_matches, conf_thresh);

	// Leave only images we are sure are from the same panorama
	vector<int> indices = leaveBiggestComponent(features, pairwise_matches, conf_thresh);
	vector<Mat> img_subset;
	vector<string> img_names_subset;
	vector<Size> full_img_sizes_subset;

	for (size_t i = 0; i < indices.size(); ++i)
	{
		img_names_subset.emplace_back(img_name[indices[i]]);
		img_subset.emplace_back(images[indices[i]]);
		full_img_sizes_subset.emplace_back(full_img_sizes[indices[i]]);
	}

	images = img_subset;
	img_name = img_names_subset;
	full_img_sizes = full_img_sizes_subset;

	num_images = static_cast<int>(img_name.size());
	// Check if we still have enough images
	if (num_images < 2) {
		cout << "Need more images\n";
		return;
	}

	t = getTickCount();

	HomographyBasedEstimator estimator;
	vector<CameraParams> cameras;

	if (!estimator(features, pairwise_matches, cameras)) {

		cout << "Estimator Error!!" << endl;
		return;
	}

	cout << "Homography-based init, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec" << endl;

	for (size_t i = 0; i < cameras.size(); ++i)
	{
		Mat R;
		cameras[i].R.convertTo(R, CV_32F);
		cameras[i].R = R;
		//cout << "Initial intrinsics #" << indices[i] + 1 << ":\n" << cameras[i].K() << endl;
	}

	t = getTickCount();

	Ptr<BundleAdjusterBase> adjuster;

	if (ba_cost_func == "reproj")
		adjuster = new detail::BundleAdjusterReproj();
	else if (ba_cost_func == "ray")
		adjuster = new detail::BundleAdjusterRay();
	else if (ba_cost_func == "affine") {
		adjuster = new detail::BundleAdjusterAffine();
	}
	else if (ba_cost_func == "affinepartial")
		adjuster = new detail::BundleAdjusterAffinePartial();
	else
	{
		cout << "Unknown bundle adjustment cost function : '" << ba_cost_func << "'." << endl;
		return;
	}

	adjuster->setConfThresh(conf_thresh);
	Mat_<uchar> refine_mask = Mat::zeros(3, 3, CV_8U);
	if (ba_refine_mask[0] == 'x') refine_mask(0, 0) = 1;
	if (ba_refine_mask[1] == 'x') refine_mask(0, 1) = 1;
	if (ba_refine_mask[2] == 'x') refine_mask(0, 2) = 1;
	if (ba_refine_mask[3] == 'x') refine_mask(1, 1) = 1;
	if (ba_refine_mask[4] == 'x') refine_mask(1, 2) = 1;
	adjuster->setRefinementMask(refine_mask);
	(*adjuster)(features, pairwise_matches, cameras);

	cout << "Bundle Adjustment, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec" << endl;

	vector<double> focals;

	for (size_t i = 0; i < cameras.size(); ++i)
	{
		//cout << "Camera #" << indices[i] + 1 << ":\n" << cameras[i].K() << endl;
		focals.emplace_back(cameras[i].focal);
	}

	sort(focals.begin(), focals.end());

	float warped_image_scale;
	if (focals.size() % 2 == 1)
		warped_image_scale = static_cast<float>(focals[focals.size() / 2]);
	else
		warped_image_scale = static_cast<float>(focals[focals.size() / 2 - 1] + focals[focals.size() / 2]) * 0.5f;

	if (do_wave_correct)
	{
		vector<Mat> rmats;
		for (size_t i = 0; i < cameras.size(); ++i)
			rmats.emplace_back(cameras[i].R);

		waveCorrect(rmats, wave_correct);

		for (size_t i = 0; i < cameras.size(); ++i)
			cameras[i].R = rmats[i];
	}

	t = getTickCount();

	vector<Point> corners(num_images);
	vector<UMat> masks_warped(num_images);
	vector<UMat> images_warped(num_images);
	vector<Size> sizes(num_images);
	vector<UMat> masks(num_images);

	// Preapre images masks
	for (int i = 0; i < num_images; ++i)
	{
		masks[i].create(images[i].size(), CV_8U);
		masks[i].setTo(Scalar::all(255));
	}

	Ptr<WarperCreator> warper_creator;

	if (warp_type == "plane")
		warper_creator = new cv::PlaneWarper();
	else if (warp_type == "cylindrical")
		warper_creator = new cv::CylindricalWarper();
	else if (warp_type == "spherical")
		warper_creator = new cv::SphericalWarper();
	else if (warp_type == "transverseMercator")
		warper_creator = new cv::TransverseMercatorWarper();
	else if (warp_type == "mercator")
		warper_creator = new cv::MercatorWarper();

	if (warper_creator.empty())
	{
		cout << "Can't create the following warper '" << warp_type << "'\n";
		return;
	}

	Ptr<RotationWarper> warper = warper_creator->create(static_cast<float>(warped_image_scale * seam_work_aspect));

	for (int i = 0; i < num_images; ++i)
	{
		Mat_<float> K;
		cameras[i].K().convertTo(K, CV_32F);
		float swa = (float)seam_work_aspect;
		K(0, 0) *= swa; K(0, 2) *= swa;
		K(1, 1) *= swa; K(1, 2) *= swa;

		corners[i] = warper->warp(images[i], K, cameras[i].R, INTER_LINEAR, BORDER_REFLECT, images_warped[i]);
		sizes[i] = images_warped[i].size();

		warper->warp(masks[i], K, cameras[i].R, INTER_LINEAR, BORDER_CONSTANT, masks_warped[i]);

	}

	vector<UMat> images_warped_f(num_images);
	for (int i = 0; i < num_images; ++i)
		images_warped[i].convertTo(images_warped_f[i], CV_32F);

	cout << "Warping images, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec" << endl;

	t = getTickCount();

	Ptr<detail::ExposureCompensator> compensator = detail::ExposureCompensator::createDefault(expos_comp_type);
	compensator->feed(corners, images_warped, masks_warped);

	Ptr<detail::SeamFinder> seam_finder;

	if (seam_find_type == "no")
		seam_finder = new detail::NoSeamFinder();
	else if (seam_find_type == "voronoi")
		seam_finder = new detail::VoronoiSeamFinder();
	else if (seam_find_type == "gc_color") {
		seam_finder = new detail::GraphCutSeamFinder(detail::GraphCutSeamFinderBase::COST_COLOR);
	}
	else if (seam_find_type == "gc_colorgrad") {
		seam_finder = new detail::GraphCutSeamFinder(detail::GraphCutSeamFinderBase::COST_COLOR_GRAD);
	}

	if (seam_finder.empty())
	{
		cout << "Can't create the following seam finder '" << seam_find_type << "'\n";
		return;
	}

	seam_finder->find(images_warped_f, corners, masks_warped);

	waitKey(0);
	images.clear();
	masks.clear();

	images_warped.clear();
	images_warped_f.clear();

	cout << "Gain compensation, Seam finder, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec" << endl;

	t = getTickCount();

	Mat img_warped, img_warped_s;
	Mat dilated_mask, seam_mask, mask, mask_warped;
	Ptr<detail::Blender> blender;

	double compose_seam_aspect = 1;
	double compose_work_aspect = 1;

	for (int img_idx = 0; img_idx < num_images; ++img_idx)
	{
		cout << "Compositing image #" << indices[img_idx] + 1 << endl;

		// Read image and resize it if necessary
		full_img = imread(img_name[img_idx]);
		if (!is_compose_scale_set)
		{
			if (compose_megapix > 0)
				compose_scale = min(1.0, sqrt(compose_megapix * 1e6 / full_img.size().area()));
			is_compose_scale_set = true;

			// Compute relative scales
			compose_seam_aspect = compose_scale / seam_scale;
			compose_work_aspect = compose_scale / work_scale;

			// Update warped image scale
			warped_image_scale *= static_cast<float>(compose_work_aspect);
			warper = warper_creator->create(warped_image_scale);

			// Update corners and sizes
			for (int i = 0; i < num_images; ++i)
			{
				// Update intrinsics
				cameras[i].focal *= compose_work_aspect;
				cameras[i].ppx *= compose_work_aspect;
				cameras[i].ppy *= compose_work_aspect;

				// Update corner and size
				Size sz = full_img_sizes[i];
				if (std::abs(compose_scale - 1) > 1e-1)
				{
					sz.width = cvRound(full_img_sizes[i].width * compose_scale);
					sz.height = cvRound(full_img_sizes[i].height * compose_scale);
				}

				Mat K;
				cameras[i].K().convertTo(K, CV_32F);
				Rect roi = warper->warpRoi(sz, K, cameras[i].R);
				corners[i] = roi.tl();
				sizes[i] = roi.size();
			}
		}
		if (abs(compose_scale - 1) > 1e-1)
			resize(full_img, img, Size(), compose_scale, compose_scale);
		else
			img = full_img;
		full_img.release();
		Size img_size = img.size();

		Mat K;
		cameras[img_idx].K().convertTo(K, CV_32F);

		// Warp the current image
		warper->warp(img, K, cameras[img_idx].R, INTER_LINEAR, BORDER_REFLECT, img_warped);

		// Warp the current image mask
		mask.create(img_size, CV_8U);
		mask.setTo(Scalar::all(255));
		warper->warp(mask, K, cameras[img_idx].R, INTER_NEAREST, BORDER_CONSTANT, mask_warped);

		// Compensate exposure
		compensator->apply(img_idx, corners[img_idx], img_warped, mask_warped);

		img_warped.convertTo(img_warped_s, CV_16S);
		img_warped.release();
		img.release();
		mask.release();

		dilate(masks_warped[img_idx], dilated_mask, Mat());
		resize(dilated_mask, seam_mask, mask_warped.size());
		mask_warped = seam_mask & mask_warped;

		//imshow(to_string(img_idx), mask_warped);

		if (blender.empty())
		{
			blender = detail::Blender::createDefault(blend_type, try_gpu);
			Size dst_sz = detail::resultRoi(corners, sizes).size();
			float blend_width = sqrt(static_cast<float>(dst_sz.area())) * blend_strength / 100.f;
			if (blend_width < 1.f)
				blender = Blender::createDefault(Blender::NO, try_gpu);
			else if (blend_type == Blender::MULTI_BAND)
			{
				MultiBandBlender* mb = dynamic_cast<MultiBandBlender*>(static_cast<Blender*>(blender));
				mb->setNumBands(static_cast<int>(ceil(log(blend_width) / log(2.)) - 1.));
				cout << "Multi-band blender, number of bands: " << mb->numBands() << endl;
			}
			else if (blend_type == Blender::FEATHER)
			{
				FeatherBlender* fb = dynamic_cast<FeatherBlender*>(static_cast<Blender*>(blender));
				fb->setSharpness(1.f / blend_width);
				cout << "Feather blender, sharpness: " << fb->sharpness() << endl;
			}
			blender->prepare(corners, sizes);
		}

		// Blend the current image
		blender->feed(img_warped_s, mask_warped, corners[img_idx]);

	}

	Mat result, result_mask;
	blender->blend(result, result_mask);

	cout << "Compositing, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec" << endl;
	//cout << "Finished, total time: " << ((getTickCount() - app_start_time) / getTickFrequency()) << " sec" << endl;

	result.convertTo(result, CV_8UC3);
	imwrite("./result/" + setDlg.GetSaveName(), result);

	namedWindow("PANO", WINDOW_NORMAL);
	imshow("PANO", result);
	waitKey(0);

}




void CImageStitch::OnBnDeleteClicked()
{
	int index = m_fileList.GetCurSel();

	if (index != LB_ERR) {
		CString str;
		m_fileList.GetText(index, str);
		m_fileList.DeleteString(index);
		img_name.erase(img_name.begin() + index);
	}
	
}


void CImageStitch::OnBnClickedOk()
{
	
	ImageStitching();
	CDialogEx::OnOK();
}
