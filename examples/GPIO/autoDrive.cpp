#include <iostream>
#include <stdlib.h>
#include <vector>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>


#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include "GPIOlib.h"

#define PI 3.1415926

//Uncomment this line at run-time to skip GUI rendering
//#define _DEBUG

using namespace cv;
using namespace std;
using namespace GPIO;


void on_exit(void)
{
	stopLeft();
	stopRight();
}
void signal_crash_handler(int sig)
{
	on_exit();
	exit(-1);
}
void signal_exit_handler(int sig)
{
	exit(0);
}

const string CAM_PATH = "/dev/video0";
const string MAIN_WINDOW_NAME = "Processed Image";
const string CANNY_WINDOW_NAME = "Canny";
const string THRESHOLD_WINDOW_NAME = "Threshold";
const string DILATE_WINDOW_NAME = "Dilate";
const string ERODE_WINDOW_NAME = "Erode";
const string ROTATED_WINDOW_NAME = "Rotated";

const int CANNY_LOWER_BOUND = 50;
const int CANNY_UPPER_BOUND = 250;
const int HOUGH_THRESHOLD = 100;
const int K = 10;

struct test_line
{
	float rho;
	float angle;
};

test_line test_lines[20];
test_line result_lines[4];
void lineCluster();
int main()
{

	init();

	atexit(on_exit);
	signal(SIGTERM, signal_exit_handler);
	signal(SIGINT, signal_exit_handler);

	// ignore SIGPIPE
	signal(SIGPIPE, SIG_IGN);

	signal(SIGBUS, signal_crash_handler);  // 总线错误
	signal(SIGSEGV, signal_crash_handler); // SIGSEGV，非法内存访问
	signal(SIGFPE, signal_crash_handler);  // SIGFPE，数学相关的异常，如被0除，浮点溢出，等等
	signal(SIGABRT, signal_crash_handler);

	int speed = 10;
	int abs_speed = 10;
	int direction = FORWARD;
	int angle = 0;
	turnTo(angle);
	controlLeft(FORWARD, abs_speed);
	controlRight(FORWARD, abs_speed);

	VideoCapture capture(CAM_PATH);
	//If this fails, try to open as a video camera, through the use of an integer param
	if (!capture.isOpened())
	{
		capture.open(atoi(CAM_PATH.c_str()));
	}

	double dWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH);   //the width of frames of the video
	double dHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT); //the height of frames of the video
	clog << "Frame Size: " << dWidth << "x" << dHeight << endl;

	Mat image;
	while (true)
	{
		capture >> image;
		if (image.empty())
			break;

		//Set the ROI for the image
		Rect roi(0, image.rows / 3, image.cols, image.rows / 3);
		Mat imgROI = image(roi);
		cvtColor(imgROI, imgROI, COLOR_BGR2GRAY);

		//二值化
		Mat result_thre = imgROI;
		//threshold(imgROI, result_thre,150, 255, CV_THRESH_BINARY);
#ifdef _DEBUG
		imshow(THRESHOLD_WINDOW_NAME, result_thre);
#endif

		//膨胀
		Mat result_dilate;
		Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
		dilate(result_thre, result_dilate, element);
#ifdef _DEBUG
		//imshow(DILATE_WINDOW_NAME, result_dilate);
#endif

		//腐蚀
		Mat result_erode;
		erode(result_dilate, result_erode, element);
#ifdef _DEBUG
		//imshow(ERODE_WINDOW_NAME, result_erode);
#endif

		//Canny algorithm（边缘检测）
		Mat contours;
		Canny(result_erode, contours, CANNY_LOWER_BOUND, CANNY_UPPER_BOUND);
#ifdef _DEBUG
		//imshow(CANNY_WINDOW_NAME, contours);
#endif

		vector<Vec2f> lines;
		HoughLines(contours, lines, 1, PI / 180, HOUGH_THRESHOLD);
		Mat result1(imgROI.size(), CV_8U, Scalar(255));
		//imgROI.copyTo(result);

		float maxRad = -2 * PI;
		float minRad = 2 * PI;

		//myself
		int lines_num = 0;

		//Draw the lines and judge the slope
		for (vector<Vec2f>::const_iterator it = lines.begin(); it != lines.end(); ++it)
		{
			float rho = (*it)[0];   //First element is distance rho
			float theta = (*it)[1]; //Second element is angle theta

			//myself
			test_lines[lines_num].rho = rho;
			test_lines[lines_num].angle = theta * 180 / CV_PI;
			lines_num++;

			//Filter to remove vertical and horizontal lines,
			//and atan(0.09) equals about 5 degrees.
			if ((theta > 0.09 && theta < 1.48) || (theta > 1.62 && theta < 3.05))
			{
				if (theta > maxRad)
					maxRad = theta;
				if (theta < minRad)
					minRad = theta;
			}
		}
		//myself
		test_lines[lines_num].rho = 0;
		test_lines[lines_num].angle = 404;
		lineCluster();
		/*clog << "OUT1:";
		for (int i = 0; i < 20 && test_lines[i].angle != 404;i++) {
			clog << "Line: (" << test_lines[i].rho << "," << test_lines[i].angle << ")\n";
		}
		clog << "OUT2:";*/
		for (int i = 0; i < 4 && result_lines[i].angle != 404; i++)
		{
			//clog << "Line: (" << result_lines[i].rho << "," << result_lines[i].angle << ")\n";
			float theta = result_lines[i].angle * CV_PI / 180;
			float rho = result_lines[i].rho;
			//point of intersection of the line with first row
			Point pt1(rho / cos(theta), 0);
			//point of intersection of the line with last row
			Point pt2((rho - result1.rows * sin(theta)) / cos(theta), result1.rows);
			//Draw a line
			line(result1, pt1, pt2, Scalar(0, 255, 255), 3, CV_AA);
		}

		//透视变换
		vector<Point> not_a_rect_shape;
		not_a_rect_shape.push_back(Point(image.cols * 0.4, 0));
		not_a_rect_shape.push_back(Point(image.cols * 0.6, 0));
		not_a_rect_shape.push_back(Point(0, image.rows / 3));
		not_a_rect_shape.push_back(Point(image.cols, image.rows / 3));
		Point2f src_vertices[4];
		src_vertices[0] = not_a_rect_shape[0];
		src_vertices[1] = not_a_rect_shape[1];
		src_vertices[2] = not_a_rect_shape[2];
		src_vertices[3] = not_a_rect_shape[3];

		Point2f dst_vertices[4];
		dst_vertices[0] = Point(0, 0);
		dst_vertices[1] = Point(image.cols, 0);
		dst_vertices[2] = Point(0, image.rows / 3);
		dst_vertices[3] = Point(image.cols, image.rows / 3);
		Mat warpMatrix = getPerspectiveTransform(src_vertices, dst_vertices);
		Mat result_rotated;
		warpPerspective(result1, result_rotated, warpMatrix, result_rotated.size(), INTER_LINEAR, BORDER_CONSTANT);
#ifdef _DEBUG
		imshow(ROTATED_WINDOW_NAME, result_rotated);
#endif

		//Canny algorithm（边缘检测）
		Canny(result_rotated, contours, CANNY_LOWER_BOUND, CANNY_UPPER_BOUND);

		HoughLines(contours, lines, 1, PI / 180, HOUGH_THRESHOLD);
		Mat result2(imgROI.size(), CV_8U, Scalar(255));
		lines_num = 0;
		//Draw the lines and judge the slope
		for (vector<Vec2f>::const_iterator it = lines.begin(); it != lines.end(); ++it)
		{
			float rho = (*it)[0];   //First element is distance rho
			float theta = (*it)[1]; //Second element is angle theta

			//myself
			test_lines[lines_num].rho = rho;
			test_lines[lines_num].angle = theta * 180 / CV_PI;
			lines_num++;

			//Filter to remove vertical and horizontal lines,
			//and atan(0.09) equals about 5 degrees.
			if ((theta > 0.09 && theta < 1.48) || (theta > 1.62 && theta < 3.05))
			{
				if (theta > maxRad)
					maxRad = theta;
				if (theta < minRad)
					minRad = theta;
			}
		}
		//myself
		test_lines[lines_num].rho = 0;
		test_lines[lines_num].angle = 404;
		lineCluster();
		/*clog << "OUT1:";
		for (int i = 0; i < 20 && test_lines[i].angle != 404; i++) {
			clog << "Line: (" << test_lines[i].rho << "," << test_lines[i].angle << ")\n";
		}
		clog << "OUT2:";*/
		for (int i = 0; i < 4 && result_lines[i].angle != 404; i++)
		{
			//clog << "Line: (" << result_lines[i].rho << "," << result_lines[i].angle << ")\n";
			float theta = result_lines[i].angle * CV_PI / 180;
			float rho = result_lines[i].rho;
			//point of intersection of the line with first row
			Point pt1(rho / cos(theta), 0);
			//point of intersection of the line with last row
			Point pt2((rho - result1.rows * sin(theta)) / cos(theta), result1.rows);
			//Draw a line
			line(result2, pt1, pt2, Scalar(0, 255, 255), 3, CV_AA);
		}

		float result_angle;
		result_angle = 404;
		if (result_lines[0].angle != 404)
		{
			if (result_lines[0].angle < 90)
			{
				result_angle = result_lines[0].angle;
			}
			else if (result_lines[0].angle > 90)
			{
				result_angle = result_lines[0].angle - 180;
			}
		}
		else
		{
			result_angle = 0;
		}
		clog << "Angle:" << result_angle << "\n";
		if(result_angle>-90&&result_angle<90){
			angle=-result_angle;
			turnTo(angle);
		}

#ifdef _DEBUG
		stringstream overlayedText;
		//overlayedText << "Lines: " << i;
		putText(result1, overlayedText.str(), Point(10, result1.rows - 10), 2, 0.8, Scalar(0, 0, 255), 0);
		imshow(MAIN_WINDOW_NAME, result1);
		imshow("result2", result2);

#endif

		lines.clear();
		waitKey(1);
	}
	return 0;
}

