// FireDetect.cpp : Defines the entry point for the console application.
//

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <algorithm>
#include <iomanip>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "libssd.h"
#include "pmedia\videoitem.h"


const char *keys =
{
	"{vdf|video_file              |           |set video file name}"
	"{th|confidence_threshold  | 0.3       |confidence threshold}"
};

void usages(const char *path)
{
	const char *fname = strrchr(path, '\\');
	if (fname != NULL)
		fname++;
	else
		fname = path;
	std::cout << fname << " model_file weight_file" << std::endl;
}

float s_confidence_threshold = 0.3f;

cv::Scalar getColor(int label)
{
	cv::Scalar clr(255, 255, 255);

	switch (label) {
	case 0:
		clr = cv::Scalar(0, 0, 255);
		break;
	case 1:
		clr = cv::Scalar(255, 0, 0);
		break;
	default:
		clr = cv::Scalar(0, 0, 0);
		break;
	}

	return clr;
}

void detect(ssd::handle hd, cv::Mat& img)
{
	const float threshold = (float)s_confidence_threshold;

	ssd::DetectedObjList dets;
	ssd::detect(hd, img.cols, img.rows, img.type(), img.data, (int)img.step[0], threshold, &dets);

	for (int i = 0; i < dets.size(); ++i) {
		const ssd::DetectedObj& det = dets[i];
		cv::Scalar clr = getColor(det.label);

		int x = (det.x < 0) ? 0 : det.x;
		int y = (det.y < 0) ? 0 : det.y;
		int width = (x + det.width > img.cols) ? img.cols - x : det.width;
		int height = (y + det.height > img.rows) ? img.rows - y : det.height;

		cv::rectangle(img, cv::Rect(x, y, width, height), clr, 2);
	}
	

}

int main(int argc, char** argv)
{
	cv::CommandLineParser parser(argc, argv, keys);
	if (argc < 3) {
		usages(argv[0]);
		parser.printParams();
		return 1;
	}

	const std::string& plate_model_file = argv[1];
	const std::string& plate_weights_file = argv[2];
	int width = std::stoi(argv[3]);
	int height = std::stoi(argv[4]);
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@%d,   ###########  %d", width, height);

	std::string& video_file = parser.get<std::string>("vdf");
	s_confidence_threshold = parser.get<float>("th");

	ssd::handle h_detector = ssd::createDetector(plate_model_file.c_str(), plate_weights_file.c_str());
	if (h_detector == NULL) {
		usages(argv[0]);
		parser.printParams();
		return 1;
	}
	if (video_file.empty()) {
		usages(argv[0]);
		parser.printParams();
		return 1;
	}

	pm::VideoItemData vid;
	vid.setType(pm::VIDEO_TYPE_FILE);
	vid.setVideoName(video_file);

	pm::VideoItem vi;
	bool res = vi.open(vid);

	if (!res) {
		std::cout << "Failed open video file: " << video_file << std::endl;
		return 0;
	}

	bool init = true;


	//cv::VideoWriter out("outcpp.avi", CV_FOURCC('M', 'J', 'P', 'G'), 10, cv::Size(640, 360));
	cv::VideoWriter out("outcpp.avi", CV_FOURCC('M', 'J', 'P', 'G'), 20, cv::Size(width, height));

	while (res) {
		cv::Mat frame;
		bool bres = vi.read(frame);
		if (bres) {
			init = false;
			
			detect(h_detector, frame);

			out.write(frame);
			cv::imshow(video_file, frame);

			int key_pressed = 0xFF & cv::waitKey(30);
			if (key_pressed == 27)
				break;
		}
		else if (!init) {
			break;
		}
	}

	ssd::releaseDetector(h_detector);
	out.release();

	cv::destroyAllWindows();

	return 0;
}

