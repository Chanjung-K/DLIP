#ifndef _DLIP_LAB_H		// same as "#if !define _TU_DLIP_H" (or #pragma once) 
#define _DLIP_LAB_H

#include <iostream>
#include <opencv.hpp>

using namespace std;
using namespace cv;

#define		GetKernel(Shape, size)	getStructuringElement(Shape, Size(size, size));

/**
* @brief: find a center point of image using moments
* @param image: image to refer
* @return: a center point as Point2f data type
*/
Point2f Find_CenterPoint(Mat image);

/**
* @brief: find a center point of contour using moments
* @param contour: contour to refer
* @return: a center point as Point data type
*/
Point Find_CenterPoint(vector<Point> contour);

/**
* @brief: Calculates average excluding abnormal values
* @param data: Dataset arranged in column
* @param StartIdx: Index to start
* @param EndIdx: Index to end
* @param RowNum: # of row to refer
* @return: a center point as Point data type
*/
double Median_Avg(Mat data, int StartIdx, int EndIdx, int RowNum);

double Series_Avg(double avg, double data, int iter);

void plotHist(Mat src, string plotname, int width, int height);



#endif // !_DLIP_LAB_H
