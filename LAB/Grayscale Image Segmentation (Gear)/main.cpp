#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <opencv.hpp>

using namespace std;
using namespace cv;

#define GEAR_NUM		(int) 4
#define GEAR_TEETH		(int) 20

void main() {
	// Variables used in image loading and noise elimination
	Mat src[4];
	Mat processed_src[4];
	char WindowName[GEAR_NUM][6] = { "Gear1", "Gear2", "Gear3", "Gear4" };
	char SrcWin[GEAR_NUM][10] = { "src1", "src2", "src3", "src4" };
	char fileName[40];
	Mat Kernel = getStructuringElement(MORPH_RECT, Size(5, 5));

	// Image load
	for (int k = 0; k < GEAR_NUM; k++) {
		sprintf(fileName, "../../image/%s.jpg", WindowName[k]);
		src[k] = imread(fileName, IMREAD_GRAYSCALE);

		// Eliminate Noises
		morphologyEx(src[k], src[k], MORPH_CLOSE, Kernel, Point(-1, -1));
		morphologyEx(src[k], src[k], MORPH_OPEN, Kernel, Point(-1, -1));
	}

	// Variables used in Pre-Processing
	vector<vector<Point>> contours_teeth[GEAR_NUM];
	Mat drawing_teeth[GEAR_NUM]; 
	int radius[GEAR_NUM] = {168, 168, 188, 190};
	Point center[4];

	// Gear Teeth Extraction (Pre-Processing)
	for (int i = 0; i < GEAR_NUM; i++) {
		drawing_teeth[i] = Mat(src[0].size(), CV_8U, Scalar(0));
		src[i].copyTo(processed_src[i]);
		findContours(processed_src[i], contours_teeth[i], RETR_CCOMP, CHAIN_APPROX_SIMPLE, Point(0, 0));

		// Draw Center Circle
		drawContours(drawing_teeth[i], contours_teeth[i], 1, Scalar(255), FILLED);

		// Find origin of the gear
		Moments M = moments(drawing_teeth[i]);
		center[i].x = int(M.m10 / M.m00) - (i == 0) - 4 * (i > 1);
		center[i].y = int(M.m01 / M.m00) - (i == 2) - 2 * (i == 3);

		// Set the radius
		circle(drawing_teeth[i], center[i], radius[i], Scalar(255), -1);

		// Subtract circles from src images
		processed_src[i] = processed_src[i] - drawing_teeth[i];

		// Minimize noise and detect edge
		morphologyEx(processed_src[i], processed_src[i], MORPH_OPEN, Kernel, Point(-1, -1));
		Canny(processed_src[i], processed_src[i], 100, 150, 3);

		// uncomment to see pre-processed images
		// imshow(SrcWin[i], src[i]);
	}

	// Variables used in Gear teeth contour 
	vector<vector<Point>> contours[GEAR_NUM];
	vector<cv::Vec4i> hier[GEAR_NUM];
	Mat drawing[GEAR_NUM];
	Mat Teeth_Data(Size(GEAR_NUM, GEAR_TEETH), CV_32FC1, Scalar(0));
	double avg[GEAR_NUM] = { 0.0 };
	double tol = 100;
	int AvgStart = GEAR_TEETH * 0.25, AvgEnd = GEAR_TEETH - AvgStart;
	int DefectiveCount = 0;
	bool flag = true;
	// colors & points
	Scalar red(0, 0, 255);
	Scalar green(0, 255, 0);
	Point temp;
	int Sub = 20;
	
	// Gear Teeth Recognition
	for (int k = 0; k < GEAR_NUM; k++) {
		// Create a drawing images
		drawing[k] = Mat(processed_src[k].size(), CV_32FC3, Scalar(0, 0, 0));
		DefectiveCount = 0;
		flag = true;
		// find contours
		findContours(processed_src[k], contours[k], hier[k], RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

		// Store Area datas to Teeth_Data
		for (int i = 0; i < contours[k].size(); i++) {
			Teeth_Data.at<float>(i, k) = contourArea(contours[k][i]);
		}

		// Sort to avoid abnormal
		Mat temp = Teeth_Data(Rect(k, 0, 1, GEAR_TEETH));
		cv::sort(temp, temp, SORT_EVERY_COLUMN);

		// Calculating average to detect normal area
		for (int i = AvgStart; i < AvgEnd; i++) {
			// avg_k = avg_k-1 + (newdata - avg_k-1) / k
			avg[k] = avg[k] + (Teeth_Data.at<float>(i, k) - avg[k]) / (i - AvgStart + 1);
		}

		// Draw contours and text into drawing image
		for (int i = 0; i < contours[k].size(); i++) {
			int area = (int)contourArea(contours[k][i]);

			// Find each Teeth's center point
			Moments M = moments(contours[k][i]);
			double teeth_x = M.m10 / M.m00;
			double teeth_y = M.m01 / M.m00;
			// Find angle formed by teeth and gear center
			double theta = atan2(teeth_y - center[k].y, teeth_x - center[k].x);
			// Determine text point
			Point text(center[k].x + (radius[k] - Sub) * cos(theta) - 20, center[k].y + (radius[k] - Sub) * sin(theta) + 5 + 10 * sin(theta));

			// Normal Teeth
			if (fabs(avg[k] - area) < tol)
			{
				drawContours(drawing[k], contours[k], i, green);
				putText(drawing[k], to_string(area), text, FONT_HERSHEY_PLAIN, 1, green);
			}
			// Abnormal Teeth
			else
			{
				drawContours(drawing[k], contours[k], i, red);
				putText(drawing[k], to_string(area), text, FONT_HERSHEY_PLAIN, 1, red);
				circle(src[k], Point(teeth_x, teeth_y), 30, Scalar(135), 3, LINE_8);
				DefectiveCount++;
				flag = false;
			}
		}

		// Show original images
		imshow(SrcWin[k], src[k]);

		// Show & print the results
		imshow(WindowName[k], drawing[k]);
		cout << "Teeth numbers: " << contours[k].size() << "\tAvg. Teeth Area: " << avg[k] << "   \tDefective Teeth: " << DefectiveCount << "\tQuality: " << boolalpha << flag << "\n";

	}

	// Wait to end
	waitKey();
}
