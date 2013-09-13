#ifndef READPLATETEXT_H
#define READPLATETEXT_H

#include <opencv2/core/core.hpp>

using namespace cv;

int AVS_InitTesseract(void);
int ReadPlateText(Mat &imagePlateLoc, char *szPlateText);

#endif // READPLATETEXT_H
