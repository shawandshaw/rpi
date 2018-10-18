#include <cstdlib>
#include <iostream>
#include <vector>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#define PI 3.1415926

//Uncomment this line at run-time to skip GUI rendering
#define _DEBUG

using namespace cv;
using namespace std;

const string CAM_PATH = "/dev/video0";
const string MAIN_WINDOW_NAME = "Processed Image";
const string CANNY_WINDOW_NAME = "Canny";
const string THRESHOLD_WINDOW_NAME = "Threshold";
const string DILATE_WINDOW_NAME = "Dilate";
const string ERODE_WINDOW_NAME = "Erode";
const string ROTATED_WINDOW_NAME = "Rotate";

const int CANNY_LOWER_BOUND = 50;
const int CANNY_UPPER_BOUND = 250;
const int HOUGH_THRESHOLD = 150;

IplImage* src = 0;

void on_mouse(int event, int x, int y, int flags, void* ustc);
int main()
{
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

		IplImage imgTmp = imgROI;
		src = cvCloneImage(&imgTmp);
		cvNamedWindow("src", 1);
		cvSetMouseCallback("src", on_mouse, 0);

		cvShowImage("src", src);

		//二值化
		Mat result_thre;
		threshold(imgROI, result_thre, 150, 255, CV_THRESH_BINARY);
#ifdef _DEBUG
		imshow(THRESHOLD_WINDOW_NAME, result_thre);
#endif
		//膨胀
		Mat result_dilate;
		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
		dilate(result_thre, result_dilate, element);
#ifdef _DEBUG
		imshow(DILATE_WINDOW_NAME, result_dilate);
#endif

		//腐蚀
		Mat result_erode;
		erode(result_dilate, result_erode, element);
#ifdef _DEBUG
		imshow(ERODE_WINDOW_NAME, result_erode);
#endif

		//透视变换
		vector<Point> not_a_rect_shape;
		not_a_rect_shape.push_back(Point(122, 0));
		not_a_rect_shape.push_back(Point(814, 0));
		not_a_rect_shape.push_back(Point(22, 540));
		not_a_rect_shape.push_back(Point(910, 540));
		Point2f src_vertices[4];
		src_vertices[0] = not_a_rect_shape[0];
		src_vertices[1] = not_a_rect_shape[1];
		src_vertices[2] = not_a_rect_shape[2];
		src_vertices[3] = not_a_rect_shape[3];

		Point2f dst_vertices[4];
		dst_vertices[0] = Point(0, 0);
		dst_vertices[1] = Point(960, 0);
		dst_vertices[2] = Point(0, 540);
		dst_vertices[3] = Point(960, 540);
		Mat warpMatrix = getPerspectiveTransform(src_vertices, dst_vertices);
		Mat result_rotated;
		warpPerspective(result_erode, result_rotated, warpMatrix, result_rotated.size(), INTER_LINEAR, BORDER_CONSTANT);
#ifdef _DEBUG
		imshow(ROTATED_WINDOW_NAME, result_rotated);
#endif

		//Canny algorithm（边缘检测）
		Mat contours;
		Canny(result_erode, contours, CANNY_LOWER_BOUND, CANNY_UPPER_BOUND);
#ifdef _DEBUG
		imshow(CANNY_WINDOW_NAME, contours);
#endif

		vector<Vec2f> lines;
		HoughLines(contours, lines, 1, PI / 180, HOUGH_THRESHOLD);
		Mat result(imgROI.size(), CV_8U, Scalar(255));
		imgROI.copyTo(result);
		clog << lines.size() << endl;

		float maxRad = -2 * PI;
		float minRad = 2 * PI;
		//Draw the lines and judge the slope
		for (vector<Vec2f>::const_iterator it = lines.begin(); it != lines.end(); ++it)
		{
			float rho = (*it)[0];   //First element is distance rho
			float theta = (*it)[1]; //Second element is angle theta

			//Filter to remove vertical and horizontal lines,
			//and atan(0.09) equals about 5 degrees.
			if ((theta > 0.09 && theta < 1.48) || (theta > 1.62 && theta < 3.05))
			{
				if (theta > maxRad)
					maxRad = theta;
				if (theta < minRad)
					minRad = theta;

#ifdef _DEBUG
				//point of intersection of the line with first row
				Point pt1(rho / cos(theta), 0);
				//point of intersection of the line with last row
				Point pt2((rho - result.rows * sin(theta)) / cos(theta), result.rows);
				//Draw a line
				line(result, pt1, pt2, Scalar(0, 255, 255), 3, CV_AA);
#endif
			}
#ifdef _DEBUG
			clog << "Line: (" << rho << "," << theta << ")\n";
#endif
		}

#ifdef _DEBUG
		stringstream overlayedText;
		overlayedText << "Lines: " << lines.size();
		putText(result, overlayedText.str(), Point(10, result.rows - 10), 2, 0.8, Scalar(0, 0, 255), 0);
		imshow(MAIN_WINDOW_NAME, result);
#endif
		lines.clear();
		waitKey(1);
	}
	return 0;
}

void on_mouse(int event, int x, int y, int flags, void* ustc)
{
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);

	if (event == CV_EVENT_LBUTTONDOWN)
	{
		CvPoint pt = cvPoint(x, y);
		char temp[16];
		sprintf_s(temp, "(%d,%d)", pt.x, pt.y);
		cvPutText(src, temp, pt, &font, cvScalar(255, 255, 255, 0));
		cvCircle(src, pt, 2, cvScalar(255, 0, 0, 0), CV_FILLED, CV_AA, 0);
		cvShowImage("src", src);
	}
}