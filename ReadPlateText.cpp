#include <iostream>
#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#ifdef AVS_TESSERACT
#include <tesseract/baseapi.h>
#endif

#include "ReadPlateText.h"

using namespace std;
using namespace cv;

#ifdef AVS_TESSERACT
tesseract::TessBaseAPI *api;

int AVS_InitTesseract(void) {
     api = new tesseract::TessBaseAPI();

     if (api->Init(NULL, "eng")) {
         printf("Failed to initialise Tesseract\n");
         return(-1);
         }

     api->SetPageSegMode(tesseract::PSM_SINGLE_LINE);
     api->SetVariable("tessedit_char_whitelist", "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890");
     api->SetVariable("tessedit_unrej_any_wd", "1");


     printf("Tesseract - initialised.\n");

     return(0);

     }
#endif // AVS_TESSERACT

int ReadPlateText(Mat &imagePlate, char *szPlateText) {
    char *szOutText;
    Mat imageGray;
    Mat imageThreshold;

#ifndef AVS_TESSERACT
    printf("ReadPlateText: AVS_TESSERACT Not defined, so not doing ReadPlateText()\n");
    return(-1);
#else

    cvtColor(imagePlate, imageGray, CV_BGR2GRAY);
    threshold(imageGray, imageThreshold, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);
#ifdef AVS_QT
    imshow("ThreshPlate", imageThreshold);
#endif // AVS_QT

    api->TesseractRect(imageThreshold.data, 1, imageThreshold.step1(), 0, 0, imageThreshold.size().width, imageThreshold.size().height);
    szOutText = api->GetUTF8Text();
    printf("OCR output: %s", szOutText);
#ifdef AVS_QT
    imwrite("plate-thresh.jpg", imageThreshold);
    //cvWaitKey(0);
#endif // AVS_QT

    strcpy(szPlateText, szOutText);


#endif // End 'else, AVS_TESSERACT is defined, so do the OCR'

    return(0);
    }
