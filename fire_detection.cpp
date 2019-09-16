#include <opencv2\opencv.hpp>
#include <vector>
#include <iostream>
using namespace std;
typedef unsigned char byte;
std::vector<byte> matToBytes(cv::Mat image)
{
	int size = image.total() * image.elemSize();
	std::vector<byte> img_bytes(size);
	img_bytes.assign(image.datastart, image.dataend);
	return img_bytes;
}
int main()
{
	//file = "test.mp4" // 영상 경로입력. 웹캠이용시 주석
	//cv::VideoCapture video(file);

	cv::VideoCapture video(0);// 웹캠일시 주석 해제

	cv::Ptr<cv::BackgroundSubtractor> pMOG2;
	pMOG2 = new cv::BackgroundSubtractorMOG2();
	bool isMask = true;
	// 비디오를 열지 못할 경우 "Can not open video" 출력
	if (!video.isOpened()) {
		std::cout << "Can not open video" << std::endl;
		return 0;
	}

	// 비디오 프레임을 담을 Mat형 변수
	cv::Mat originalFrame;
	cv::Mat resultFrame;

	// "FireVideo"라는 윈도우 창을 생성
	cv::namedWindow("OriginalVideo", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("ResultVideo", cv::WINDOW_AUTOSIZE);

	std::vector<cv::Mat> channels;

	while (1) {

		// 비디오 영상 프레임을 frame 변수에 받기
		video >> originalFrame;

		// 주변 픽셀들의 값을 이용해 평균을 적용하여 잡음 제거함
		cv::medianBlur(originalFrame, resultFrame, 3);

		// 채널 분리. BGR 순서대로 분리된다.
		cv::split(resultFrame, channels);

		cv::Mat3b hsv;
		cv::cvtColor(resultFrame, hsv, cv::COLOR_BGR2HSV);

		cv::Mat1b mask1, mask2;
		inRange(hsv, cv::Scalar(0, 70, 50), cv::Scalar(10, 255, 255), mask1);
		inRange(hsv, cv::Scalar(170, 70, 50), cv::Scalar(180, 255, 255), mask2);

		cv::Mat1b mask = mask1 | mask2;
		// morphological opening 작은 점들을 제거
		cv::erode(mask, mask, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
		cv::dilate(mask, mask, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
		// morphological closing 영역의 구멍 메우기
		cv::dilate(mask, mask, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
		cv::erode(mask, mask, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

		cv::threshold(mask, mask, 200, 255, cv::THRESH_BINARY);   //이진화 동영상일 땐 이게 제일 낫네요

		int size = mask.total() * mask.elemSize();
		std::vector<byte> img_bytes(size);
		img_bytes = matToBytes(mask);
		int sum = 0;
		for (int i = 0; i < img_bytes.size(); i++) {
			sum += img_bytes[i];
		}
		if (sum > 115200)
			cout << "FIRE* ";

		// 윈도우 창에 비디오 영상 프레임을 받는 변수를 이용해 출력
		cv::imshow("OriginalVideo", originalFrame);

		//결과 영상 출력
		cv::imshow("ResultVideo", mask);

		channels.clear();

		// 30 프레임마다 검사. "esc" 키를 눌렀을 시에 break
		if (cv::waitKey(30) == 27) {
			break;
		}
	}

	// 윈도우 창 모두 삭제
	cv::destroyAllWindows();
	// Mat 형 변수 release;
	originalFrame.release();
	resultFrame.release();

	video.release();


	return 0;
}
