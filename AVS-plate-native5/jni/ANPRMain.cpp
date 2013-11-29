#include <iostream>
#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "AVS-ANPR.h"
#include "ANPRMain.h"
#include "FindPlates.h"
//#include "ReadPlateText.h"

using namespace std;
using namespace cv;

bool verifySizes(RotatedRect rectCandidatePlate);
double LineLength(cv::Point *pt1, cv::Point *pt2);

///////////////////////////////////////////////////////////////////////
// ANPRMain()
// Top-level function encapsulating the whole process, so that on
// Android, as little as possible has to be done in Java.
//
// Params:
// imageSource - source image, but will also be overwritten with
//    whatever debug window the user selects, if any.  The found
//    plate, and the text, will also be displayed in the TLH corner.
// iDebugMode - what information to display on screen (i.e. what
//    to copy over the imageSource - whatever goes there will be
//    displayed).
//    0 - Source only
//    1 - Sobel vert edges
//    2 - Morph
//    3 - Debug: source, with all found plates highlighted
//
// Returns:
//    Number of plates found, or -1 to exit program (e.g. ESC pressed
//    by user).
//
///////////////////////////////////////////////////////////////////////

int ANPRMain(Mat &imageSource, Mat &imagePlate, int iDebugMode) {

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
    Mat imageDebug;
    Mat imageGray;
    Mat imageThreshold;
    Mat imageThresholdBackToColorForDisplay;
    Mat imageDilated;

    imageSource.copyTo(imageDebug);

    Mat element = getStructuringElement(MORPH_RECT, Size(25,3));

    rectsNotRotated.clear();
    rectsRotated.clear();

    FindPlates(imageSource, rectsNotRotated, rectsRotated, imageDebug, iDebugMode);

    // Draw all the rectangles returned

    printf("There are %d rects\n", rectsNotRotated.size());
    printf("There are %d rotated rects\n", rectsRotated.size());

    if (rectsNotRotated.size() < 1) {
#ifdef AVS_QT // No cvWaitKey on Android
    	iKey = cvWaitKey(1);
        if (iKey == 27) {
            return(-1);                 // -1 == tell caller to exit
            }
#endif // AVS_QT


        // No plates found, but need to copy over whatever debug window they want
        // to see anyway.
        imageDebug.copyTo(imageSource);
        return(0);                      // 0 == no plates found
        }

    for (vector<Rect>::size_type i=0; i != rectsNotRotated.size(); i++) {
    	Rect rectTemp = rectsNotRotated[i];
    	rectangle(imageDebug, rectTemp, CV_RGB(255,0,0), 3);

        //rectangle(imageDebug, rectsNotRotated[i], CV_RGB(255,0,0), 3);
        }



    for (vector<RotatedRect>::size_type i=0; i != rectsRotated.size(); i++) {
        // Draw the points at the corners of this 'square'
        RotatedRect thisRotatedRect = rectsRotated[i];
        Point2f points[4];

        thisRotatedRect.points(points);

        for (int iPoint=0; iPoint<4; iPoint++) {
            line(imageDebug, points[iPoint], points[(iPoint+1) % 4], Scalar(255,0,0,1), 2);
            circle(imageDebug, points[iPoint], 3, Scalar(0,0,255,1), -1);
            }

        }

    //FindLargestPlate(rectsRotated, rotatedrectLargestPlate);
    FindBestPlateCandidate(rectsRotated, rotatedrectLargestPlate, imageSource.size().width, imageSource.size().height);
    Point2f points[4];
    rotatedrectLargestPlate.points(points);

    for (int iPoint=0; iPoint<4; iPoint++) {
        line(imageDebug, points[iPoint], points[(iPoint+1) % 4], Scalar(0,255,0,1), 2);
        circle(imageDebug, points[iPoint], 3, Scalar(0,0,255,1), -1);
        }

#ifdef AVS_QT
    imshow("Debug", imageDebug);
#endif

    imagePlateLoc = DerotateAndCorrectPerspective(imageSource, rotatedrectLargestPlate);

#ifdef OBSOLETE

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

#endif

    Rect roi(Point(0,0), imagePlateLoc.size()); // ROI for colour plate display
    Rect roi2(Point(0,imagePlateLoc.size().height), imagePlateLoc.size()); // ROI for thresholded plate display

    //imagePlateLoc.copyTo(imageDebug(roi));

    if (iDebugMode == AVS_DEBUG_MODE_DISPLAY_PLATE) {
    	// Return a full size image, but with just the plate in the TLH corner,
    	// and the rest of the image blacked out.  Later, we'll return just the
    	// plate itself.
    	imageDebug.setTo(0);

//        imagePlateLoc.copyTo(imageSource);
        }

    cvtColor(imagePlateLoc, imageGray, CV_BGR2GRAY);
    threshold(imageGray, imageThreshold, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);

    // Set up imageWithWhiteBorder

    //imageWithWhiteBorder = Mat(imageDilated.Size.width, imageDilated.Size.height, )
    dilate(imageThreshold, imageDilated, cv::Mat());
    //Mat imageWithWhiteBorder = Mat::zeros(imageDilated.size().width+30, imageDilated.size().height+30, CV_8UC1);
    //Mat imageWithWhiteBorder =  Mat::zeros(70, 20, imageDilated.type());

    //Rect roi3(Point(0,imagePlateLoc.size().height*2), imageWithWhiteBorder.size()); // ROI for thresholded plate display
    //imageDilated.copyTo(imageWithWhiteBorder(roi));
    cvtColor(imageDilated, imageThresholdBackToColorForDisplay, CV_GRAY2BGRA);
    //cvtColor(imageWithWhiteBorder, imageThresholdBackToColorForDisplay, CV_GRAY2BGRA);

    imageDebug.copyTo(imageSource);

    //imagePlate = imagePlateLoc;
    //imagePlate = imageThreshold;
    imagePlate = imageDilated;

    // Copy the plate onto the source image (or whichever debug is chosen) - this
    // could be optional later.



    imagePlateLoc.copyTo(imageSource(roi));
    //imageDilated.copyTo(imageSource(roi2));
    imageThresholdBackToColorForDisplay.copyTo(imageSource(roi2));
    //imageThresholdBackToColorForDisplay.copyTo(imageSource(roi3));

    return(0);
    }