void lineCluster()
{
	int tempNum[3] = {0, 0, 0};
	int result_lines_num = 0;
	for (int i = 0; test_lines[i].rho != -1 && result_lines_num < 3; i++)
	{
		int inArray = 0;
		if (result_lines_num == 0)
		{
			result_lines[0] = test_lines[0];
			tempNum[0]++;
			result_lines_num++;
			continue;
		}
		for (int j = 0; j < result_lines_num; j++)
		{
			float angleDiff = (test_lines[i].angle - result_lines[j].angle) * (test_lines[i].angle - result_lines[j].angle);
			float rhoDiff = (test_lines[i].rho - result_lines[j].rho) * (test_lines[i].rho - result_lines[j].rho);
			if (angleDiff < 150 && rhoDiff < 150)
			{
				result_lines[j].angle = (result_lines[j].angle * tempNum[j] + test_lines[i].angle) / (tempNum[j] + 1);
				result_lines[j].rho = (result_lines[j].rho * tempNum[j] + test_lines[i].rho) / (tempNum[j] + 1);
				tempNum[j]++;
				inArray = 1;
				break;
			}
		}
		//clog << "inArray:"<<inArray<<"\n";
		if (inArray == 0)
		{
			result_lines[result_lines_num] = test_lines[i];
			tempNum[result_lines_num] = 1;
			result_lines_num++;
		}
	}
	result_lines[result_lines_num].rho = 0;
	result_lines[result_lines_num].angle = 404;

	/*clog << "IN:\n";
	clog << "num:"<< result_lines_num<<"\n";
	for (int i = 0; i<result_lines_num;i++) {
		clog << "Line: (" << result_lines[i].rho << "," << result_lines[i].angle << ")\n";
	}*/
}
