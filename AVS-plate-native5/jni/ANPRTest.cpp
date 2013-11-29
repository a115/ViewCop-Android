#include <iostream>
#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "ANPRTest.h"

using namespace std;
using namespace cv;


int ANPRTest(Mat &imageSource) {

    circle(imageSource, Point(50,50), 30, CV_RGB(0,0,255), -1);

    return(0);
    }
