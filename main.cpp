#include <iostream>
#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "ANPRMain.h"
#include "FindPlates.h"
#include "ReadPlateText.h"

using namespace std;
using namespace cv;

bool verifySizes(RotatedRect rectCandidatePlate);

int main()
{
    int iKey;
    Mat imageSource;
    Mat imageDebug;                     // Candidate rectangles
    int iNumPlates;
    char szPlateText[10000];            // TODO - make smaller and pass max size as parameter

#ifdef AVS_TESSERACT
    AVS_InitTesseract();
#endif

    VideoCapture cap(0);

    if (!cap.isOpened()) {
        printf("Failed to open webcam\n");
        cvWaitKey(0);
        return(-1);
        }

    cap.read(imageSource);

    while(1) {

        cap.read(imageSource);
        imageSource.copyTo(imageDebug);

        imshow("Source", imageSource);

        cvWaitKey(20);

        iNumPlates = ANPRMain(imageSource,
                              szPlateText,
                              imageDebug,
                              1);       // 1 == All debug windows separately (for Win/Linux)
        if (iNumPlates >= 1) {
            printf("Found %d plates", iNumPlates);
            printf("Returned plates string :%s:\n", szPlateText);
            }
        if (iNumPlates == -1) {  // User pressed ESC during processing
            return(-1);
            }

//#ifdef OBSOLETE
        //continue;

        } // End 'while each frame'

    return 0;
}

