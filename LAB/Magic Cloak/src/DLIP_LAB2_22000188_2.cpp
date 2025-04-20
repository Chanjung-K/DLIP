/**
******************************************************************************
* @course 	Deep Learning and Image Processing - HGU
* @author	Chanjung Kim / 22000188
* @Created	2025-04-16 by CJKIM
* @Modified	2025-04-18 by CJKIM
* @brief	[DLIP] LAB for Magic Cloak Sample #2
******************************************************************************
*/

#include <iostream>
#include <opencv.hpp>
#include "../../include/DLIP_LAB.h"

using namespace cv;
using namespace std;

int main() {
	// Reference and opening image load
	Mat img_src = imread("sample#2.png");
	Mat Opening = imread("Opening.png");
	Mat hsv;
	cvtColor(img_src, hsv, COLOR_BGR2HSV);
	Scalar means, stddev;
	meanStdDev(hsv, means, stddev);

	// Video Parameter
	VideoCapture Video = VideoCapture("Sample2.mp4");
	VideoWriter Output;
	Mat back, src, temp, dst;

	// Parameter Tuning
	int delta = 20;
	stddev[0] += 0.4 * delta;
	stddev[1] += 2 * delta;

	// setting min & max (hsv)
	Scalar Min = Scalar(means[0] - 2 * stddev[0], means[1] - 2 * stddev[1], 70);
	Scalar Max = Scalar(means[0] + 2 * stddev[0], MIN(means[1] + 2 * stddev[1], 255), 255);
	printf("Selected Min value are \nHue:%.4f\tSaturation:%.4f\tValue:%.4f\n", Min[0], Min[1], Min[2]);
	printf("Selected Max value are \nHue:%.4f\tSaturation:%.4f\tValue:%.4f\n", Max[0], Max[1], Max[2]);

	// Background image read
	Video.read(back);

	// Morphology Kernel initiation
	int size_op = 11, size_cl = 9, size_di = 5;
	Mat kernel_open = GetKernel(MORPH_RECT, size_op);
	Mat kernel_close = GetKernel(MORPH_RECT, size_cl);
	Mat kernel_dilate = GetKernel(MORPH_RECT, size_di);
	
	// Open a file to write a video
	Output.open("Output#2.mp4", VideoWriter::fourcc('m', 'p', '4', 'v'), 30.0, back.size(), true);

	// Opening Image
	for (int i = 0; i < 30*5; i ++)
		Output.write(Opening);

	// Main Loop
	while (Video.isOpened()) {
		Video.read(temp);
		temp.copyTo(src);

		if (temp.empty())
			break;
		// Color convertion BGR --> HSV
		cvtColor(temp, temp, COLOR_BGR2HSV);
		imshow("TEMP", temp);

		// HSV --> Gray Scale
		inRange(temp, Min, Max, temp);

		// Opening morphology to eliminate noises
		erode(temp, temp, kernel_open, Point(-1, -1), 2);
		dilate(temp, temp, kernel_open, Point(-1, -1), 2);

		// Closing Morphology to fill out small pores
		if (!temp.empty())
		{
			morphologyEx(temp, temp, MORPH_CLOSE, kernel_close, Point(-1, -1));
			dilate(temp, temp, kernel_dilate, Point(-1, -1), 1);
		}

		// Gray scale (1 Channel) --> BGR (3 Channel)
		cvtColor(temp, temp, COLOR_GRAY2BGR);

		// Bitwise Operation to provide final output
		dst = back & temp;
		bitwise_not(temp, temp);
		dst = (src & temp) + dst;

		// Show and write
		imshow("result", dst);
		Output.write(dst);

		// Stop key
		if (waitKey(20) == 27)	break;
	}

	// Release memory
	img_src.release();
	hsv.release();
	Video.release();
	Output.release();
	back.release();
	src.release();
	temp.release();
	dst.release();
	
	waitKey();

	return 0;
}