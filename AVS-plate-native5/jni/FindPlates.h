#ifndef FINDPLATE_H
#define FINDPLATE_H

using namespace cv;

#include <opencv2/core/core.hpp>

int FindPlates(Mat &imageSource, vector<Rect> &rectsNotRotated, vector<RotatedRect> &rectsRotated, Mat &imageDebug, int iDebugMode);
void FindLargestPlate(vector<RotatedRect> &rectsRotated, RotatedRect &rotatedrectLargestPlate);
void FindBestPlateCandidate(vector<RotatedRect> &rectsRotated, RotatedRect &rotatedrectBestPlate, int iImageWidth, int iImageHeight);
Mat DerotateAndCorrectPerspective(Mat &imageSource, RotatedRect &rectRotated);

#endif // FINDPLATE_H
