#ifndef ANPRMAIN_H
#define ANPRMAIN_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

int ANPRMain(Mat &imageSource, Mat &imagePlate, int iDebugMode);

#endif // ANPRMAIN_H
