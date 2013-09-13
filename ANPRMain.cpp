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
double LineLength(cv::Point *pt1, cv::Point *pt2);

///////////////////////////////////////////////////////////////////////
// ANPRMain()
// Top-level function encapsulating the whole process, so that on
// Android, as little as possible has to be done in Java.
//
// Returns:
//    Number of plates found, or -1 to exit program (e.g. ESC pressed
//    by user).
///////////////////////////////////////////////////////////////////////

int ANPRMain(Mat &imageSource, char *szPlateText, Mat &imageDebug, int iDebugMode) {

    vector< vector< Point> > contours;
    vector<Rect> rectsNotRotated;
    vector<RotatedRect> rectsRotated;
    vector<Point> thisSquare;
    RotatedRect rotatedrectLargestPlate;
    Mat imagePlateLoc;
    int rc;
    char szUniquePart[100];
    char szDebugImageFilename[100];
    int iKey;

    Mat element = getStructuringElement(MORPH_RECT, Size(25,3));

    rectsNotRotated.clear();
    rectsRotated.clear();

    FindPlates(imageSource, rectsNotRotated, rectsRotated);

    // Draw all the rectangles returned

    printf("There are %d rects\n", rectsNotRotated.size());
    printf("There are %d rotated rects\n", rectsRotated.size());

    if (rectsNotRotated.size() < 1) {
        iKey = cvWaitKey(1);
        if (iKey == 27) {
            return(-1);                 // -1 == tell caller to exit
            }
        return(0);                      // 0 == no plates found
        }

    for (vector<Rect>::size_type i=0; i != rectsNotRotated.size(); i++) {
        rectangle(imageDebug, rectsNotRotated[i], CV_RGB(255,0,0), 3);
        }

    for (vector<RotatedRect>::size_type i=0; i != rectsRotated.size(); i++) {
        // Draw the points at the corners of this 'square'
        RotatedRect thisRotatedRect = rectsRotated[i];
        Point2f points[4];

        thisRotatedRect.points(points);

        for (int iPoint=0; iPoint<4; iPoint++) {
            line(imageDebug, points[iPoint], points[(iPoint+1) % 4], Scalar(0,255,0,1), 2);
            circle(imageDebug, points[iPoint], 3, CV_RGB(0,0,255), -1);
             }

        }

    FindLargestPlate(rectsRotated, rotatedrectLargestPlate);
    Point2f points[4];
    rotatedrectLargestPlate.points(points);

    for (int iPoint=0; iPoint<4; iPoint++) {
        //line(imageSource, points[iPoint], points[(iPoint+1) % 4], CV_RGB(0,255,255), 2);
        //circle(imageSource, points[iPoint], 3, CV_RGB(0,0,255), -1);
        }

    imshow("Debug", imageDebug);

    imagePlateLoc = DerotateAndCorrectPerspective(imageSource, rotatedrectLargestPlate);

#ifdef AVS_TESSERACT
    printf("About to call ReadPlateText\n");


    rc = ReadPlateText(imagePlateLoc, szPlateText);

    strcpy(szUniquePart, "plateXXXXXX");
    mktemp(szUniquePart);
    sprintf(szDebugImageFilename, "%s.jpg", szUniquePart);

    //imwrite(szDebugImageFilename, imagePlateLoc);
#endif
    printf("After ReadPlateText\n");
    imshow("Derotated", imagePlateLoc);

    iKey = cvWaitKey(1);
    if (iKey == 27) {
        return(-1);                     // -1 == tell caller to exit
        }
//#endif // OBSOLETE

    return(rectsRotated.size());
    }
