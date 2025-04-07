/**
******************************************************************************
* @course 	Deep Learning and Image Processing - HGU
* @author	Chanjung Kim / 22000188
* @Created	2025-04-01 by CJKIM
* @Modified	2025-04-08 by CJKIM
* @brief	[DLIP] Assignment for Line Detection
*
******************************************************************************
*/

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <opencv.hpp>
#include <math.h>
#include "../../include/DLIP_LAB.h"

#define RAD2DEG		(double)(	180 / CV_PI	)
#define DEG2RAD		(double)(	CV_PI / 180	)
#define IMG_NUM		(int)	(		2		)

using namespace cv;
using namespace std;


int main() {

	Mat src[IMG_NUM];
	src[0] = imread("../../image/Lane_center.jpg");
	src[1] = imread("../../image/Lane_changing.jpg");
	Mat dst[IMG_NUM], gray[IMG_NUM], ROI_left[IMG_NUM], ROI_right[IMG_NUM];
	int width[IMG_NUM], height[IMG_NUM];
	double m1, m2, c1, c2;

	for (int i = 0; i < IMG_NUM; i++)
	{


		if (!src[i].data)
		{
			printf("Error No image in #%d", i + 1);
			return -1;
		}
		cvtColor(src[i], gray[i], COLOR_BGR2GRAY);

		GaussianBlur(gray[i], dst[i], Size(5, 5), 1, 1);	

		// ROI Setting
		width[i] = cvRound(src[0].cols * 0.25);
		height[i] = cvRound(src[0].rows * 0.5);
		ROI_left[i] = gray[i](Rect(width[i], height[i], width[i], height[i]));
		ROI_right[i] = gray[i](Rect(2 * width[i], height[i], width[i], height[i]));

		// Canny Edge Detection
		Canny(ROI_left[i], ROI_left[i], 50, 200, 3);
		Canny(ROI_right[i], ROI_right[i], 50, 200, 3);

		// Hough
		vector<Vec2f> left, right;
		HoughLines(ROI_left [i], left, 1, CV_PI / 180, 50, 0, 0, 25 * DEG2RAD, 60 * DEG2RAD);
		HoughLines(ROI_right[i], right, 1, CV_PI / 180, 50, 0, 0, 115 * DEG2RAD, 135 * DEG2RAD);

		float avg_theta = 0, avg_rho = 0;
		int count = 0;

		// For Left ROI
		for (size_t k = 0; k < left.size(); k++)
		{
			float rho = left[k][0], theta = left[k][1];
			
			// rho exception
			if (rho > width[i] * 0.73)
			{
				avg_theta = Series_Avg(avg_theta, theta, count);
				avg_rho = Series_Avg(avg_rho, rho, count);

				count++;
			}
		}

		// Point calculation
		double a = cos(avg_theta), b = sin(avg_theta);
		double x0 = a * avg_rho + width[i], y0 = b * avg_rho + height[i];
		double R0 = (dst[i].rows - y0) / a;
		Point pt1_L(x0, y0), pt2_L(x0 - R0 * b, y0 + R0 * a);

		// Slope and y-intercept calculation
		if (abs(pt1_L.x - pt2_L.x) > 1)
			c1 = (pt2_L.y * pt1_L.x - pt1_L.y * pt2_L.x) / (pt1_L.x - pt2_L.x);
		else
			c1 = 0;
		m1 = (pt1_L.y - c1) / pt1_L.x;

		avg_theta = 0, avg_rho = 0, count = 0;
		// For Right ROI
		for (size_t k = 0; k < right.size(); k++)
		{
			float rho = right[k][0], theta = right[k][1];

			// rho exception
			if (rho > 0 && rho < width[i] * 0.2) {
				avg_theta = Series_Avg(avg_theta, theta, count);
				avg_rho = Series_Avg(avg_rho, rho, count);

				count++;
			}
		}


		// Point calculation
		a = cos(avg_theta);
		b = sin(avg_theta);
		x0 = a * avg_rho + 2*width[i];
		y0 = b * avg_rho + height[i];
		R0 = (dst[i].rows - y0) / a;
		Point pt1_R(x0, y0), pt2_R(x0 - R0 * b, y0 + R0 * a);

		// slope and y-intercept calculation
		if (fabs(pt1_R.x - pt2_R.x) > 1)
			c2 = (pt2_R.y * pt1_R.x - pt1_R.y * pt2_R.x) / (pt1_R.x - pt2_R.x);
		else
			c2 = 0;
		m2 = (pt1_R.y - c2) / pt1_R.x;

		// Intersecting point calculation
		Point dst;
		dst.x = (c2 - c1) / (m1 - m2);
		dst.y = m1 * dst.x + c1;

		// Draw lines
		cv::line(src[i], dst, pt2_L, Scalar(0, 0, 255), 3, LINE_AA);
		cv::line(src[i], dst, pt2_R, Scalar(0, 255, 0), 3, LINE_AA);

		// Draw circle 
		circle(src[i], dst, 10, Scalar(0, 100, 255), 3, -1);
	}

	cv::imshow("center", src[0]);
	cv::imshow("changing", src[1]);

	cv::waitKey();
}
