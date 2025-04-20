#include "DLIP_LAB.h"

Point2f Find_CenterPoint(Mat image) {
	Moments M = moments(image);
	Point2f center(int(M.m10 / M.m00), int(M.m01 / M.m00));

	return center;
}

Point Find_CenterPoint(vector<Point> contour) {
	Moments M = moments(contour);
	Point center(int(M.m10 / M.m00), int(M.m01 / M.m00));

	return center;
}

double Median_Avg(Mat data, int StartIdx, int EndIdx, int RowNum)
{
	double avg = 0;
	for (int i = StartIdx; i < EndIdx; i++) {
		avg = avg + (data.at<float>(i, RowNum) - avg) / (i - StartIdx + 1);
	}
	return avg;
}

double Series_Avg(double avg, double data, int iter)
{
	avg = avg + (data - avg) / (iter + 1);
	return avg;
}

void plotHist(Mat src, string plotname, int width, int height) {
	/// Compute the histograms 
	Mat hist;
	/// Establish the number of bins (for uchar Mat type)
	int histSize = 256;
	/// Set the ranges (for uchar Mat type)
	float range[] = { 0, 256 };

	const float* histRange = { range };
	calcHist(&src, 1, 0, Mat(), hist, 1, &histSize, &histRange);

	double min_val, max_val;
	cv::minMaxLoc(hist, &min_val, &max_val);
	Mat hist_normed = hist * height / max_val;
	float bin_w = (float)width / histSize;
	Mat histImage(height, width, CV_8UC1, Scalar(0));
	for (int i = 0; i < histSize - 1; i++) {
		line(histImage,
			Point((int)(bin_w * i), height - cvRound(hist_normed.at<float>(i, 0))),
			Point((int)(bin_w * (i + 1)), height - cvRound(hist_normed.at<float>(i + 1, 0))),
			Scalar(255), 2, 8, 0);
	}

	imshow(plotname, histImage);
}