#include <iostream>
#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

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
    vector< vector< Point> > contours;
    vector<Rect> rectsNotRotated;
    vector<RotatedRect> rectsRotated;
    vector<Point> thisSquare;
    RotatedRect rotatedrectLargestPlate;
    Mat imagePlateLoc;
    int rc;
    char szPlateText[10000];
    char szUniquePart[100];
    char szDebugImageFilename[100];

    Mat element = getStructuringElement(MORPH_RECT, Size(25,3));

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



//#ifdef OBSOLETE
        //continue;

        rectsNotRotated.clear();
        rectsRotated.clear();

        FindPlates(imageSource, rectsNotRotated, rectsRotated);

        // Draw all the rectangles returned

        printf("There are %d rects\n", rectsNotRotated.size());
        printf("There are %d rotated rects\n", rectsRotated.size());

        if (rectsNotRotated.size() < 1) {
            iKey = cvWaitKey(1);
            if (iKey == 27) {
                break;
                }
            continue;
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
                line(imageDebug, points[iPoint], points[(iPoint+1) % 4], CV_RGB(0,255,0), 2);
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
            break;
            }
//#endif // OBSOLETE
        } // End 'while each frame'

    return 0;
}

