#ifndef FINDPLATE_H
#define FINDPLATE_H

using namespace cv;

#include <opencv2/core/core.hpp>

int FindPlates(Mat &imageSource, vector<Rect> &rectsNotRotated);

#endif // FINDPLATE_H
